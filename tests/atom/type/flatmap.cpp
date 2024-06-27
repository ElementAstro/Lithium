#include "atom/type/flatmap.hpp"
#include <gtest/gtest.h>

TEST(QuickFlatMapTest, FindExistingKey) {
    QuickFlatMap<int, std::string> map;
    map[1] = "one";
    map[2] = "two";

    auto iter = map.find(1);

    EXPECT_NE(iter, map.end());
    EXPECT_EQ(iter->first, 1);
    EXPECT_EQ(iter->second, "one");
}

TEST(QuickFlatMapTest, FindNonExistingKey) {
    QuickFlatMap<int, std::string> map;
    map[1] = "one";
    map[2] = "two";

    auto iter = map.find(3);

    EXPECT_EQ(iter, map.end());
}

TEST(QuickFlatMapTest, InsertOrAssignExistingKey) {
    QuickFlatMap<int, std::string> map;
    map[1] = "one";

    auto result = map.insertOrAssign(1, "new_one");

    EXPECT_FALSE(result.second);
    EXPECT_EQ(result.first->second, "new_one");
}

TEST(QuickFlatMapTest, InsertOrAssignNewKey) {
    QuickFlatMap<int, std::string> map;
    map[1] = "one";

    auto result = map.insertOrAssign(2, "two");

    EXPECT_TRUE(result.second);
    EXPECT_EQ(result.first->second, "two");
}

TEST(QuickFlatMapTest, InsertExistingKey) {
    QuickFlatMap<int, std::string> map;
    map[1] = "one";

    auto result = map.insert({1, "new_one"});

    EXPECT_FALSE(result.second);
    EXPECT_EQ(result.first->second, "one");
}

TEST(QuickFlatMapTest, InsertNewKey) {
    QuickFlatMap<int, std::string> map;
    map[1] = "one";

    auto result = map.insert({2, "two"});

    EXPECT_TRUE(result.second);
    EXPECT_EQ(result.first->second, "two");
}

TEST(QuickFlatMapTest, AtExistingKey) {
    QuickFlatMap<int, std::string> map;
    map[1] = "one";

    try {
        auto& value = map.at(1);
        EXPECT_EQ(value, "one");
    } catch (const std::out_of_range& e) {
        FAIL() << e.what();
    }
}

TEST(QuickFlatMapTest, AtNonExistingKey) {
    QuickFlatMap<int, std::string> map;
    map[1] = "one";

    try {
        auto& value = map.at(2);
        FAIL() << "Expected std::out_of_range exception";
    } catch (const std::out_of_range& e) {
        EXPECT_STREQ(e.what(), "Unknown key: 2");
    } catch (...) {
        FAIL() << "Expected std::out_of_range exception";
    }
}