#include "atom/async/safetype.hpp"
#include <gtest/gtest.h>
#include <algorithm>
#include <thread>
#include <vector>

using namespace atom::async;

class LockFreeStackTest : public ::testing::Test {
protected:
    LockFreeStack<int> stack;

    void SetUp() override {
        // 可以在这里初始化 stack，或者在每个测试中初始化
    }

    void TearDown() override {
        // 每次测试后清理
    }
};

TEST_F(LockFreeStackTest, InitialState) {
    EXPECT_TRUE(stack.empty());
    EXPECT_EQ(stack.size(), 0);
}

TEST_F(LockFreeStackTest, PushAndSize) {
    stack.push(1);
    stack.push(2);
    stack.push(3);

    EXPECT_EQ(stack.size(), 3);
    EXPECT_FALSE(stack.empty());
}

TEST_F(LockFreeStackTest, Pop) {
    stack.push(1);
    stack.push(2);

    auto value = stack.pop();
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), 2);

    value = stack.pop();
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), 1);

    value = stack.pop();
    EXPECT_FALSE(value.has_value());
}

TEST_F(LockFreeStackTest, Top) {
    stack.push(1);
    stack.push(2);
    stack.push(3);

    auto value = stack.top();
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), 3);

    stack.pop();
    value = stack.top();
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), 2);
}

TEST_F(LockFreeStackTest, Empty) {
    EXPECT_TRUE(stack.empty());
    stack.push(1);
    EXPECT_FALSE(stack.empty());
    stack.pop();
    EXPECT_TRUE(stack.empty());
}

TEST_F(LockFreeStackTest, ApproximateSize) {
    stack.push(1);
    stack.push(2);
    EXPECT_EQ(stack.size(), 2);
    stack.pop();
    EXPECT_EQ(stack.size(), 1);
}

TEST_F(LockFreeStackTest, ConcurrentPushAndPop) {
    const int numThreads = 4;
    const int numIterations = 1000;
    std::vector<std::thread> threads;

    auto push_function = [this](int id, int numIterations) {
        for (int i = 0; i < numIterations; ++i) {
            stack.push(i + id * 1000);
        }
    };

    auto pop_function = [this](int numIterations) {
        for (int i = 0; i < numIterations; ++i) {
            stack.pop();
        }
    };

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(push_function, i, numIterations);
    }

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(pop_function, numIterations);
    }

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_TRUE(stack.empty());
}

TEST_F(LockFreeStackTest, TopEmptyStack) {
    auto value = stack.top();
    EXPECT_FALSE(value.has_value());
}

TEST_F(LockFreeStackTest, PopEmptyStack) {
    auto value = stack.pop();
    EXPECT_FALSE(value.has_value());
}

class LockFreeHashTableTest : public ::testing::Test {
protected:
    LockFreeHashTable<int, std::string> table;

    void SetUp() override {
        // 初始化哈希表
    }

    void TearDown() override {
        // 测试后清理
    }
};

TEST_F(LockFreeHashTableTest, InitialState) { EXPECT_TRUE(table.empty()); }

TEST_F(LockFreeHashTableTest, InsertAndFind) {
    table.insert(1, "one");
    table.insert(2, "two");

    auto value = table.find(1);
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), "one");

    value = table.find(2);
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), "two");

    value = table.find(3);
    EXPECT_FALSE(value.has_value());
}

TEST_F(LockFreeHashTableTest, Erase) {
    table.insert(1, "one");
    table.insert(2, "two");

    table.erase(1);
    auto value = table.find(1);
    EXPECT_FALSE(value.has_value());

    value = table.find(2);
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), "two");

    table.erase(2);
    value = table.find(2);
    EXPECT_FALSE(value.has_value());
}

TEST_F(LockFreeHashTableTest, Empty) {
    EXPECT_TRUE(table.empty());
    table.insert(1, "one");
    EXPECT_FALSE(table.empty());
    table.erase(1);
    EXPECT_TRUE(table.empty());
}

TEST_F(LockFreeHashTableTest, Size) {
    EXPECT_EQ(table.size(), 0);
    table.insert(1, "one");
    table.insert(2, "two");
    EXPECT_EQ(table.size(), 2);
    table.erase(1);
    EXPECT_EQ(table.size(), 1);
    table.erase(2);
    EXPECT_EQ(table.size(), 0);
}

