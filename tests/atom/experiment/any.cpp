#include <gtest/gtest.h>
#include "atom/experiment/any.hpp"

TEST(AnyTest, Construction)
{
    Any a1;
    EXPECT_TRUE(a1.empty());

    int int_value = 42;
    Any a2(int_value);
    EXPECT_FALSE(a2.empty());
    EXPECT_EQ(typeid(int), a2.type());

    double double_value = 3.14;
    Any a3(double_value);
    EXPECT_FALSE(a3.empty());
    EXPECT_EQ(typeid(double), a3.type());
}

TEST(AnyTest, CopyConstructor)
{
    int int_value = 42;
    Any a1(int_value);
    Any a2(a1);

    EXPECT_FALSE(a2.empty());
    EXPECT_EQ(typeid(int), a2.type());
    EXPECT_EQ(int_value, any_cast<int>(a2));
}

TEST(AnyTest, MoveConstructor)
{
    int int_value = 42;
    Any a1(int_value);
    Any a2(std::move(a1));

    EXPECT_TRUE(a1.empty());
    EXPECT_FALSE(a2.empty());
    EXPECT_EQ(typeid(int), a2.type());
    EXPECT_EQ(int_value, any_cast<int>(a2));
}

TEST(AnyTest, Assignment)
{
    int int_value = 42;
    Any a1(int_value);
    Any a2;

    a2 = a1;
    EXPECT_FALSE(a2.empty());
    EXPECT_EQ(typeid(int), a2.type());
    EXPECT_EQ(int_value, any_cast<int>(a2));

    double double_value = 3.14;
    a2 = double_value;
    EXPECT_FALSE(a2.empty());
    EXPECT_EQ(typeid(double), a2.type());
    EXPECT_EQ(double_value, any_cast<double>(a2));
}

TEST(AnyTest, TypeCheck)
{
    int int_value = 42;
    Any a1(int_value);

    EXPECT_EQ(typeid(int), a1.type());
    EXPECT_ANY_THROW(any_cast<double>(a1)); // 预期抛出异常，类型不匹配
}

TEST(AnyTest, MoveAssignment)
{
    int int_value = 42;
    Any a1(int_value);

    Any a2;
    a2 = std::move(a1);

    EXPECT_TRUE(a1.empty());
    EXPECT_FALSE(a2.empty());
    EXPECT_EQ(typeid(int), a2.type());
    EXPECT_EQ(int_value, any_cast<int>(a2));
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
