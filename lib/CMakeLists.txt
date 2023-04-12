set(PROJECT ${CMAKE_PROJECT_NAME})

# include directories
include_directories(TaskScheduler)

# recursive search
file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.h *.cpp)

set(SOURCE_FILES ${SOURCES} TaskScheduler/TaskScheduler.cpp TaskScheduler/TaskScheduler.h)

include(CMakePrintHelpers)
CMAKE_PRINT_VARIABLES(SOURCES)

add_library(${PROJECT}_lib STATIC ${SOURCE_FILES})