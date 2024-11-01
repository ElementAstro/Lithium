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
