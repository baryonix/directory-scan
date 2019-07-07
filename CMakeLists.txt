cmake_minimum_required(VERSION 3.14)
project(directory_scan)

set(CMAKE_CXX_STANDARD 17)

enable_testing()

find_package(Boost)
find_package(GTest)

add_executable(scandir scandir.cpp)
target_link_libraries(scandir -lboost_chrono -lboost_filesystem -lboost_program_options -lpthread)

add_executable(all_tests test_escape.cpp)
target_link_libraries(all_tests -lgtest_main -lgtest)
gtest_discover_tests(all_tests)
