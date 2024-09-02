#include "atom/type/indestructible.hpp"
#include <gtest/gtest.h>
#include <list>
#include <set>
#include <string>
#include <vector>

struct TestStruct {
    int value;
    TestStruct(int v) : value(v) {}
    ~TestStruct() { value = -1; }  // Mark destruction
};

// Test construction
TEST(IndestructibleTest, Constructible) {
    Indestructible<TestStruct> obj(std::in_place, 42);
    EXPECT_EQ(obj.get().value, 42);
}

// Test copy construction
TEST(IndestructibleTest, CopyConstructible) {
    Indestructible<TestStruct> obj1(std::in_place, 42);
    Indestructible<TestStruct> obj2(obj1);
    EXPECT_EQ(obj2.get().value, 42);
}

// Test move construction
TEST(IndestructibleTest, MoveConstructible) {
    Indestructible<TestStruct> obj1(std::in_place, 42);
    Indestructible<TestStruct> obj2(std::move(obj1));
    EXPECT_EQ(obj2.get().value, 42);
}

// Test copy assignment
TEST(IndestructibleTest, CopyAssignable) {
    Indestructible<TestStruct> obj1(std::in_place, 42);
    Indestructible<TestStruct> obj2(std::in_place, 0);
    obj2 = obj1;
    EXPECT_EQ(obj2.get().value, 42);
}

// Test move assignment
TEST(IndestructibleTest, MoveAssignable) {
    Indestructible<TestStruct> obj1(std::in_place, 42);
    Indestructible<TestStruct> obj2(std::in_place, 0);
    obj2 = std::move(obj1);
    EXPECT_EQ(obj2.get().value, 42);
}

// Test destruction
TEST(IndestructibleTest, Destruction) {
    TestStruct* ptr = nullptr;
    {
        Indestructible<TestStruct> obj(std::in_place, 42);
        ptr = &obj.get();
        EXPECT_EQ(ptr->value, 42);
    }
    EXPECT_EQ(ptr->value, -1);  // Ensure destructor was called
}

// Test non-trivially destructible type
struct NonTrivialType {
    NonTrivialType() = default;
    ~NonTrivialType() {}
};

TEST(IndestructibleTest, NonTriviallyDestructible) {
    Indestructible<NonTrivialType> obj(std::in_place);
    EXPECT_NO_THROW(obj.~Indestructible());
}

// Test usage as a pointer
TEST(IndestructibleTest, PointerUsage) {
    Indestructible<TestStruct> obj(std::in_place, 42);
    EXPECT_EQ(obj->value, 42);
}

// Test usage as a reference
TEST(IndestructibleTest, ReferenceUsage) {
    Indestructible<TestStruct> obj(std::in_place, 42);
    TestStruct& ref = obj;
    EXPECT_EQ(ref.value, 42);
}

// Test interaction with standard containers
TEST(IndestructibleTest, VectorWithIndestructible) {
    std::vector<Indestructible<TestStruct>> vec;
    vec.emplace_back(std::in_place, 1);
    vec.emplace_back(std::in_place, 2);
    vec.emplace_back(std::in_place, 3);

    EXPECT_EQ(vec[0]->value, 1);
    EXPECT_EQ(vec[1]->value, 2);
    EXPECT_EQ(vec[2]->value, 3);
}

TEST(IndestructibleTest, ListWithIndestructible) {
    std::list<Indestructible<TestStruct>> lst;
    lst.emplace_back(std::in_place, 1);
    lst.emplace_back(std::in_place, 2);
    lst.emplace_back(std::in_place, 3);

    auto it = lst.begin();
    EXPECT_EQ(it->get().value, 1);
    ++it;
    EXPECT_EQ(it->get().value, 2);
    ++it;
    EXPECT_EQ(it->get().value, 3);
}

TEST(IndestructibleTest, SetWithIndestructible) {
    std::set<Indestructible<int>> s;
    s.emplace(std::in_place, 1);
    s.emplace(std::in_place, 2);
    s.emplace(std::in_place, 3);

    auto it = s.begin();
    EXPECT_EQ(it->get(), 1);
    ++it;
    EXPECT_EQ(it->get(), 2);
    ++it;
    EXPECT_EQ(it->get(), 3);
}

TEST(IndestructibleTest, StringWithIndestructible) {
    Indestructible<std::string> obj(std::in_place, "Hello, world!");
    EXPECT_EQ(obj.get(), "Hello, world!");
}
