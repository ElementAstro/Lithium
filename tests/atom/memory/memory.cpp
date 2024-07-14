#include "atom/memory/memory.hpp"
#include <gtest/gtest.h>
// Test structure to allocate using MemoryPool
struct TestStruct {
    int a;
    double b;

    TestStruct() : a(0), b(0.0) {}
    TestStruct(int a, double b) : a(a), b(b) {}
};

// Tests for MemoryPool class
TEST(MemoryPoolTest, AllocateAndDeallocate) {
    MemoryPool<TestStruct> pool;

    // Allocate memory for one TestStruct
    TestStruct* ptr = pool.allocate(1);
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(ptr->a, 0);
    EXPECT_EQ(ptr->b, 0.0);

    // Use placement new to construct the object
    new (ptr) TestStruct(42, 3.14);
    EXPECT_EQ(ptr->a, 42);
    EXPECT_EQ(ptr->b, 3.14);

    // Destruct the object manually since we used placement new
    ptr->~TestStruct();

    // Deallocate the memory
    pool.deallocate(ptr, 1);
}

TEST(MemoryPoolTest, AllocateMultiple) {
    const size_t numObjects = 10;
    MemoryPool<TestStruct> pool;

    // Allocate memory for multiple TestStruct objects
    TestStruct* ptr = pool.allocate(numObjects);
    ASSERT_NE(ptr, nullptr);

    // Use placement new to construct the objects
    for (size_t i = 0; i < numObjects; ++i) {
        new (ptr + i) TestStruct(static_cast<int>(i), i * 1.1);
    }

    // Verify the objects
    for (size_t i = 0; i < numObjects; ++i) {
        EXPECT_EQ(ptr[i].a, static_cast<int>(i));
        EXPECT_EQ(ptr[i].b, i * 1.1);
    }

    // Destruct the objects manually since we used placement new
    for (size_t i = 0; i < numObjects; ++i) {
        (ptr + i)->~TestStruct();
    }

    // Deallocate the memory
    pool.deallocate(ptr, numObjects);
}

TEST(MemoryPoolTest, AllocateExceedingBlockSize) {
    const size_t largeSize = 4096 / sizeof(TestStruct) + 1;
    MemoryPool<TestStruct> pool;

    // Allocate memory exceeding the block size
    TestStruct* ptr = pool.allocate(largeSize);
    ASSERT_NE(ptr, nullptr);

    // Use placement new to construct one object
    new (ptr) TestStruct(123, 4.56);
    EXPECT_EQ(ptr->a, 123);
    EXPECT_EQ(ptr->b, 4.56);

    // Destruct the object manually since we used placement new
    ptr->~TestStruct();

    // Deallocate the memory
    pool.deallocate(ptr, largeSize);
}

TEST(MemoryPoolTest, ReuseMemory) {
    MemoryPool<TestStruct> pool;

    // Allocate memory for one TestStruct
    TestStruct* ptr1 = pool.allocate(1);
    ASSERT_NE(ptr1, nullptr);

    // Destruct the object manually since we used placement new
    ptr1->~TestStruct();

    // Deallocate the memory
    pool.deallocate(ptr1, 1);

    // Allocate memory again and check if the same memory is reused
    TestStruct* ptr2 = pool.allocate(1);
    ASSERT_NE(ptr2, nullptr);
    EXPECT_EQ(ptr1, ptr2);

    // Use placement new to construct the object
    new (ptr2) TestStruct(78, 9.10);
    EXPECT_EQ(ptr2->a, 78);
    EXPECT_EQ(ptr2->b, 9.10);

    // Destruct the object manually since we used placement new
    ptr2->~TestStruct();

    // Deallocate the memory
    pool.deallocate(ptr2, 1);
}