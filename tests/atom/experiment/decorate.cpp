#include <gtest/gtest.h>
#include <functional>
#include <iostream>
#include <chrono>
#include <utility>

#include "atom/experiment/decorate.hpp"

// Test functions
int add(int a, int b)
{
    return a + b;
}

int multiply(int a, int b)
{
    return a * b;
}

void before()
{
    std::cout << "Before running the function.\n";
}

void callback(int result)
{
    std::cout << "Function returned " << result << ".\n";
}

void after(long long duration)
{
    std::cout << "Function execution time: " << duration << " microseconds.\n";
}

TEST(DecoratorTest, WithHooksAdd)
{
    auto decorated_add = make_decorator(add);
    auto custom_decorated_add = decorated_add.with_hooks(before, callback, after);
    int result_add = custom_decorated_add(3, 4);
    EXPECT_EQ(result_add, 7);
}

TEST(DecoratorTest, WithHooksMultiply)
{
    auto decorated_multiply = make_decorator(multiply);
    auto custom_decorated_multiply = decorated_multiply.with_hooks(before, callback, after);
    int result_multiply = custom_decorated_multiply(5, 6);
    EXPECT_EQ(result_multiply, 30);
}

TEST(DecoratorTest, WithoutHooksAdd)
{
    auto decorated_add = make_decorator(add);
    int result_add = decorated_add(3, 4);
    EXPECT_EQ(result_add, 7);
}

TEST(DecoratorTest, WithoutHooksMultiply)
{
    auto decorated_multiply = make_decorator(multiply);
    int result_multiply = decorated_multiply(5, 6);
    EXPECT_EQ(result_multiply, 30);
}

TEST(DecoratorTest, BeforeHookOnly)
{
    auto decorated_add = make_decorator(add);
    auto custom_decorated_add = decorated_add.with_hooks(before, nullptr, nullptr);
    int result_add = custom_decorated_add(3, 4);
    EXPECT_EQ(result_add, 7);
}

TEST(DecoratorTest, CallbackHookOnly)
{
    auto decorated_add = make_decorator(add);
    auto custom_decorated_add = decorated_add.with_hooks(nullptr, callback, nullptr);
    int result_add = custom_decorated_add(3, 4);
    EXPECT_EQ(result_add, 7);
}

TEST(DecoratorTest, AfterHookOnly)
{
    auto decorated_add = make_decorator(add);
    auto custom_decorated_add = decorated_add.with_hooks(nullptr, nullptr, after);
    int result_add = custom_decorated_add(3, 4);
    EXPECT_EQ(result_add, 7);
}

TEST(MakeDecoratorTest, Add)
{
    auto decorated_add = make_decorator(add);
    int result_add = decorated_add(3, 4);
    EXPECT_EQ(result_add, 7);
}

TEST(MakeDecoratorTest, Multiply)
{
    auto decorated_multiply = make_decorator(multiply);
    int result_multiply = decorated_multiply(5, 6);
    EXPECT_EQ(result_multiply, 30);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}