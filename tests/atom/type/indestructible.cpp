#include "atom/type/indestructible.hpp"
#include <gtest/gtest.h>

TEST(IndestructibleTest, Constructor) {
    Indestructible<int> obj(10);
    EXPECT_EQ(obj.get(), 10);
}

TEST(IndestructibleTest, CopyConstructor) {
    Indestructible<int> obj1(10);
    Indestructible<int> obj2(obj1);
    EXPECT_EQ(obj2.get(), 10);
}

TEST(IndestructibleTest, MoveConstructor) {
    Indestructible<int> obj1(10);
    Indestructible<int> obj2(std::move(obj1));
    EXPECT_EQ(obj2.get(), 10);
}

TEST(IndestructibleTest, CopyAssignment) {
    Indestructible<int> obj1(10);
    Indestructible<int> obj2(20);
    obj2 = obj1;
    EXPECT_EQ(obj2.get(), 10);
}

TEST(IndestructibleTest, MoveAssignment) {
    Indestructible<int> obj1(10);
    Indestructible<int> obj2(20);
    obj2 = std::move(obj1);
    EXPECT_EQ(obj2.get(), 10);
}

TEST(IndestructibleTest, Destructor) {
    bool destructed = false;
    struct TestObject {
        ~TestObject() { destructed = true; }
    };
    Indestructible<TestObject> obj(TestObject());
    EXPECT_FALSE(destructed);
    // obj goes out of scope here, destructor should be called
    EXPECT_TRUE(destructed);
}