cmake_minimum_required(VERSION 3.25)
project(TaskScheduler)

set(CMAKE_CXX_STANDARD 17)

#include_directories(src)
include_directories(lib)
#include_directories(test)

add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(test)