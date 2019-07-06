#include <boost/chrono/time_point.hpp>
#include <boost/chrono/io/time_point_io.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>

#include "collector.h"
#include "directory_scan.h"
#include "escape.h"


namespace chrono = boost::chrono;


class Configuration {
public:
    std::vector<std::string> directories;
    unsigned parallelism = 0;
    std::string outputFileName;
};


static Configuration parse_command_line(int argc, char *argv[]) {
    using namespace boost::program_options;
    Configuration configuration;

    options_description description{"Options"};
    description.add_options()
            ("help,h", "Help screen")
            ("directory,d", value(&configuration.directories), "Directories to search")
            ("parallelism,p", value(&configuration.parallelism)->default_value(1),
             "Number of concurrent threads")
            ("output-file,o", value(&configuration.outputFileName)->default_value("-"),
             "Output file name, or \"-\" for standard output");

    positional_options_description positional;
    positional.add("directory", -1);

    auto parseResult = command_line_parser(argc, argv).options(description).positional(positional).run();

    variables_map variablesMap;
    store(parseResult, variablesMap);
    notify(variablesMap);

    if (variablesMap.count("help")) {
        std::cerr << description;
        std::exit(1);
    }

    return configuration;
}


template<class Iterator>
static void outputResults(std::ostream &output, Iterator first, Iterator last) {
    boost::chrono::set_time_fmt(output, std::string("%FT%T%Ez"));
    for (Iterator iterator = first; iterator != last; ++iterator) {
        const auto &file = *iterator;
        output
                << file.lastWriteTime() << ' '
                << file.size() << ' '
                << escape::escapeSpecialChars(file.path()) << std::endl;
    }
}


template<class Consumer>
static void performScan(const Configuration &configuration, Consumer &consumer) {
    typedef chrono::high_resolution_clock clock;
    auto startTime = clock::now();

    const auto &directories = configuration.directories;
    directory_scan::scanDirectories(directories.begin(), directories.end(), consumer, configuration.parallelism);

    chrono::duration duration = clock::now() - startTime;
    std::cerr
            << "File system scan completed in "
            << chrono::duration_cast<chrono::milliseconds>(duration).count() * 1.0e-3
            << " seconds" << std::endl;
}


static void scanAndOutput(const Configuration &configuration, std::ostream &output) {
    collector::VectorCollector<directory_scan::FileInformation> collector;
    performScan(configuration, collector);
    auto files = collector.retrieveAndClear();
    std::sort(files.begin(), files.end());
    outputResults(output, files.begin(), files.end());
}


int main(int argc, char *argv[]) {
    Configuration configuration = parse_command_line(argc, argv);

    if (configuration.outputFileName == "-") {
        scanAndOutput(configuration, std::cout);
    } else {
        std::ofstream output;
        output.exceptions(std::ofstream::failbit);
        output.open(configuration.outputFileName);
        scanAndOutput(configuration, output);
    }

    return 0;
}
