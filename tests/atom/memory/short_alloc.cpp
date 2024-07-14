#include <gtest/gtest.h>

#include "atom/memory/short_alloc.hpp"

using namespace std;
using namespace atom::memory;

TEST(ArenaTest, BasicAllocation) {
    constexpr size_t N = 1024;
    Arena<N> arena;
    void* p1 = arena.allocate(128);
    void* p2 = arena.allocate(128);
    EXPECT_NE(p1, nullptr);
    EXPECT_NE(p2, nullptr);
    EXPECT_EQ(arena.used(), 256);
}

TEST(ArenaTest, Alignment) {
    constexpr size_t N = 1024;
    constexpr size_t alignment = alignof(max_align_t);
    Arena<N, alignment> arena;
    void* p1 = arena.allocate(128);
    void* p2 = arena.allocate(128);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(p1) % alignment, 0);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(p2) % alignment, 0);
}

TEST(ArenaTest, Reset) {
    constexpr size_t N = 1024;
    Arena<N> arena;
    void* p1 = arena.allocate(128);
    void* p2 = arena.allocate(128);
    arena.reset();
    void* p3 = arena.allocate(128);
    EXPECT_NE(p3, nullptr);
    EXPECT_EQ(arena.used(), 128);
}

TEST(ShortAllocTest, BasicAllocation) {
    constexpr size_t N = 1024;
    Arena<N> arena;
    ShortAlloc<int, N> alloc(arena);
    int* p1 = alloc.allocate(10);
    EXPECT_NE(p1, nullptr);
    alloc.deallocate(p1, 10);
}

TEST(ShortAllocTest, ConstructAndDestroy) {
    constexpr size_t N = 1024;
    Arena<N> arena;
    ShortAlloc<int, N> alloc(arena);
    int* p = alloc.allocate(1);
    alloc.construct(p, 42);
    EXPECT_EQ(*p, 42);
    alloc.destroy(p);
    alloc.deallocate(p, 1);
}