// FILE: test_object.hpp
#ifndef ATOM_MEMORY_TEST_OBJECT_POOL_HPP
#define ATOM_MEMORY_TEST_OBJECT_POOL_HPP

#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include "atom/memory/object.hpp"

using namespace atom::memory;

// Sample Resettable class for testing
class TestObject {
public:
    void reset() { value = 0; }

    int value = 0;
};

class ObjectPoolTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }
};

TEST_F(ObjectPoolTest, Constructor) {
    ObjectPool<TestObject> pool(10);
    EXPECT_EQ(pool.available(), 10);
    EXPECT_EQ(pool.size(), 0);
}

TEST_F(ObjectPoolTest, AcquireAndRelease) {
    ObjectPool<TestObject> pool(10);
    auto obj = pool.acquire();
    EXPECT_NE(obj, nullptr);
    EXPECT_EQ(pool.available(), 9);
    EXPECT_EQ(pool.size(), 1);

    obj->value = 42;
    obj.reset();
    EXPECT_EQ(pool.available(), 10);
    EXPECT_EQ(pool.size(), 1);
    EXPECT_EQ(pool.inUseCount(), 0);

    auto obj2 = pool.acquire();
    EXPECT_EQ(obj2->value, 0);  // Ensure the object was reset
}

TEST_F(ObjectPoolTest, TryAcquireFor) {
    ObjectPool<TestObject> pool(1);
    auto obj = pool.acquire();
    EXPECT_NE(obj, nullptr);
    EXPECT_EQ(pool.available(), 0);

    auto obj2 = pool.tryAcquireFor(std::chrono::milliseconds(100));
    EXPECT_FALSE(obj2.has_value());

    obj.reset();
    auto obj3 = pool.tryAcquireFor(std::chrono::milliseconds(100));
    EXPECT_TRUE(obj3.has_value());
}

TEST_F(ObjectPoolTest, Prefill) {
    ObjectPool<TestObject> pool(10);
    pool.prefill(5);
    EXPECT_EQ(pool.available(), 10);
    EXPECT_EQ(pool.size(), 5);

    auto obj = pool.acquire();
    EXPECT_NE(obj, nullptr);
    EXPECT_EQ(pool.available(), 9);
    EXPECT_EQ(pool.size(), 6);
}

TEST_F(ObjectPoolTest, Clear) {
    ObjectPool<TestObject> pool(10);
    pool.prefill(5);
    EXPECT_EQ(pool.available(), 10);
    EXPECT_EQ(pool.size(), 5);

    pool.clear();
    EXPECT_EQ(pool.available(), 10);
    EXPECT_EQ(pool.size(), 0);
}

TEST_F(ObjectPoolTest, Resize) {
    ObjectPool<TestObject> pool(10);
    pool.prefill(5);
    EXPECT_EQ(pool.available(), 10);
    EXPECT_EQ(pool.size(), 5);

    pool.resize(20);
    EXPECT_EQ(pool.available(), 20);
    EXPECT_EQ(pool.size(), 5);

    pool.resize(5);
    EXPECT_EQ(pool.available(), 5);
    EXPECT_EQ(pool.size(), 5);
}

TEST_F(ObjectPoolTest, ApplyToAll) {
    ObjectPool<TestObject> pool(10);
    pool.prefill(5);

    pool.applyToAll([](TestObject& obj) { obj.value = 42; });

    for (int i = 0; i < 5; ++i) {
        auto obj = pool.acquire();
        EXPECT_EQ(obj->value, 42);
    }
}

TEST_F(ObjectPoolTest, InUseCount) {
    ObjectPool<TestObject> pool(10);
    EXPECT_EQ(pool.inUseCount(), 0);

    auto obj = pool.acquire();
    EXPECT_EQ(pool.inUseCount(), 1);

    obj.reset();
    EXPECT_EQ(pool.inUseCount(), 0);
}

TEST_F(ObjectPoolTest, ThreadSafety) {
    ObjectPool<TestObject> pool(10);
    std::vector<std::thread> threads;

    threads.reserve(10);
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&pool]() {
            for (int j = 0; j < 100; ++j) {
                auto obj = pool.acquire();
                obj->value = j;
                obj.reset();
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(pool.available(), 10);
    EXPECT_EQ(pool.size(), 0);
}

#endif  // ATOM_MEMORY_TEST_OBJECT_POOL_HPP
