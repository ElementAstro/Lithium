#include <gtest/gtest.h>
#include "atom/algorithm/convolve.hpp"

TEST(ConvolutionTest, Convolve1DTest) {
    std::vector<double> input = {1.0, 2.0, 3.0, 4.0, 5.0};
    std::vector<double> kernel = {0.5, 0.5};

    std::vector<double> result = atom::algorithm::convolve(input, kernel);
    ASSERT_EQ(result.size(), input.size() - kernel.size() + 1);
    EXPECT_DOUBLE_EQ(result[0], 1.0);
    EXPECT_DOUBLE_EQ(result[1], 2.0);
    EXPECT_DOUBLE_EQ(result[2], 3.0);
}

TEST(ConvolutionTest, Deconvolve1DTest) {
    std::vector<double> input = {1.0, 2.0, 3.0, 4.0, 5.0};
    std::vector<double> kernel = {0.5, 0.5};

    std::vector<double> result = atom::algorithm::deconvolve(input, kernel);

    ASSERT_EQ(result.size(), input.size() + kernel.size() - 1);

    EXPECT_DOUBLE_EQ(result[0], 2.0);
    EXPECT_DOUBLE_EQ(result[1], 4.0);
    EXPECT_DOUBLE_EQ(result[2], 6.0);
}

TEST(ConvolutionTest, Convolve2DTest) {
    std::vector<std::vector<double>> input = {
        {1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}, {7.0, 8.0, 9.0}};
    std::vector<std::vector<double>> kernel = {{0.5, 0.5}, {0.5, 0.5}};

    std::vector<std::vector<double>> result =
        atom::algorithm::convolve2D(input, kernel, 4);

    ASSERT_EQ(result.size(), input.size() - kernel.size() + 1);
    ASSERT_EQ(result[0].size(), input[0].size() - kernel[0].size() + 1);

    EXPECT_DOUBLE_EQ(result[0][0], 5.0);
    EXPECT_DOUBLE_EQ(result[0][1], 8.0);
}

TEST(ConvolutionTest, Deconvolve2DTest) {
    std::vector<std::vector<double>> input = {
        {1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}, {7.0, 8.0, 9.0}};
    std::vector<std::vector<double>> kernel = {{0.5, 0.5}, {0.5, 0.5}};

    std::vector<std::vector<double>> result =
        atom::algorithm::deconvolve2D(input, kernel);
    ASSERT_EQ(result.size(), input.size() + kernel.size() - 1);
    ASSERT_EQ(result[0].size(), input[0].size() + kernel[0].size() - 1);

    EXPECT_DOUBLE_EQ(result[0][0], 1.0);
    EXPECT_DOUBLE_EQ(result[0][1], 3.0);
}
