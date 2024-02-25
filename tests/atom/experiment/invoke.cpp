#include "atom/experiment/invoke.hpp"

#include <gtest/gtest.h>

TEST(DelayInvokeTest, InvokeLambdaWithArgs)
{
    auto delayed_add = delay_invoke<std::function<int(int, int)>>([](int a, int b)
                                                                  { return a + b; },
                                                                  3, 4);
    int result = delayed_add();
    EXPECT_EQ(result, 7);
}

TEST(DelayInvokeTest, InvokeFunctionWithArgs)
{
    auto multiply = [](int a, int b)
    { return a * b; };
    auto delayed_multiply = delay_invoke<std::function<int(int, int)>>(multiply, 5, 6);
    int result = delayed_multiply();
    EXPECT_EQ(result, 30);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}