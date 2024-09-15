#include "atom/async/async.hpp"

#include <gtest/gtest.h>

// Test fixture for AsyncWorker class
class AsyncWorkerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up code before each test case
    }

    void TearDown() override {
        // Tear down code after each test case
    }

    // Helper function to validate the result of a task
    bool validateResult(const std::function<bool(int)>& validator, int result) {
        return validator(result);
    }
};

TEST_F(AsyncWorkerTest, StartAsync_ValidFunction_ReturnsExpectedResult) {
    // Arrange
    atom::async::AsyncWorker<int> asyncWorker;
    std::function<int()> task = []() { return 42; };

    // Act
    asyncWorker.startAsync(task);

    // Assert
    EXPECT_TRUE(asyncWorker.isActive());
}

TEST_F(AsyncWorkerTest, GetResult_ValidTask_ReturnsExpectedResult) {
    // Arrange
    atom::async::AsyncWorker<int> asyncWorker;
    std::function<int()> task = []() { return 42; };
    asyncWorker.startAsync(task);

    // Act
    int result = asyncWorker.getResult();

    // Assert
    EXPECT_EQ(result, 42);
}

TEST_F(AsyncWorkerTest, Cancel_ActiveTask_WaitsForCompletion) {
    // Arrange
    atom::async::AsyncWorker<int> asyncWorker;
    std::function<int()> task = []() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return 42;
    };
    asyncWorker.startAsync(task);

    // Act
    asyncWorker.cancel();

    // Assert
    EXPECT_FALSE(asyncWorker.isActive());
}

TEST_F(AsyncWorkerTest, Validate_ValidResult_ReturnsTrue) {
    // Arrange
    atom::async::AsyncWorker<int> asyncWorker;
    std::function<int()> task = []() { return 42; };
    asyncWorker.startAsync(task);
    std::function<bool(int)> validator = [](int result) {
        return result == 42;
    };

    // Act
    bool isValid = asyncWorker.validate(validator);

    // Assert
    EXPECT_TRUE(isValid);
}

TEST_F(AsyncWorkerTest, Validate_InvalidResult_ReturnsFalse) {
    // Arrange
    atom::async::AsyncWorker<int> asyncWorker;
    std::function<int()> task = []() { return 42; };
    asyncWorker.startAsync(task);
    std::function<bool(int)> validator = [](int result) {
        return result == 43;
    };

    // Act
    bool isValid = asyncWorker.validate(validator);

    // Assert
    EXPECT_FALSE(isValid);
}

TEST_F(AsyncWorkerTest, SetCallback_ValidCallback_CallsCallbackWithResult) {
    // Arrange
    atom::async::AsyncWorker<int> asyncWorker;
    std::function<int()> task = []() { return 42; };
    std::function<void(int)> callback = [](int result) {
        EXPECT_EQ(result, 42);
    };
    asyncWorker.setCallback(callback);
    asyncWorker.startAsync(task);

    // Act
    asyncWorker.waitForCompletion();
}

TEST_F(AsyncWorkerTest, SetTimeout_ValidTimeout_WaitsForTimeout) {
    // Arrange
    atom::async::AsyncWorker<int> asyncWorker;
    std::function<int()> task = []() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return 42;
    };
    asyncWorker.setTimeout(std::chrono::seconds(1));
    asyncWorker.startAsync(task);

    // Act
    asyncWorker.waitForCompletion();

    // Assert
    EXPECT_FALSE(asyncWorker.isActive());
}

// Test fixture for AsyncWorkerManager class
class AsyncWorkerManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up code before each test case
    }

    void TearDown() override {
        // Tear down code after each test case
    }

    // Helper function to create and start a task
    std::shared_ptr<atom::async::AsyncWorker<int>> createAndStartTask(
        const std::function<int()>& task) {
        auto worker = asyncWorkerManager.createWorker(task);
        worker->startAsync(task);
        return worker;
    }

    atom::async::AsyncWorkerManager<int> asyncWorkerManager;
};

