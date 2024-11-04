#ifndef ATOM_META_TEST_VANY_HPP
#define ATOM_META_TEST_VANY_HPP

#include "atom/function/vany.hpp"
#include <gtest/gtest.h>

using namespace atom::meta;

TEST(AnyTest, DefaultConstructor) {
    Any any;
    EXPECT_FALSE(any.hasValue());
}

TEST(AnyTest, CopyConstructor) {
    Any any1(std::string("test"));
    Any any2(any1);
    EXPECT_TRUE(any2.hasValue());
    EXPECT_EQ(any2.cast<std::string>(), "test");
}

TEST(AnyTest, MoveConstructor) {
    Any any1(std::string("test"));
    Any any2(std::move(any1));
    EXPECT_TRUE(any2.hasValue());
    EXPECT_EQ(any2.cast<std::string>(), "test");
    EXPECT_FALSE(any1.hasValue());
}

TEST(AnyTest, CopyAssignment) {
    Any any1(std::string("test"));
    Any any2;
    any2 = any1;
    EXPECT_TRUE(any2.hasValue());
    EXPECT_EQ(any2.cast<std::string>(), "test");
}

TEST(AnyTest, MoveAssignment) {
    Any any1(std::string("test"));
    Any any2;
    any2 = std::move(any1);
    EXPECT_TRUE(any2.hasValue());
    EXPECT_EQ(any2.cast<std::string>(), "test");
    EXPECT_FALSE(any1.hasValue());
}

TEST(AnyTest, Reset) {
    Any any(std::string("test"));
    any.reset();
    EXPECT_FALSE(any.hasValue());
}

TEST(AnyTest, Type) {
    Any any(std::string("test"));
    EXPECT_EQ(any.type(), typeid(std::string));
}

TEST(AnyTest, Is) {
    Any any(std::string("test"));
    EXPECT_TRUE(any.is<std::string>());
    EXPECT_FALSE(any.is<int>());
}

TEST(AnyTest, Cast) {
    Any any(std::string("test"));
    EXPECT_EQ(any.cast<std::string>(), "test");
    EXPECT_THROW(any.cast<int>(), std::bad_cast);
}

TEST(AnyTest, ToString) {
    Any any(std::string("test"));
    EXPECT_EQ(any.toString(), "test");

    Any any2(42);
    EXPECT_EQ(any2.toString(), "42");

    Any any3;
    EXPECT_EQ(any3.toString(), "Empty Any");
}

TEST(AnyTest, Invoke) {
    Any any(std::string("test"));
    bool invoked = false;
    any.invoke([&invoked](const void* ptr) {
        invoked = true;
        EXPECT_EQ(*static_cast<const std::string*>(ptr), "test");
    });
    EXPECT_TRUE(invoked);
}

TEST(AnyTest, Foreach) {
    std::vector<int> vec = {1, 2, 3};
    Any any(vec);
    std::vector<int> result;
    any.foreach (
        [&result](const Any& item) { result.push_back(item.cast<int>()); });
    EXPECT_EQ(result, vec);
}

#endif  // ATOM_META_TEST_VANY_HPP