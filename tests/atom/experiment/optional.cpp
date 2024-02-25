#include "atom/experiment/optional.hpp"

#include <gtest/gtest.h>

TEST(OptionalTest, DefaultConstructor)
{
    Optional<int> opt;
    EXPECT_FALSE(opt);
}

TEST(OptionalTest, ValueConstructor)
{
    Optional<int> opt(5);
    EXPECT_TRUE(opt);
    EXPECT_EQ(opt.value(), 5);
}

TEST(OptionalTest, MoveValueConstructor)
{
    Optional<std::string> opt(std::string("Hello"));
    Optional<std::string> opt2(std::move(opt));
    EXPECT_TRUE(opt2);
    EXPECT_EQ(opt2.value(), "Hello");
}

TEST(OptionalTest, CopyConstructor)
{
    Optional<int> opt1(10);
    Optional<int> opt2(opt1);
    EXPECT_TRUE(opt1);
    EXPECT_TRUE(opt2);
    EXPECT_EQ(opt1.value(), 10);
    EXPECT_EQ(opt2.value(), 10);
}

TEST(OptionalTest, MoveConstructor)
{
    Optional<std::string> opt1("Hello");
    Optional<std::string> opt2(std::move(opt1));
    EXPECT_TRUE(opt2);
    EXPECT_EQ(opt2.value(), "Hello");
}

TEST(OptionalTest, AssignmentOperator)
{
    Optional<int> opt1(15);
    Optional<int> opt2;
    opt2 = opt1;
    EXPECT_TRUE(opt1);
    EXPECT_TRUE(opt2);
    EXPECT_EQ(opt1.value(), 15);
    EXPECT_EQ(opt2.value(), 15);
}

TEST(OptionalTest, MoveAssignmentOperator)
{
    Optional<std::string> opt1("Hello");
    Optional<std::string> opt2;
    opt2 = std::move(opt1);
    EXPECT_TRUE(opt2);
    EXPECT_EQ(opt2.value(), "Hello");
}

TEST(OptionalTest, Reset)
{
    Optional<int> opt(20);
    opt.reset();
    EXPECT_FALSE(opt);
}

TEST(OptionalTest, Value)
{
    Optional<int> opt(25);
    EXPECT_EQ(opt.value(), 25);
}

TEST(OptionalTest, ConstValue)
{
    const Optional<int> opt(30);
    EXPECT_EQ(opt.value(), 30);
}

TEST(OptionalTest, BoolConversion)
{
    Optional<int> opt;
    EXPECT_FALSE(opt);
    opt = 40;
    EXPECT_TRUE(opt);
}

TEST(OptionalTest, EqualityOperator)
{
    Optional<int> opt1(50);
    Optional<int> opt2(50);
    Optional<int> opt3(60);
    EXPECT_TRUE(opt1 == opt2);
    EXPECT_FALSE(opt1 == opt3);
}

TEST(OptionalTest, InequalityOperator)
{
    Optional<int> opt1(70);
    Optional<int> opt2(70);
    Optional<int> opt3(80);
    EXPECT_FALSE(opt1 != opt2);
    EXPECT_TRUE(opt1 != opt3);
}

TEST(OptionalTest, ValueOr)
{
    Optional<int> opt;
    EXPECT_EQ(opt.value_or(90), 90);
    opt = 100;
    EXPECT_EQ(opt.value_or(90), 100);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
