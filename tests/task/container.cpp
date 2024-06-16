#include "gtest/gtest.h"
#include "task/container.hpp"

TEST(TaskContainerTest, AddTask) {
    auto container = lithium::TaskContainer::createShared();
    auto task = std::make_shared<SimpleTask>("test_task");
    
    container->addTask(task);
    
    ASSERT_EQ(task, container->getTask("test_task").value());
}

TEST(TaskContainerTest, RemoveTask) {
    auto container = lithium::TaskContainer::createShared();
    auto task = std::make_shared<SimpleTask>("test_task");
    
    container->addTask(task);
    container->removeTask("test_task");
    
    ASSERT_FALSE(container->getTask("test_task").has_value());
}

TEST(TaskContainerTest, GetTaskCount) {
    auto container = lithium::TaskContainer::createShared();
    
    ASSERT_EQ(0, container->getTaskCount());
    
    auto task1 = std::make_shared<SimpleTask>("test_task1");
    auto task2 = std::make_shared<SimpleTask>("test_task2");
    
    container->addTask(task1);
    container->addTask(task2);
    
    ASSERT_EQ(2, container->getTaskCount());
}

TEST(TaskContainerTest, ClearTasks) {
    auto container = lithium::TaskContainer::createShared();
    
    auto task1 = std::make_shared<SimpleTask>("test_task1");
    auto task2 = std::make_shared<SimpleTask>("test_task2");
    
    container->addTask(task1);
    container->addTask(task2);
    
    container->clearTasks();
    
    ASSERT_EQ(0, container->getTaskCount());
}

TEST(TaskContainerTest, FindTasks) {
    auto container = lithium::TaskContainer::createShared();
    
    auto task1 = std::make_shared<SimpleTask>("test_task1");
    auto task2 = std::make_shared<SimpleTask>("test_task2");
    
    task1->setPriority(1);
    task1->setStatus(true);
    
    task2->setPriority(2);
    task2->setStatus(false);
    
    container->addTask(task1);
    container->addTask(task2);
    
    auto result = container->findTasks(1, true);
    
    ASSERT_EQ(1, result.size());
    ASSERT_EQ(task1, result[0]);
}

TEST(TaskContainerTest, SortTasks) {
    auto container = lithium::TaskContainer::createShared();
    
    auto task1 = std::make_shared<SimpleTask>("test_task1");
    auto task2 = std::make_shared<SimpleTask>("test_task2");
    
    task1->setPriority(2);
    task2->setPriority(1);
    
    container->addTask(task1);
    container->addTask(task2);
    
    container->sortTasks([](const auto& task1, const auto& task2) {
        return task1->getPriority() < task2->getPriority();
    });
    
    ASSERT_EQ(task2, container->getAllTasks()[0]);
    ASSERT_EQ(task1, container->getAllTasks()[1]);
}

TEST(TaskContainerTest, BatchAddTasks) {
    auto container = lithium::TaskContainer::createShared();
    
    auto task1 = std::make_shared<SimpleTask>("test_task1");
    auto task2 = std::make_shared<SimpleTask>("test_task2");
    
    container->batchAddTasks({task1, task2});
    
    ASSERT_EQ(task1, container->getTask("test_task1").value());
    ASSERT_EQ(task2, container->getTask("test_task2").value());
}

TEST(TaskContainerTest, BatchRemoveTasks) {
    auto container = lithium::TaskContainer::createShared();
    
    auto task1 = std::make_shared<SimpleTask>("test_task1");
    auto task2 = std::make_shared<SimpleTask>("test_task2");
    
    container->addTask(task1);
    container->addTask(task2);
    
    container->batchRemoveTasks({"test_task1"});
    
    ASSERT_FALSE(container->getTask("test_task1").has_value());
    ASSERT_EQ(task2, container->getTask("test_task2").value());
}

TEST(TaskContainerTest, BatchModifyTasks) {
    auto container = lithium::TaskContainer::createShared();
    
    auto task1 = std::make_shared<SimpleTask>("test_task1");
    auto task2 = std::make_shared<SimpleTask>("test_task2");
    
    container->addTask(task1);
    container->addTask(task2);
    
    container->batchModifyTasks([](auto& task) {
        task->setStatus(false);
    });
    
    ASSERT_FALSE(container->getTask("test_task1").value()->getStatus());
    ASSERT_FALSE(container->getTask("test_task2").value()->getStatus());
}

TEST(TaskContainerTest, AddOrUpdateTaskParams) {
    auto container = lithium::TaskContainer::createShared();
    
    json params = {{"key", "value"}};
    
    container->addOrUpdateTaskParams("test_task", params);
    
    ASSERT_EQ(params, container->getTaskParams("test_task").value());
    
    json newParams = {{"key", "new_value"}};
    
    container->addOrUpdateTaskParams("test_task", newParams);
    
    ASSERT_EQ(newParams, container->getTaskParams("test_task").value());
}

TEST(TaskContainerTest, InsertTaskParams) {
    auto container = lithium::TaskContainer::createShared();
    
    json params1 = {{"key1", "value1"}};
    json params2 = {{"key2", "value2"}};
    
    container->insertTaskParams("test_task1", params1, 0);
    container->insertTaskParams("test_task2", params2, 1);
    
    ASSERT_EQ(params1, container->getTaskParams("test_task1").value());
    ASSERT_EQ(params2, container->getTaskParams("test_task2").value());
}

TEST(TaskContainerTest, ListTaskParams) {
    auto container = lithium::TaskContainer::createShared();
    
    json params1 = {{"key1", "value1"}};
    json params2 = {{"key2", "value2"}};
    
    container->insertTaskParams("test_task1", params1, 0);
    container->insertTaskParams("test_task2", params2, 1);
    
    testing::internal::CaptureStdout();
    
    container->listTaskParams();
    
    std::string output = testing::internal::GetCapturedStdout();
    
    ASSERT_TRUE(output.find("Task name: test_task1") != std::string::npos);
    ASSERT_TRUE(output.find("Task name: test_task2") != std::string::npos);
}