TEST_F(AsyncWorkerManagerTest, CreateWorker_ValidFunction_ReturnsValidWorker) {
    // Arrange
    std::function<int()> task = []() { return 42; };

    // Act
    auto worker = asyncWorkerManager.createWorker(task);

    // Assert
    EXPECT_TRUE(worker != nullptr);
    EXPECT_TRUE(worker->isActive());
}

TEST_F(AsyncWorkerManagerTest, CancelAll_AllTasks_CancelsAllTasks) {
    // Arrange
    std::function<int()> task1 = []() { return 42; };
    std::function<int()> task2 = []() { return 43; };
    auto worker1 = createAndStartTask(task1);
    auto worker2 = createAndStartTask(task2);

    // Act
    asyncWorkerManager.cancelAll();

    // Assert
    EXPECT_FALSE(worker1->isActive());
    EXPECT_FALSE(worker2->isActive());
}

TEST_F(AsyncWorkerManagerTest, AllDone_AllTasksDone_ReturnsTrue) {
    // Arrange
    std::function<int()> task1 = []() { return 42; };
    std::function<int()> task2 = []() { return 43; };
    createAndStartTask(task1);
    createAndStartTask(task2);

    // Act
    bool allDone = asyncWorkerManager.allDone();

    // Assert
    EXPECT_TRUE(allDone);
}

TEST_F(AsyncWorkerManagerTest, WaitForAll_AllTasks_WaitsForAllTasks) {
    // Arrange
    std::function<int()> task1 = []() { return 42; };
    std::function<int()> task2 = []() { return 43; };
    createAndStartTask(task1);
    createAndStartTask(task2);

    // Act
    asyncWorkerManager.waitForAll();

    // Assert
    EXPECT_FALSE(asyncWorkerManager.allDone());
}

TEST_F(AsyncWorkerManagerTest, IsDone_ValidWorker_ReturnsExpectedResult) {
    // Arrange
    std::function<int()> task = []() { return 42; };
    auto worker = createAndStartTask(task);

    // Act
    bool isDone = asyncWorkerManager.isDone(worker);

    // Assert
    EXPECT_TRUE(isDone);
}

TEST_F(AsyncWorkerManagerTest, Cancel_ValidWorker_CancelsWorker) {
    // Arrange
    std::function<int()> task = []() { return 42; };
    auto worker = createAndStartTask(task);

    // Act
    asyncWorkerManager.cancel(worker);

    // Assert
    EXPECT_FALSE(worker->isActive());
}

