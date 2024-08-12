#include "atom/memory/scoped.hpp"
#include <gtest/gtest.h>


class TestObject {
public:
    TestObject() : value(0) {}
    explicit TestObject(int v) : value(v) {}
    int getValue() const { return value; }
    void setValue(int v) { value = v; }

private:
    int value;
};

// Tests for scoped_ptr
TEST(ScopedPtrTest, DefaultConstructor) {
    scoped_ptr<TestObject> ptr;
    EXPECT_EQ(ptr.get(), nullptr);
    EXPECT_FALSE(ptr);
}

TEST(ScopedPtrTest, PointerConstructor) {
    scoped_ptr<TestObject> ptr(new TestObject(42));
    EXPECT_NE(ptr.get(), nullptr);
    EXPECT_TRUE(ptr);
    EXPECT_EQ(ptr->getValue(), 42);
    EXPECT_EQ((*ptr).getValue(), 42);
}

TEST(ScopedPtrTest, MoveConstructor) {
    scoped_ptr<TestObject> ptr1(new TestObject(42));
    scoped_ptr<TestObject> ptr2(std::move(ptr1));
    EXPECT_EQ(ptr1.get(), nullptr);
    EXPECT_EQ(ptr2->getValue(), 42);
}

TEST(ScopedPtrTest, MoveAssignment) {
    scoped_ptr<TestObject> ptr1(new TestObject(42));
    scoped_ptr<TestObject> ptr2;
    ptr2 = std::move(ptr1);
    EXPECT_EQ(ptr1.get(), nullptr);
    EXPECT_EQ(ptr2->getValue(), 42);
}

TEST(ScopedPtrTest, Reset) {
    scoped_ptr<TestObject> ptr(new TestObject(42));
    ptr.reset(new TestObject(24));
    EXPECT_EQ(ptr->getValue(), 24);
    ptr.reset();
    EXPECT_EQ(ptr.get(), nullptr);
}

TEST(ScopedPtrTest, Release) {
    scoped_ptr<TestObject> ptr(new TestObject(42));
    TestObject* rawPtr = ptr.release();
    EXPECT_EQ(ptr.get(), nullptr);
    EXPECT_EQ(rawPtr->getValue(), 42);
    delete rawPtr;
}

TEST(ScopedPtrTest, Swap) {
    scoped_ptr<TestObject> ptr1(new TestObject(42));
    scoped_ptr<TestObject> ptr2(new TestObject(24));
    swap(ptr1, ptr2);
    EXPECT_EQ(ptr1->getValue(), 24);
    EXPECT_EQ(ptr2->getValue(), 42);
}

TEST(ScopedPtrTest, BoolConversion) {
    scoped_ptr<TestObject> ptr;
    EXPECT_FALSE(ptr);
    ptr.reset(new TestObject(42));
    EXPECT_TRUE(ptr);
}

TEST(ScopedPtrTest, MakeScoped) {
    auto ptr = scoped_ptr<TestObject>::make_scoped(new TestObject(42));
    EXPECT_EQ(ptr->getValue(), 42);
}
