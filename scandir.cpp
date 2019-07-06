#include <boost/chrono/time_point.hpp>
#include <boost/chrono/io/time_point_io.hpp>
#include <boost/program_options.hpp>
#include <iostream>

#include "collector.h"
#include "directory_scan.h"
#include "escape.h"


static void parse_command_line(int argc, char *argv[]) {
    using namespace boost::program_options;
    options_description description{"Options"};
    description.add_options()
            ("help,h", "Help screen");
}


int main(int argc, char *argv[]) {
    std::vector<std::string> directoriesToScan = {"/home/jan"};
    collector::VectorCollector<directory_scan::FileInformation> collector;
    directory_scan::scanDirectories(directoriesToScan.cbegin(), directoriesToScan.cend(), collector, 16);
    auto files = collector.retrieveAndClear();
    std::sort(files.begin(), files.end());

    boost::chrono::set_time_fmt(std::cout, std::string("%FT%T%Ez"));
    for (const auto &file : files) {
        std::cout
                << boost::chrono::system_clock::from_time_t(file.lastWriteTime()) << ' '
                << file.size() << ' '
                << escape::escapeSpecialChars(file.path()) << std::endl;
    }

    return 0;
}
