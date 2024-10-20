# Minimum version required
cmake_minimum_required(VERSION 3.10)

project(Tests)

enable_testing()

find_package(GTest CONFIG REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(TEST_SOURCES
    thread_safe_variable_test.cpp
    thread_safe_wait_test.cpp
    thread_safe_thread_test.cpp
    thread_safe_queue_test.cpp
)


set(GTEST_LIBS
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    GTest::gmock_main
)

foreach(TEST_SOURCE ${TEST_SOURCES})
    # Extract test name from the source file name (e.g., thread_safe_variable_test from thread_safe_variable_test.cpp)
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)

    add_executable(${TEST_NAME} ${TEST_SOURCE})

    target_link_libraries(${TEST_NAME} PUBLIC ThreadSafe PRIVATE ${GTEST_LIBS})

    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

    target_compile_features(${TEST_NAME} PUBLIC cxx_std_17)
endforeach()

