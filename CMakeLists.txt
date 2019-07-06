cmake_minimum_required(VERSION 3.14)
project(directory_scan)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost)

add_executable(scandir scandir.cpp)

target_link_libraries(scandir -lboost_chrono -lboost_filesystem -lboost_program_options -lpthread)
