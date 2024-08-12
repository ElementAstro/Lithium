#include "task/container.hpp"
#include <gtest/gtest.h>
#include "task/task.hpp"


using json = nlohmann::json;

class TaskContainerTest : public ::testing::Test {
protected:
    std::shared_ptr<lithium::TaskContainer> container;

    void SetUp() override {
        container = lithium::TaskContainer::createShared();
    }

    std::shared_ptr<Task> createTask(const std::string& name,
                                     const json& params = "{}"_json) {
        return std::make_shared<Task>(name, params,
                                      [](const json&) { return "{}"_json; });
    }

    void addTasks(std::initializer_list<std::shared_ptr<Task>> tasks) {
        for (const auto& task : tasks) {
            container->addTask(task);
        }
    }
};

TEST_F(TaskContainerTest, AddTask) {
    auto task = createTask("test_task");
    container->addTask(task);

    ASSERT_EQ(task, container->getTask("test_task").value());
}

TEST_F(TaskContainerTest, RemoveTask) {
    auto task = createTask("test_task");
    container->addTask(task);
    container->removeTask("test_task");

    ASSERT_FALSE(container->getTask("test_task").has_value());
}

TEST_F(TaskContainerTest, GetTaskCount) {
    ASSERT_EQ(0, container->getTaskCount());

    auto task1 = createTask("test_task1");
    auto task2 = createTask("test_task2");

    addTasks({task1, task2});

    ASSERT_EQ(2, container->getTaskCount());
}

TEST_F(TaskContainerTest, ClearTasks) {
    auto task1 = createTask("test_task1");
    auto task2 = createTask("test_task2");

    addTasks({task1, task2});

    container->clearTasks();

    ASSERT_EQ(0, container->getTaskCount());
}

TEST_F(TaskContainerTest, FindTasks) {
    auto task1 = createTask("test_task1");
    auto task2 = createTask("test_task2");

    task1->setStatus(Task::Status::Running);
    task2->setStatus(Task::Status::Pending);

    addTasks({task1, task2});

    auto result = container->findTasks(1, Task::Status::Running);

    ASSERT_EQ(1, result.size());
    ASSERT_EQ(task1, result[0]);
}

TEST_F(TaskContainerTest, BatchAddTasks) {
    auto task1 = createTask("test_task1");
    auto task2 = createTask("test_task2");

    container->batchAddTasks({task1, task2});

    ASSERT_EQ(task1, container->getTask("test_task1").value());
    ASSERT_EQ(task2, container->getTask("test_task2").value());
}

TEST_F(TaskContainerTest, BatchRemoveTasks) {
    auto task1 = createTask("test_task1");
    auto task2 = createTask("test_task2");

    addTasks({task1, task2});

    container->batchRemoveTasks({"test_task1"});

    ASSERT_FALSE(container->getTask("test_task1").has_value());
    ASSERT_EQ(task2, container->getTask("test_task2").value());
}

TEST_F(TaskContainerTest, BatchModifyTasks) {
    auto task1 = createTask("test_task1");
    auto task2 = createTask("test_task2");

    addTasks({task1, task2});

    container->batchModifyTasks(
        [](auto& task) { task->setStatus(Task::Status::Running); });

    EXPECT_EQ(container->getTask("test_task1").value()->getStatus(),
              Task::Status::Running);
    EXPECT_EQ(container->getTask("test_task2").value()->getStatus(),
              Task::Status::Running);
}

TEST_F(TaskContainerTest, AddOrUpdateTaskParams) {
    json params = {{"key", "value"}};

    container->addOrUpdateTaskParams("test_task", params);

    ASSERT_EQ(params, container->getTaskParams("test_task").value());

    json newParams = {{"key", "new_value"}};

    container->addOrUpdateTaskParams("test_task", newParams);

    ASSERT_EQ(newParams, container->getTaskParams("test_task").value());
}

TEST_F(TaskContainerTest, InsertTaskParams) {
    auto task1 = createTask("test_task1");
    auto task2 = createTask("test_task2");

    json params1 = {{"key1", "value1"}};
    json params2 = {{"key2", "value2"}};

    container->insertTaskParams("test_task1", params1, 0);
    container->insertTaskParams("test_task2", params2, 1);

    ASSERT_EQ(params1, container->getTaskParams("test_task1").value());
    ASSERT_EQ(params2, container->getTaskParams("test_task2").value());
}
