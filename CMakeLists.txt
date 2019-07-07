cmake_minimum_required(VERSION 3.14)
project(directory_scan)

set(CMAKE_CXX_STANDARD 17)

enable_testing()

find_package(Threads)
find_package(Boost REQUIRED COMPONENTS chrono program_options)
find_package(GTest MODULE)
find_package(GTest CONFIG REQUIRED COMPONENTS gmock_main)

add_executable(scandir scandir.cpp)
target_link_libraries(scandir Boost::chrono Boost::program_options Threads::Threads)

add_executable(all_tests test_escape.cpp test_collector.cpp test_directory_scan.cpp)
target_link_libraries(all_tests GTest::gmock_main Boost::chrono)
gtest_discover_tests(all_tests)
