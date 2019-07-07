cmake_minimum_required(VERSION 3.14)
project(directory_scan)

set(CMAKE_CXX_STANDARD 17)

enable_testing()

find_package(Boost)
find_package(GTest)

add_executable(scandir scandir.cpp)
target_link_libraries(scandir -lboost_chrono -lboost_program_options -lpthread)

add_executable(all_tests test_escape.cpp test_collector.cpp test_directory_scan.cpp)
target_link_libraries(all_tests -lgmock -lgtest_main -lgtest -lboost_chrono -lpthread)
gtest_discover_tests(all_tests)
