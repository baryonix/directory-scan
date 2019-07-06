#ifndef DIRECTORY_SCAN_H
#define DIRECTORY_SCAN_H

#include <filesystem>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <boost/asio.hpp>
#include <boost/chrono/time_point.hpp>


namespace directory_scan {

    namespace asio = boost::asio;
    namespace chrono = boost::chrono;
    namespace fs = std::filesystem;


    /*
     * Thin wrapper for the POSIX lstat() call, which we call directly due to
     * some deficiencies in the boost::filesystem and std::filesystem APIs.
     * Using the latter we end up doing multiple stat() calls to retrieve
     * different attributes of the same file, have no option to use lstat()
     * instead of stat() and get spurious EPERM errors on symbolic links even
     * when the underlying stat() call returns successfully.
     */
    static struct stat do_lstat(const fs::path &path) {
        struct stat result;
        int rc = lstat(path.string().c_str(), &result);
        if (rc != 0)
            throw fs::filesystem_error(std::string(strerror(errno)),
                                       boost::system::error_code(errno, boost::system::system_category()));
        return result;
    }


    class FileInformation {
    public:
        typedef chrono::time_point<chrono::system_clock> time_type;

        FileInformation(std::string path, uint64_t size, const time_type &lastWriteTime) : _path(std::move(path)),
                                                                                           _size(size),
                                                                                           _lastWriteTime(
                                                                                                   lastWriteTime) {}

        const std::string &path() const {
            return _path;
        }

        uint64_t size() const {
            return _size;
        }

        const time_type &lastWriteTime() const {
            return _lastWriteTime;
        }

        bool operator<(const FileInformation &other) const {
            return _path < other._path;
        }

    private:
        std::string _path;
        uint64_t _size;
        time_type _lastWriteTime;
    };


    template<class Consumer>
    class PathScanner {
    public:
        PathScanner(fs::path path, asio::thread_pool &pool, Consumer &consumer)
                : path(std::move(path)), pool(pool), consumer(consumer) {}

        void operator()() {
            try {
                scan();
            } catch (const fs::filesystem_error &error) {
                logError(error, "Failed to scan directory");
            }
        }

    private:
        void scan() {
            for (const fs::directory_entry &entry : fs::directory_iterator(path)) {
                try {
                    processEntry(entry);
                } catch (const fs::filesystem_error &error) {
                    logError(error, "Failed to process");
                }
            }
        }

        void processEntry(const fs::directory_entry &entry) {
            const fs::path &childPath = entry.path();
            struct stat status = do_lstat(childPath);
            if (S_ISDIR(status.st_mode)) {
                asio::post(pool, childScanner(childPath));
            } else {
                consumer(fileInformation(childPath, status));
            }
        }

        PathScanner childScanner(const fs::path &child) {
            return PathScanner{child, pool, consumer};
        }

        static FileInformation fileInformation(const fs::path &path, const struct stat &status) {
            return FileInformation(path.string(), status.st_size, fromTimespec(status.st_mtim));
        }

        static FileInformation::time_type fromTimespec(const timespec &timespec) {
            return FileInformation::time_type(chrono::seconds(timespec.tv_sec))
                   + chrono::nanoseconds(timespec.tv_nsec);
        }

        static void logError(const fs::filesystem_error &error, const std::string &message) {
            std::cerr
                    << message << ' '
                    << error.path1() << ": "
                    << error.what() << std::endl;
        }

        fs::path path;
        asio::thread_pool &pool;
        Consumer &consumer;
    };


    template<class Iterator, class Consumer>
    static void scanDirectories(Iterator first, Iterator last, Consumer &consumer, unsigned parallelism) {
        asio::thread_pool pool{parallelism};
        for (Iterator iterator = first; iterator != last; ++iterator)
            asio::post(pool, PathScanner{*iterator, pool, consumer});
        pool.join();
    }

}

#endif //DIRECTORY_SCAN_H
