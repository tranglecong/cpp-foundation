project(Tests)


enable_testing()

find_package(GTest CONFIG REQUIRED)

add_executable(thread_safe_variable_test thread_safe_variable_test.cpp)
target_link_libraries(thread_safe_variable_test PUBLIC ThreadSafe PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

add_executable(thread_safe_wait_test thread_safe_wait_test.cpp)
target_link_libraries(thread_safe_wait_test PUBLIC ThreadSafe PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

add_executable(thread_safe_thread_test thread_safe_thread_test.cpp)
target_link_libraries(thread_safe_thread_test PUBLIC ThreadSafe PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)