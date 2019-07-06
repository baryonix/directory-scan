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


template<class Iterator>
static void outputResults(std::ostream &output, Iterator first, Iterator last) {
    boost::chrono::set_time_fmt(output, std::string("%FT%T%Ez"));
    for (Iterator iterator = first; iterator != last; ++iterator) {
        const auto &file = *iterator;
        output
                << boost::chrono::system_clock::from_time_t(file.lastWriteTime()) << ' '
                << file.size() << ' '
                << escape::escapeSpecialChars(file.path()) << std::endl;
    }
}


int main(int argc, char *argv[]) {
    std::vector<std::string> directoriesToScan = {"/home/jan"};
    collector::VectorCollector<directory_scan::FileInformation> collector;
    directory_scan::scanDirectories(directoriesToScan.cbegin(), directoriesToScan.cend(), collector, 16);
    auto files = collector.retrieveAndClear();
    std::sort(files.begin(), files.end());

    outputResults(std::cout, files.begin(), files.end());

    return 0;
}
