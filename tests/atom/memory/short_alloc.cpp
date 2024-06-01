#include "atom/memory/short_alloc.hpp"
#include <gtest/gtest.h>


// Tests for arena class
TEST(ArenaTest, AllocateAndDeallocate) {
    arena<1024> a;

    void* p1 = a.allocate(128);
    ASSERT_NE(p1, nullptr);
    EXPECT_EQ(a.used(), 128);

    void* p2 = a.allocate(256);
    ASSERT_NE(p2, nullptr);
    EXPECT_EQ(a.used(), 384);

    a.deallocate(p2, 256);
    EXPECT_EQ(a.used(), 128);

    a.deallocate(p1, 128);
    EXPECT_EQ(a.used(), 0);
}

TEST(ArenaTest, AllocateExceedingSize) {
    arena<1024> a;

    EXPECT_NO_THROW(a.allocate(512));
    EXPECT_THROW(a.allocate(1024), std::bad_alloc);
}

TEST(ArenaTest, Reset) {
    arena<1024> a;

    a.allocate(512);
    EXPECT_EQ(a.used(), 512);

    a.reset();
    EXPECT_EQ(a.used(), 0);
}

// Tests for short_alloc class
TEST(ShortAllocTest, AllocateAndDeallocate) {
    arena<1024> a;
    short_alloc<int, 1024> alloc(a);

    int* p1 = alloc.allocate(10);
    ASSERT_NE(p1, nullptr);

    for (int i = 0; i < 10; ++i) {
        new (&p1[i]) int(i);
        EXPECT_EQ(p1[i], i);
    }

    alloc.deallocate(p1, 10);
}

TEST(ShortAllocTest, RebindAllocator) {
    arena<1024> a;
    short_alloc<int, 1024> alloc(a);

    // Rebinding allocator to another type
    short_alloc<double, 1024>::rebind<int>::other int_alloc = alloc;

    int* p1 = int_alloc.allocate(10);
    ASSERT_NE(p1, nullptr);

    for (int i = 0; i < 10; ++i) {
        new (&p1[i]) int(i);
        EXPECT_EQ(p1[i], i);
    }

    int_alloc.deallocate(p1, 10);
}

TEST(ShortAllocTest, EqualityComparison) {
    arena<1024> a;
    short_alloc<int, 1024> alloc1(a);
    short_alloc<int, 1024> alloc2(a);

    EXPECT_TRUE(alloc1 == alloc2);
    EXPECT_FALSE(alloc1 != alloc2);

    arena<1024> b;
    short_alloc<int, 1024> alloc3(b);

    EXPECT_FALSE(alloc1 == alloc3);
    EXPECT_TRUE(alloc1 != alloc3);
}
