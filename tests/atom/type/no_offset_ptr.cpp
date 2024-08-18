#include "atom/type//no_offset_ptr.hpp"
#include <gtest/gtest.h>

struct TestObject {
    int value;
    TestObject() = default;
    explicit TestObject(int val) : value(val) {}
    TestObject(const TestObject&) = default;
    TestObject& operator=(const TestObject&) = default;
    ~TestObject() = default;
};

TEST(UnshiftedPtrTest, DefaultConstructor) {
    UnshiftedPtr<TestObject> ptr;
    EXPECT_EQ(ptr->value, 0);  // assuming TestObject has a default value of 0
}

TEST(UnshiftedPtrTest, ParameterizedConstructor) {
    UnshiftedPtr<TestObject> ptr(42);
    EXPECT_EQ(ptr->value, 42);
}

TEST(UnshiftedPtrTest, Reset) {
    UnshiftedPtr<TestObject> ptr(42);
    ptr.reset(84);
    EXPECT_EQ(ptr->value, 84);
}

TEST(UnshiftedPtrTest, Release) {
    UnshiftedPtr<TestObject> ptr(42);
    TestObject* raw_ptr = ptr.release();
    EXPECT_EQ(raw_ptr->value, 42);
    delete raw_ptr;
}

TEST(UnshiftedPtrTest, MoveConstructorDisabled) {
    static_assert(!std::is_move_constructible_v<UnshiftedPtr<TestObject>>,
                  "UnshiftedPtr should not be movable");
}

TEST(UnshiftedPtrTest, CopyConstructorDisabled) {
    static_assert(!std::is_copy_constructible_v<UnshiftedPtr<TestObject>>,
                  "UnshiftedPtr should not be copyable");
}