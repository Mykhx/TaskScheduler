### set project var ###
set(PROJECT ${CMAKE_PROJECT_NAME})

### google test ###
include(FetchContent)
FetchContent_Declare(
        googletest
        # Specify the commit you depend on and update it regularly.
        URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

### find all .h and .cpp files and set them as sources ###
file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false *.h *.cpp)
set(SOURCES ${TEST_SOURCES})

### Add executable ###
add_executable(${PROJECT}_test ${TEST_SOURCES} DummyClassTest.cpp)

### Link lib and googletest
target_link_libraries(${PROJECT}_test PUBLIC ${CMAKE_PROJECT_NAME}_lib GTest::gtest_main)

# include(GoogleTest)
# gtest_discover_tests(${CMAKE_PROJECT_NAME}_test)