// Helper function to create a short delay
void delayMs(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

// Test: Simple asynchronous task execution
TEST(EnhancedFutureTest, AsyncTaskExecution) {
    auto future = atom::async::makeEnhancedFuture([]() -> int {
        delayMs(500);
        return 42;
    });

    // Test that the task is completed
    EXPECT_EQ(future.wait(), 42);
}

// Test: Void asynchronous task with then() chaining
TEST(EnhancedFutureTest, ThenChaining) {
    std::atomic<bool> thenExecuted{false};

    auto future = atom::async::makeEnhancedFuture([]() {
        delayMs(500);
        std::cout << "Task Done" << std::endl;
    });

    // Chain then to check if it executes after task completion
    future.then([&]() {
        thenExecuted = true;
        std::cout << "Then executed" << std::endl;
    });

    // Verify that the then() function executed successfully
    future.wait();
    EXPECT_TRUE(thenExecuted);
}

// Test: onComplete() callback invocation
TEST(EnhancedFutureTest, OnCompleteCallback) {
    std::atomic<bool> callbackExecuted{false};

    auto future = atom::async::makeEnhancedFuture([]() -> int {
        delayMs(500);
        return 42;
    });

    // Set a completion callback to be executed when the task is done
    future.onComplete([&](int result) {
        EXPECT_EQ(result, 42);
        callbackExecuted = true;
    });

    // Wait for completion and check if the callback was invoked
    future.wait();
    EXPECT_TRUE(callbackExecuted);
}

// Test: WaitFor with timeout and automatic cancellation
TEST(EnhancedFutureTest, WaitForTimeoutAndCancel) {
    auto future = atom::async::makeEnhancedFuture([]() -> int {
        delayMs(2000);  // Simulate a long task (2 seconds)
        return 42;
    });

    // Wait for 1 second, which is less than the task duration
    auto result = future.waitFor(std::chrono::milliseconds(1000));

    // Since the task is not done within the timeout, it should return nullopt
    EXPECT_FALSE(result.has_value());
    EXPECT_TRUE(future.isCancelled());
}

// Test: Retry mechanism with successful retry
TEST(EnhancedFutureTest, RetryWithSuccess) {
    auto future = atom::async::makeEnhancedFuture([]() -> int {
        static int attempts = 0;
        ++attempts;
        delayMs(500);
        if (attempts < 3) {
            throw std::runtime_error("Simulated failure");
        }
        return 42;
    });

    // Retry the task with a maximum of 5 retries
    auto retryFuture = future.retry([](int result) { return result; }, 5);

    // Wait for completion and check the final result after retries
    int result = retryFuture.wait();
    EXPECT_EQ(result, 42);
}

// Test: Retry mechanism failure after max retries
TEST(EnhancedFutureTest, RetryWithFailure) {
    auto future = atom::async::makeEnhancedFuture([]() -> int {
        delayMs(500);
        throw std::runtime_error("Simulated failure");
    });

    // Retry the task with only 2 retries (which should fail)
    auto retryFuture = future.retry([](int result) { return result; }, 2);

    // Expect that an exception is thrown after retries are exhausted
    EXPECT_THROW(retryFuture.wait(), std::runtime_error);
}

// Test: Cancel functionality
TEST(EnhancedFutureTest, CancelFunctionality) {
    auto future = atom::async::makeEnhancedFuture([]() -> int {
        delayMs(2000);  // Simulate a long task (2 seconds)
        return 42;
    });

    // Cancel the future
    future.cancel();

    // Trying to wait should throw a cancellation exception
    EXPECT_THROW(future.wait(), std::runtime_error);
    EXPECT_TRUE(future.isCancelled());
}

// Test: Exception handling
TEST(EnhancedFutureTest, ExceptionHandling) {
    auto future = atom::async::makeEnhancedFuture(
        []() -> int { throw std::runtime_error("Test Exception"); });

    // Expect that waiting for the future throws the exception
    EXPECT_THROW(future.wait(), std::runtime_error);

    // Check that the exception is captured
    auto exception = future.getException();
    EXPECT_NE(exception, nullptr);
}

// Test: Void task with onComplete callback
TEST(EnhancedFutureTest, VoidTaskWithOnCompleteCallback) {
    std::atomic<bool> callbackExecuted{false};

    auto future = atom::async::makeEnhancedFuture([]() {
        delayMs(500);
        std::cout << "Void Task Done" << std::endl;
    });

    // Set a completion callback
    future.onComplete([&]() {
        callbackExecuted = true;
        std::cout << "Callback executed" << std::endl;
    });

    // Wait for the future and check if the callback was invoked
    future.wait();
    EXPECT_TRUE(callbackExecuted);
}

// Test: WaitFor on void future with timeout
TEST(EnhancedFutureTest, VoidWaitForTimeout) {
    auto future = atom::async::makeEnhancedFuture([]() {
        delayMs(2000);  // Simulate a long task (2 seconds)
    });

    // Wait for 1 second, less than task duration
    bool result = future.waitFor(std::chrono::milliseconds(1000));

    // Expect that the future was not completed and was cancelled
    EXPECT_FALSE(result);
    EXPECT_TRUE(future.isCancelled());
}

// Test: Multiple callbacks onComplete
TEST(EnhancedFutureTest, MultipleOnCompleteCallbacks) {
    std::atomic<int> callbackCount{0};

    auto future = atom::async::makeEnhancedFuture([]() -> int {
        delayMs(500);
        return 42;
    });

    // Set two callbacks
    future.onComplete([&](int result) {
        EXPECT_EQ(result, 42);
        ++callbackCount;
    });

    future.onComplete([&](int result) {
        EXPECT_EQ(result, 42);
        ++callbackCount;
    });

    // Wait for the future to complete
    future.wait();

    // Expect both callbacks to have been called
    EXPECT_EQ(callbackCount.load(), 2);
}
