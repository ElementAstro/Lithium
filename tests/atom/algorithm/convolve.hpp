#include <gtest/gtest.h>
#include "atom/algorithm/convolve.hpp"

TEST(ConvolutionTest, Convolve1DTest)
{
    std::vector<double> input = {1.0, 2.0, 3.0, 4.0, 5.0};
    std::vector<double> kernel = {0.5, 0.5};

    std::vector<double> result = Atom::Algorithm::convolve(input, kernel);

    // Verify the size of the result
    ASSERT_EQ(result.size(), input.size() - kernel.size() + 1);

    // Verify individual values of the result
    EXPECT_DOUBLE_EQ(result[0], 1.0);
    EXPECT_DOUBLE_EQ(result[1], 2.0);
    EXPECT_DOUBLE_EQ(result[2], 3.0);
    // ...

    // Add more assertions as needed
}

TEST(ConvolutionTest, Deconvolve1DTest)
{
    std::vector<double> input = {1.0, 2.0, 3.0, 4.0, 5.0};
    std::vector<double> kernel = {0.5, 0.5};

    std::vector<double> result = Atom::Algorithm::deconvolve(input, kernel);

    // Verify the size of the result
    ASSERT_EQ(result.size(), input.size() + kernel.size() - 1);

    // Verify individual values of the result
    EXPECT_DOUBLE_EQ(result[0], 2.0);
    EXPECT_DOUBLE_EQ(result[1], 4.0);
    EXPECT_DOUBLE_EQ(result[2], 6.0);
    // ...

    // Add more assertions as needed
}

TEST(ConvolutionTest, Convolve2DTest)
{
    std::vector<std::vector<double>> input = {{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}, {7.0, 8.0, 9.0}};
    std::vector<std::vector<double>> kernel = {{0.5, 0.5}, {0.5, 0.5}};

    std::vector<std::vector<double>> result = Atom::Algorithm::convolve2D(input, kernel, 4);

    // Verify the size of the result
    ASSERT_EQ(result.size(), input.size() - kernel.size() + 1);
    ASSERT_EQ(result[0].size(), input[0].size() - kernel[0].size() + 1);

    // Verify individual values of the result
    EXPECT_DOUBLE_EQ(result[0][0], 5.0);
    EXPECT_DOUBLE_EQ(result[0][1], 8.0);
    // ...

    // Add more assertions as needed
}

TEST(ConvolutionTest, Deconvolve2DTest)
{
    std::vector<std::vector<double>> input = {{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}, {7.0, 8.0, 9.0}};
    std::vector<std::vector<double>> kernel = {{0.5, 0.5}, {0.5, 0.5}};

    std::vector<std::vector<double>> result = Atom::Algorithm::deconvolve2D(input, kernel);

    // Verify the size of the result
    ASSERT_EQ(result.size(), input.size() + kernel.size() - 1);
    ASSERT_EQ(result[0].size(), input[0].size() + kernel[0].size() - 1);

    // Verify individual values of the result
    EXPECT_DOUBLE_EQ(result[0][0], 1.0);
    EXPECT_DOUBLE_EQ(result[0][1], 3.0);
    // ...

    // Add more assertions as needed
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
