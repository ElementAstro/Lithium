#include "atom/memory/scoped.hpp"
#include <gtest/gtest.h>

// Test fixture for ScopedPtr
class ScopedPtrTest : public ::testing::Test {};

// Helper class to use in tests
class TestObject {
public:
    TestObject() { ++count; }
    ~TestObject() { --count; }

    static int count;
};

int TestObject::count = 0;

// Test default constructor
TEST_F(ScopedPtrTest, DefaultConstructor) {
    ScopedPtr<TestObject> ptr;
    EXPECT_EQ(ptr.get(), nullptr);
    EXPECT_FALSE(ptr);
}

// Test constructor with pointer
TEST_F(ScopedPtrTest, ConstructorWithPointer) {
    ScopedPtr<TestObject> ptr(new TestObject);
    EXPECT_NE(ptr.get(), nullptr);
    EXPECT_TRUE(ptr);
    EXPECT_EQ(TestObject::count, 1);
}

// Test destructor
TEST_F(ScopedPtrTest, Destructor) {
    {
        ScopedPtr<TestObject> ptr(new TestObject);
        EXPECT_EQ(TestObject::count, 1);
    }
    EXPECT_EQ(TestObject::count, 0);
}

// Test move constructor
TEST_F(ScopedPtrTest, MoveConstructor) {
    ScopedPtr<TestObject> ptr1(new TestObject);
    ScopedPtr<TestObject> ptr2(std::move(ptr1));
    EXPECT_EQ(ptr1.get(), nullptr);
    EXPECT_NE(ptr2.get(), nullptr);
    EXPECT_EQ(TestObject::count, 1);
}

// Test move assignment operator
TEST_F(ScopedPtrTest, MoveAssignmentOperator) {
    ScopedPtr<TestObject> ptr1(new TestObject);
    ScopedPtr<TestObject> ptr2;
    ptr2 = std::move(ptr1);
    EXPECT_EQ(ptr1.get(), nullptr);
    EXPECT_NE(ptr2.get(), nullptr);
    EXPECT_EQ(TestObject::count, 1);
}

// Test reset
TEST_F(ScopedPtrTest, Reset) {
    ScopedPtr<TestObject> ptr(new TestObject);
    EXPECT_EQ(TestObject::count, 1);
    ptr.reset();
    EXPECT_EQ(ptr.get(), nullptr);
    EXPECT_EQ(TestObject::count, 0);
}

// Test reset with new pointer
TEST_F(ScopedPtrTest, ResetWithNewPointer) {
    ScopedPtr<TestObject> ptr(new TestObject);
    EXPECT_EQ(TestObject::count, 1);
    ptr.reset(new TestObject);
    EXPECT_EQ(ptr.get(), nullptr);    // The old object should be deleted
    EXPECT_EQ(TestObject::count, 1);  // Only one object should exist
}

// Test release
TEST_F(ScopedPtrTest, Release) {
    ScopedPtr<TestObject> ptr(new TestObject);
    EXPECT_EQ(TestObject::count, 1);
    TestObject* rawPtr = ptr.release();
    EXPECT_EQ(ptr.get(), nullptr);
    EXPECT_EQ(TestObject::count, 1);
    delete rawPtr;
    EXPECT_EQ(TestObject::count, 0);
}

// Test swap
TEST_F(ScopedPtrTest, Swap) {
    ScopedPtr<TestObject> ptr1(new TestObject);
    ScopedPtr<TestObject> ptr2(new TestObject);
    EXPECT_EQ(TestObject::count, 2);
    ptr1.swap(ptr2);
    EXPECT_EQ(TestObject::count, 2);
}

// Test makeScoped
TEST_F(ScopedPtrTest, MakeScoped) {
    auto ptr = makeScoped<TestObject>();
    EXPECT_NE(ptr.get(), nullptr);
    EXPECT_EQ(TestObject::count, 1);
}

// Test custom deleter
TEST_F(ScopedPtrTest, CustomDeleter) {
    bool deleterCalled = false;
    auto customDeleter = [&deleterCalled](TestObject* obj) {
        delete obj;
        deleterCalled = true;
    };
    {
        ScopedPtr<TestObject, decltype(customDeleter)> ptr(new TestObject,
                                                           customDeleter);
        EXPECT_EQ(TestObject::count, 1);
    }
    EXPECT_TRUE(deleterCalled);
    EXPECT_EQ(TestObject::count, 0);
}