TEST_F(LockFreeHashTableTest, Clear) {
    table.insert(1, "one");
    table.insert(2, "two");
    table.clear();
    EXPECT_TRUE(table.empty());
    EXPECT_EQ(table.size(), 0);
}

TEST_F(LockFreeHashTableTest, ConcurrentInsertAndFind) {
    const int numThreads = 4;
    const int numIterations = 1000;
    std::vector<std::thread> threads;

    auto insert_function = [this](int id, int numIterations) {
        for (int i = 0; i < numIterations; ++i) {
            table.insert(i + id * 1000,
                         "value" + std::to_string(i + id * 1000));
        }
    };

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(insert_function, i, numIterations);
    }

    for (auto& t : threads) {
        t.join();
    }

    for (int i = 0; i < numThreads; ++i) {
        for (int j = 0; j < numIterations; ++j) {
            auto value = table.find(j + i * 1000);
            ASSERT_TRUE(value.has_value());
            EXPECT_EQ(value.value(), "value" + std::to_string(j + i * 1000));
        }
    }
}

TEST_F(LockFreeHashTableTest, Iterator) {
    table.insert(1, "one");
    table.insert(2, "two");
    table.insert(3, "three");

    auto it = table.begin();
    std::vector<std::pair<int, std::string>> elements;
    while (it != table.end()) {
        elements.push_back(*it);
        ++it;
    }

    std::vector<std::pair<int, std::string>> expected = {
        {1, "one"}, {2, "two"}, {3, "three"}};

    EXPECT_EQ(elements.size(), expected.size());
    for (const auto& elem : expected) {
        EXPECT_NE(std::find(elements.begin(), elements.end(), elem),
                  elements.end());
    }
}

class ThreadSafeVectorTest : public ::testing::Test {
protected:
    ThreadSafeVector<int> vec;

    void SetUp() override {}

    void TearDown() override {}
};

TEST_F(ThreadSafeVectorTest, InitialState) {
    EXPECT_TRUE(vec.empty());
    EXPECT_EQ(vec.getSize(), 0);
    EXPECT_GE(vec.getCapacity(), 16);  // 默认初始容量
}

TEST_F(ThreadSafeVectorTest, PushBackAndSize) {
    vec.pushBack(1);
    vec.pushBack(2);
    vec.pushBack(3);

    EXPECT_EQ(vec.getSize(), 3);
    EXPECT_FALSE(vec.empty());
}

TEST_F(ThreadSafeVectorTest, PopBack) {
    vec.pushBack(1);
    vec.pushBack(2);

    auto value = vec.popBack();
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), 2);

    value = vec.popBack();
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), 1);

    value = vec.popBack();
    EXPECT_FALSE(value.has_value());
}

TEST_F(ThreadSafeVectorTest, AtMethod) {
    vec.pushBack(1);
    vec.pushBack(2);
    vec.pushBack(3);

    auto value = vec.at(0);
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), 1);

    value = vec.at(1);
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), 2);

    value = vec.at(2);
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), 3);

    value = vec.at(3);
    EXPECT_FALSE(value.has_value());
}

TEST_F(ThreadSafeVectorTest, Clear) {
    vec.pushBack(1);
    vec.pushBack(2);
    vec.clear();

    EXPECT_TRUE(vec.empty());
    EXPECT_EQ(vec.getSize(), 0);
}

TEST_F(ThreadSafeVectorTest, ResizeAndCapacity) {
    for (int i = 0; i < 20; ++i) {
        vec.pushBack(i);
    }

    EXPECT_EQ(vec.getSize(), 20);
    EXPECT_GE(vec.getCapacity(), 20);
}

TEST_F(ThreadSafeVectorTest, FrontAndBack) {
    vec.pushBack(1);
    vec.pushBack(2);
    vec.pushBack(3);

    EXPECT_EQ(vec.front(), 1);
    EXPECT_EQ(vec.back(), 3);
}

