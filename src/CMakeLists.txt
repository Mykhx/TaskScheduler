# recursive search
file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.h *.cpp)

set(PROJECT ${CMAKE_PROJECT_NAME})
set(SOURCE_FILES ${SOURCES})

add_executable(${PROJECT}_run ${SOURCE_FILES})

include_directories(${CMAKE_SOURCE_DIR}/lib)
#target_include_directories(${PROJECT}_run ${CMAKE_SOURCE_DIR}/lib)
target_link_libraries(${PROJECT}_run ${PROJECT}_lib)

# target_link_libraries(${PROJECT}_run PUBLIC ${CMAKE_SOURCE_DIR}/lib/lib${CMAKE_PROJECT_NAME}_lib.a)