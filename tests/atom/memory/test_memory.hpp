// FILE: test_memory.hpp
#ifndef ATOM_MEMORY_TEST_MEMORY_POOL_HPP
#define ATOM_MEMORY_TEST_MEMORY_POOL_HPP

#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include "atom/memory/memory.hpp"

using namespace atom::memory;

class MemoryPoolTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }
};

TEST_F(MemoryPoolTest, Constructor) {
    MemoryPool<int> pool;
    EXPECT_EQ(pool.getTotalAllocated(), 0);
    EXPECT_EQ(pool.getTotalAvailable(), 0);
}

TEST_F(MemoryPoolTest, AllocateAndDeallocate) {
    MemoryPool<int> pool;
    int* ptr = pool.allocate(10);
    EXPECT_NE(ptr, nullptr);
    EXPECT_EQ(pool.getTotalAllocated(), 10 * sizeof(int));
    EXPECT_EQ(pool.getTotalAvailable(), 4096 - 10 * sizeof(int));

    pool.deallocate(ptr, 10);
    EXPECT_EQ(pool.getTotalAllocated(), 0);
    EXPECT_EQ(pool.getTotalAvailable(), 4096);
}

TEST_F(MemoryPoolTest, AllocateExceedingBlockSize) {
    MemoryPool<int> pool;
    EXPECT_THROW(pool.allocate(4097), MemoryPoolException);
}

TEST_F(MemoryPoolTest, Reset) {
    MemoryPool<int> pool;
    int* ptr = pool.allocate(10);
    EXPECT_NE(ptr, nullptr);
    pool.reset();
    EXPECT_EQ(pool.getTotalAllocated(), 0);
    EXPECT_EQ(pool.getTotalAvailable(), 0);
}

TEST_F(MemoryPoolTest, AllocateFromPool) {
    MemoryPool<int> pool;
    int* ptr1 = pool.allocate(10);
    int* ptr2 = pool.allocate(20);
    EXPECT_NE(ptr1, nullptr);
    EXPECT_NE(ptr2, nullptr);
    EXPECT_EQ(pool.getTotalAllocated(), 30 * sizeof(int));
    EXPECT_EQ(pool.getTotalAvailable(), 4096 - 30 * sizeof(int));

    pool.deallocate(ptr1, 10);
    pool.deallocate(ptr2, 20);
    EXPECT_EQ(pool.getTotalAllocated(), 0);
    EXPECT_EQ(pool.getTotalAvailable(), 4096);
}

TEST_F(MemoryPoolTest, AllocateFromChunk) {
    MemoryPool<int> pool;
    int* ptr1 = pool.allocate(1024);
    int* ptr2 = pool.allocate(1024);
    EXPECT_NE(ptr1, nullptr);
    EXPECT_NE(ptr2, nullptr);
    EXPECT_EQ(pool.getTotalAllocated(), 2048 * sizeof(int));
    EXPECT_EQ(pool.getTotalAvailable(), 4096 - 2048 * sizeof(int));

    pool.deallocate(ptr1, 1024);
    pool.deallocate(ptr2, 1024);
    EXPECT_EQ(pool.getTotalAllocated(), 0);
    EXPECT_EQ(pool.getTotalAvailable(), 4096);
}

TEST_F(MemoryPoolTest, ThreadSafety) {
    MemoryPool<int> pool;
    std::vector<std::thread> threads;

    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&pool]() {
            for (int j = 0; j < 100; ++j) {
                int* ptr = pool.allocate(10);
                pool.deallocate(ptr, 10);
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(pool.getTotalAllocated(), 0);
    EXPECT_EQ(pool.getTotalAvailable(), 4096);
}

#endif  // ATOM_MEMORY_TEST_MEMORY_POOL_HPP