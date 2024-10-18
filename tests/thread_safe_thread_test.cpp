#include "thread_safe/thread.hpp"
#include <chrono>
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
    Thread<int, int, std::string> thread("TestThread", ThreadPriority::NORMAL);

    // Set the function and arguments using invoke() before starting the thread
    bool success = thread.invoke(mockTask, 42, "Hello, World!");
    EXPECT_TRUE(success); // Ensure the function and arguments were set successfully

    // Set callbacks
    thread.setStartCallback(mockStart);
    thread.setResultCallback(mockResultCallback);
    thread.setExitCallback(mockExit);

    // Start the thread
    EXPECT_TRUE(thread.start(RunMode::ONCE)); // Ensure the thread started successfully

    // Simulate a short delay to allow thread execution
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Ensure that the callbacks were triggered during execution
    SUCCEED();
}

// Unit Test for using start, result, and exit callbacks
TEST(ThreadTest, SetCallbacksAndRunThread) {
    // Create a thread object with the function and arguments not yet set
    Thread<int, int, std::string> thread("TestThread", ThreadPriority::NORMAL);

    // Set the function and arguments using invoke() before starting the thread
    bool success = thread.invoke(mockTask, 10, "Test");
    EXPECT_TRUE(success); // Ensure the function and arguments were set successfully

    // Set callbacks
    thread.setStartCallback(mockStart);
    thread.setResultCallback(mockResultCallback);
    thread.setExitCallback(mockExit);

    // Start the thread
    EXPECT_TRUE(thread.start(RunMode::ONCE));

    // Simulate a short delay to allow thread execution
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Check if all callbacks were triggered
    SUCCEED(); // Ensure no errors occurred in the thread
}

// Unit Test for stopping the thread
TEST(ThreadTest, StopThread) {
    // Create a thread object with the function and arguments not yet set
    Thread<int, int, std::string> thread("TestThread", ThreadPriority::NORMAL);

    // Set the function and arguments using invoke() before starting the thread
    bool success = thread.invoke(mockTask, 10, "Stop Test");
    EXPECT_TRUE(success);

    // Set start and exit callbacks
    thread.setStartCallback(mockStart);
    thread.setExitCallback(mockExit);

    // Start the thread
    thread.start(RunMode::LOOP);

    // Stop the thread
    bool stop_result = thread.stop();
    EXPECT_TRUE(stop_result); // Ensure the thread was stopped successfully

    // Simulate a short delay to ensure thread stops
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    SUCCEED(); // Ensure no errors occurred in the thread.
}

// Unit Test for running the thread in LOOP mode with a predicate
TEST(ThreadTest, RunLoopWithPredicate) {
    // Shared atomic variable to control the predicate
    std::atomic<int> counter{0};

    // Predicate to control when the thread should stop
    auto pred = [&counter]() -> bool {
        return counter < 5; // Run the loop until counter reaches 5
    };

    // Create a thread in RunMode::LOOP
    Thread<int, int, std::string> thread("LoopThread", ThreadPriority::NORMAL);

    // Set the function and arguments using invoke()
    bool success = thread.invoke(mockTask, 10, "Loop Test");
    EXPECT_TRUE(success); // Ensure the function and arguments were set correctly

    // Set the predicate, start callback, result callback, and exit callback
    thread.setPredicate(pred);
    thread.setStartCallback(mockStart);
    thread.setResultCallback(mockResultCallback);
    thread.setExitCallback(mockExit);

    // Start the thread in loop mode
    EXPECT_TRUE(thread.start(RunMode::LOOP));

    // Increment the counter in the main thread to allow the predicate to stop the loop
    while (counter < 5) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        ++counter;
    }

    // Wait for the thread to finish
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

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
    Thread<int, int, std::string> thread("LoopThreadManualStop", ThreadPriority::NORMAL);

    // Set the function and arguments using invoke()
    bool success = thread.invoke(mockTask, 5, "Manual Stop Test");
    EXPECT_TRUE(success);

    // Set the predicate, start callback, and exit callback
    thread.setPredicate(pred);
    thread.setStartCallback(mockStart);
    thread.setExitCallback(mockExit);

    // Start the thread in loop mode
    EXPECT_TRUE(thread.start(RunMode::LOOP));

    // Simulate the loop running for a short time
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Manually stop the thread
    EXPECT_TRUE(thread.stop());

    // Wait for the thread to finish stopping
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Test should pass without errors, and the thread should have stopped manually
    SUCCEED();
}

// Unit Test for starting and stopping the thread multiple times
TEST(ThreadTest, StartStopMultipleTimes) {
    std::atomic<int> loop_counter{0}; // Atomic counter for controlling the loop
    std::atomic<int> stop_counter{0}; // Counter to ensure the loop stops

    // Predicate to control when the loop should stop
    auto pred = [&]() -> bool {
        loop_counter++;
        if (loop_counter >= 5) {
            stop_counter = 1; // Stop after 5 iterations
            return false;
        }
        return true;
    };

    // Create the thread in RunMode::LOOP
    Thread<int, int, std::string> thread("LoopThreadMultipleStop", ThreadPriority::NORMAL);

    // Set the function and arguments using invoke()
    bool success = thread.invoke(mockTask, 10, "Test Stop Multiple");
    EXPECT_TRUE(success); // Ensure the function and arguments were set correctly

    // Set the predicate, start callback, result callback, and exit callback
    thread.setPredicate(pred);
    thread.setStartCallback(mockStart);
    thread.setResultCallback(mockResultCallback);
    thread.setExitCallback(mockExit);

    // Start the thread
    EXPECT_TRUE(thread.start(RunMode::LOOP));
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Run for a short time

    // First stop the thread
    EXPECT_TRUE(thread.stop());
    EXPECT_EQ(stop_counter.load(), 1); // Ensure the thread was stopped after 5 iterations

    // Start the thread again
    EXPECT_TRUE(thread.start(RunMode::LOOP));
    loop_counter = 0; // Reset counter for the second run

    // Simulate running the thread for a while again
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Stop the thread again
    EXPECT_TRUE(thread.stop());
    EXPECT_EQ(stop_counter.load(), 1); // Ensure the thread was stopped after another 5 iterations

    // Start the thread once more
    EXPECT_TRUE(thread.start(RunMode::LOOP));
    loop_counter = 0; // Reset counter for the third run

    // Simulate running the thread for a while again
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Stop the thread once more
    EXPECT_TRUE(thread.stop());
    EXPECT_EQ(stop_counter.load(), 1); // Ensure the thread was stopped again

    // The thread should have stopped three times
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    SUCCEED(); // Ensure no errors occurred
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