TEST_F(ThreadSafeVectorTest, ShrinkToFit) {
    for (int i = 0; i < 20; ++i) {
        vec.pushBack(i);
    }

    size_t old_capacity = vec.getCapacity();
    vec.shrinkToFit();
    EXPECT_EQ(vec.getSize(), vec.getCapacity());
    EXPECT_LT(vec.getCapacity(), old_capacity);
}

TEST_F(ThreadSafeVectorTest, OperatorSquareBrackets) {
    vec.pushBack(1);
    vec.pushBack(2);
    vec.pushBack(3);

    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 2);
    EXPECT_EQ(vec[2], 3);

    EXPECT_THROW(vec[3], std::out_of_range);
}

TEST_F(ThreadSafeVectorTest, ConcurrentPushBack) {
    const int numThreads = 4;
    const int numIterations = 1000;
    std::vector<std::thread> threads;

    auto push_function = [this](int start, int numIterations) {
        for (int i = 0; i < numIterations; ++i) {
            vec.pushBack(start + i);
        }
    };

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(push_function, i * numIterations, numIterations);
    }

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(vec.getSize(), numThreads * numIterations);
}

TEST_F(ThreadSafeVectorTest, ConcurrentPopBack) {
    const int numThreads = 4;
    const int numIterations = 1000;
    std::vector<std::thread> threads;

    for (int i = 0; i < numThreads * numIterations; ++i) {
        vec.pushBack(i);
    }

    auto pop_function = [this](int numIterations) {
        for (int i = 0; i < numIterations; ++i) {
            vec.popBack();
        }
    };

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(pop_function, numIterations);
    }

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_TRUE(vec.empty());
}

TEST_F(ThreadSafeVectorTest, PushBackResize) {
    vec.clear();
    size_t initial_capacity = vec.getCapacity();
    for (size_t i = 0; i < initial_capacity + 1; ++i) {
        vec.pushBack(static_cast<int>(i));
    }
    EXPECT_GT(vec.getCapacity(), initial_capacity);
    EXPECT_EQ(vec.getSize(), initial_capacity + 1);
}

TEST_F(ThreadSafeVectorTest, PopBackEmpty) {
    vec.clear();
    auto value = vec.popBack();
    EXPECT_FALSE(value.has_value());
}

class LockFreeListTest : public ::testing::Test {
protected:
    LockFreeList<int> list;

    void SetUp() override {
        // 初始化列表
    }

    void TearDown() override {
        // 测试后清理
    }
};

TEST_F(LockFreeListTest, InitialState) { EXPECT_TRUE(list.empty()); }

TEST_F(LockFreeListTest, PushFrontAndEmpty) {
    list.pushFront(1);
    EXPECT_FALSE(list.empty());
}

TEST_F(LockFreeListTest, PopFront) {
    list.pushFront(1);
    list.pushFront(2);

    auto value = list.popFront();
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), 2);

    value = list.popFront();
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), 1);

    value = list.popFront();
    EXPECT_FALSE(value.has_value());
}

TEST_F(LockFreeListTest, Iterator) {
    list.pushFront(1);
    list.pushFront(2);
    list.pushFront(3);

    auto it = list.begin();
    EXPECT_EQ(*it, 3);
    ++it;
    EXPECT_EQ(*it, 2);
    ++it;
    EXPECT_EQ(*it, 1);
    ++it;
    EXPECT_EQ(it, list.end());
}

TEST_F(LockFreeListTest, ConcurrentPushAndPop) {
    const int numThreads = 4;
    const int numIterations = 1000;
    std::vector<std::thread> threads;

    auto push_function = [this](int id, int numIterations) {
        for (int i = 0; i < numIterations; ++i) {
            list.pushFront(i + id * 1000);
        }
    };

    auto pop_function = [this](int numIterations) {
        for (int i = 0; i < numIterations; ++i) {
            list.popFront();
        }
    };

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(push_function, i, numIterations);
    }

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(pop_function, numIterations);
    }

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_TRUE(list.empty());
}

TEST_F(LockFreeListTest, FrontEmptyList) {
    auto value = list.popFront();
    EXPECT_FALSE(value.has_value());
}

TEST_F(LockFreeListTest, IterateEmptyList) {
    auto it = list.begin();
    EXPECT_EQ(it, list.end());
}