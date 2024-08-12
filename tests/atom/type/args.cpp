#include "atom/type/args.hpp"
#include <gtest/gtest.h>

TEST(ArgsTest, SetAndGet) {
    Args args;
    args.set("key1", 10);
    args.set("key2", "value2");
    args.set("key3", 3.14);

    EXPECT_EQ(args.get<int>("key1"), 10);
    EXPECT_EQ(args.get<std::string>("key2"), "value2");
    EXPECT_EQ(args.get<double>("key3"), 3.14);
}

TEST(ArgsTest, GetOr) {
    Args args;
    args.set("key1", 10);

    EXPECT_EQ(args.getOr<int>("key1", 20), 10);
    EXPECT_EQ(args.getOr<int>("key2", 20), 20);
}

TEST(ArgsTest, GetOptional) {
    Args args;
    args.set("key1", 10);

    EXPECT_TRUE(args.getOptional<int>("key1").has_value());
    EXPECT_FALSE(args.getOptional<int>("key2").has_value());
}

TEST(ArgsTest, Contains) {
    Args args;
    args.set("key1", 10);

    EXPECT_TRUE(args.contains("key1"));
    EXPECT_FALSE(args.contains("key2"));
}

TEST(ArgsTest, Remove) {
    Args args;
    args.set("key1", 10);
    args.remove("key1");

    EXPECT_FALSE(args.contains("key1"));
}

TEST(ArgsTest, Clear) {
    Args args;
    args.set("key1", 10);
    args.clear();

    EXPECT_TRUE(args.empty());
}

TEST(ArgsTest, Size) {
    Args args;
    args.set("key1", 10);
    args.set("key2", 20);

    EXPECT_EQ(args.size(), 2);
}

TEST(ArgsTest, Empty) {
    Args args;
    EXPECT_TRUE(args.empty());

    args.set("key1", 10);
    EXPECT_FALSE(args.empty());
}
