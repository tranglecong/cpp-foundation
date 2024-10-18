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

// Unit Test for running the thread in LOOP mode with a predicate
TEST(ThreadTest, RunLoopWithPredicate) {
    // Shared atomic variable to control the predicate
    std::atomic<int> counter{0};

    // Predicate to control when the thread should stop
    auto pred = [&counter]() {
        return counter < 5; // Run the loop until counter reaches 5
    };

    // Create a thread in RunMode::LOOP
    Thread<int, int, std::string> thread("LoopThread", RunMode::LOOP, ThreadPriority::NORMAL);

    // Set the function and arguments using invoke()
    bool success = thread.invoke(mockTask, 10, "Loop Test");
    EXPECT_TRUE(success); // Ensure the function and arguments were set correctly

    // Set the predicate, start callback, result callback, and exit callback
    thread.setPredicate(pred);
    thread.setStartCallback(mockStart);
    thread.setResultCallback(mockResultCallback);
    thread.setExitCallback(mockExit);

    // Start the thread in loop mode
    EXPECT_TRUE(thread.start());

    // Increment the counter in the main thread to allow the predicate to stop the loop
    while (counter < 5) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        ++counter;
    }

    // Wait for the thread to finish
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Test should pass without errors, and the thread should have run in a loop
    SUCCEED();
}

// Unit Test for stopping the thread manually in LOOP mode
TEST(ThreadTest, StopLoopManually) {
    // Shared atomic variable to control the loop
    std::atomic<int> loop_counter{0};

    // Predicate to control the loop condition (always return true here for manual stop)
    auto pred = [&loop_counter]() {
        return true; // This will allow the loop to continue indefinitely
    };

    // Create a thread in RunMode::LOOP
    Thread<int, int, std::string> thread("LoopThreadManualStop", RunMode::LOOP, ThreadPriority::NORMAL);

    // Set the function and arguments using invoke()
    bool success = thread.invoke(mockTask, 5, "Manual Stop Test");
    EXPECT_TRUE(success);

    // Set the predicate, start callback, and exit callback
    thread.setPredicate(pred);
    thread.setStartCallback(mockStart);
    thread.setExitCallback(mockExit);

    // Start the thread in loop mode
    EXPECT_TRUE(thread.start());

    // Simulate the loop running for a short time
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Manually stop the thread
    EXPECT_TRUE(thread.stop());

    // Wait for the thread to finish stopping
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Test should pass without errors, and the thread should have stopped manually
    SUCCEED();
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
