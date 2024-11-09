// FILE: test_short_alloc.hpp
#ifndef ATOM_MEMORY_TEST_SHORT_ALLOC_HPP
#define ATOM_MEMORY_TEST_SHORT_ALLOC_HPP

#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include "atom/memory/short_alloc.hpp"

using namespace atom::memory;

class ArenaTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }
};

TEST_F(ArenaTest, Constructor) {
    Arena<1024> arena;
    EXPECT_EQ(arena.size(), 1024);
    EXPECT_EQ(arena.used(), 0);
    EXPECT_EQ(arena.remaining(), 1024);
}

TEST_F(ArenaTest, AllocateAndDeallocate) {
    Arena<1024> arena;
    void* ptr = arena.allocate(100);
    EXPECT_NE(ptr, nullptr);
    EXPECT_EQ(arena.used(), 100);
    EXPECT_EQ(arena.remaining(), 924);

    arena.deallocate(ptr, 100);
    EXPECT_EQ(arena.used(), 0);
    EXPECT_EQ(arena.remaining(), 1024);
}

TEST_F(ArenaTest, AllocateExceedingSize) {
    Arena<1024> arena;
    EXPECT_THROW(arena.allocate(2048), std::bad_alloc);
}

TEST_F(ArenaTest, Reset) {
    Arena<1024> arena;
    void* ptr = arena.allocate(100);
    EXPECT_NE(ptr, nullptr);
    arena.reset();
    EXPECT_EQ(arena.used(), 0);
    EXPECT_EQ(arena.remaining(), 1024);
}

TEST_F(ArenaTest, ThreadSafety) {
    Arena<1024> arena;
    std::vector<std::thread> threads;

    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&arena]() {
            for (int j = 0; j < 10; ++j) {
                void* ptr = arena.allocate(10);
                arena.deallocate(ptr, 10);
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(arena.used(), 0);
    EXPECT_EQ(arena.remaining(), 1024);
}

class ShortAllocTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }
};

TEST_F(ShortAllocTest, Constructor) {
    Arena<1024> arena;
    ShortAlloc<int, 1024> alloc(arena);
    EXPECT_EQ(alloc.SIZE, 1024);
    EXPECT_EQ(alloc.ALIGNMENT, alignof(std::max_align_t));
}

TEST_F(ShortAllocTest, AllocateAndDeallocate) {
    Arena<1024> arena;
    ShortAlloc<int, 1024> alloc(arena);
    int* ptr = alloc.allocate(10);
    EXPECT_NE(ptr, nullptr);
    EXPECT_EQ(arena.used(), 10 * sizeof(int));
    EXPECT_EQ(arena.remaining(), 1024 - 10 * sizeof(int));

    alloc.deallocate(ptr, 10);
    EXPECT_EQ(arena.used(), 0);
    EXPECT_EQ(arena.remaining(), 1024);
}

TEST_F(ShortAllocTest, AllocateExceedingSize) {
    Arena<1024> arena;
    ShortAlloc<int, 1024> alloc(arena);
    EXPECT_THROW(alloc.allocate(1025), std::bad_alloc);
}

TEST_F(ShortAllocTest, ConstructAndDestroy) {
    Arena<1024> arena;
    ShortAlloc<int, 1024> alloc(arena);
    int* ptr = alloc.allocate(1);
    alloc.construct(ptr, 42);
    EXPECT_EQ(*ptr, 42);
    alloc.destroy(ptr);
    alloc.deallocate(ptr, 1);
}

TEST_F(ShortAllocTest, ThreadSafety) {
    Arena<1024> arena;
    ShortAlloc<int, 1024> alloc(arena);
    std::vector<std::thread> threads;

    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&alloc]() {
            for (int j = 0; j < 10; ++j) {
                int* ptr = alloc.allocate(10);
                alloc.deallocate(ptr, 10);
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(arena.used(), 0);
    EXPECT_EQ(arena.remaining(), 1024);
}

#endif  // ATOM_MEMORY_TEST_SHORT_ALLOC_HPP
