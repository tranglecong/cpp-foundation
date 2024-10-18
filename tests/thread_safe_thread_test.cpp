#include "thread_safe/thread.hpp"
#include <functional>
#include <gtest/gtest.h>
#include <string>

using namespace ThreadSafe;

// Mock function to simulate a thread task
int mockTask(int num, const std::string &message) {
    std::cout << "Task running with num: " << num << " and message: " << message << std::endl;
    return num;
}

// Mock start callback
void mockStart() { std::cout << "Start callback triggered." << std::endl; }

// Mock result callback
void mockResultCallback(const int &result) { std::cout << "Result callback received: " << result << std::endl; }

// Mock exit callback
void mockExit() { std::cout << "Exit callback triggered." << std::endl; }

// Unit Test for invoking a function with arguments and running the thread
TEST(ThreadTest, InvokeAndRunThread) {
    // Create a thread object with the function and arguments not yet set
    Thread<int, int, std::string> thread("TestThread", RunMode::ONCE, ThreadPriority::NORMAL);

    // Set the function and arguments using invoke() before starting the thread
    bool success = thread.invoke(mockTask, 42, "Hello, World!");
    EXPECT_TRUE(success); // Ensure the function and arguments were set successfully

    // Set callbacks
    thread.setStartCallback(mockStart);
    thread.setResultCallback(mockResultCallback);
    thread.setExitCallback(mockExit);

    // Start the thread
    EXPECT_TRUE(thread.start()); // Ensure the thread started successfully

    // Simulate a short delay to allow thread execution
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Ensure that the callbacks were triggered during execution
    SUCCEED();
}

// Unit Test for using start, result, and exit callbacks
TEST(ThreadTest, SetCallbacksAndRunThread) {
    // Create a thread object with the function and arguments not yet set
    Thread<int, int, std::string> thread("TestThread", RunMode::ONCE, ThreadPriority::NORMAL);

    // Set the function and arguments using invoke() before starting the thread
    bool success = thread.invoke(mockTask, 10, "Test");
    EXPECT_TRUE(success); // Ensure the function and arguments were set successfully

    // Set callbacks
    thread.setStartCallback(mockStart);
    thread.setResultCallback(mockResultCallback);
    thread.setExitCallback(mockExit);

    // Start the thread
    EXPECT_TRUE(thread.start());

    // Simulate a short delay to allow thread execution
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Check if all callbacks were triggered
    SUCCEED(); // Ensure no errors occurred in the thread
}

// Unit Test for stopping the thread
TEST(ThreadTest, StopThread) {
    // Create a thread object with the function and arguments not yet set
    Thread<int, int, std::string> thread("TestThread", RunMode::ONCE, ThreadPriority::NORMAL);

    // Set the function and arguments using invoke() before starting the thread
    bool success = thread.invoke(mockTask, 10, "Stop Test");
    EXPECT_TRUE(success);

    // Set start and exit callbacks
    thread.setStartCallback(mockStart);
    thread.setExitCallback(mockExit);

    // Start the thread
    thread.start();

    // Stop the thread
    bool stop_result = thread.stop();
    EXPECT_TRUE(stop_result); // Ensure the thread was stopped successfully

    // Simulate a short delay to ensure thread stops
    std::this_thread::sleep_for(std::chrono::seconds(1));

    SUCCEED(); // Ensure no errors occurred in the thread.
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
