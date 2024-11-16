#include "imgutils.hpp"

#include <gtest/gtest.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <tuple>

// Test case for computeParamsOneChannel with an empty image
TEST(ComputeParamsOneChannelTest, EmptyImage) {
    cv::Mat img;
    EXPECT_THROW(computeParamsOneChannel(img), std::invalid_argument);
}

// Test case for computeParamsOneChannel with a single channel image
TEST(ComputeParamsOneChannelTest, SingleChannelImage) {
    cv::Mat img = cv::Mat::zeros(10, 10, CV_8U);
    auto [shadows, midtones, highlights] = computeParamsOneChannel(img);
    EXPECT_DOUBLE_EQ(shadows, 0.0);
    EXPECT_DOUBLE_EQ(midtones, 0.0);
    EXPECT_DOUBLE_EQ(highlights, 1.0);
}

// Test case for computeParamsOneChannel with a single channel image with non-zero values
TEST(ComputeParamsOneChannelTest, SingleChannelImageNonZero) {
    cv::Mat img = cv::Mat::ones(10, 10, CV_8U) * 128;
    auto [shadows, midtones, highlights] = computeParamsOneChannel(img);
    EXPECT_DOUBLE_EQ(shadows, 0.0);
    EXPECT_DOUBLE_EQ(midtones, 0.5);
    EXPECT_DOUBLE_EQ(highlights, 1.0);
}

// Test case for computeParamsOneChannel with a single channel image with mixed values
TEST(ComputeParamsOneChannelTest, SingleChannelImageMixedValues) {
    cv::Mat img = cv::Mat::zeros(10, 10, CV_8U);
    img.at<uchar>(5, 5) = 255;
    auto [shadows, midtones, highlights] = computeParamsOneChannel(img);
    EXPECT_DOUBLE_EQ(shadows, 0.0);
    EXPECT_DOUBLE_EQ(midtones, 0.0);
    EXPECT_DOUBLE_EQ(highlights, 1.0);
}

// Test case for computeParamsOneChannel with a 16-bit single channel image
TEST(ComputeParamsOneChannelTest, SingleChannel16BitImage) {
    cv::Mat img = cv::Mat::ones(10, 10, CV_16U) * 32768;
    auto [shadows, midtones, highlights] = computeParamsOneChannel(img);
    EXPECT_DOUBLE_EQ(shadows, 0.0);
    EXPECT_DOUBLE_EQ(midtones, 0.5);
    EXPECT_DOUBLE_EQ(highlights, 1.0);
}

// Test case for computeParamsOneChannel with a single channel image with random values
TEST(ComputeParamsOneChannelTest, SingleChannelImageRandomValues) {
    cv::Mat img = cv::Mat::zeros(10, 10, CV_8U);
    cv::randu(img, 0, 255);
    auto [shadows, midtones, highlights] = computeParamsOneChannel(img);
    EXPECT_GE(shadows, 0.0);
    EXPECT_LE(shadows, 1.0);
    EXPECT_GE(midtones, 0.0);
    EXPECT_LE(midtones, 1.0);
    EXPECT_GE(highlights, 0.0);
    EXPECT_LE(highlights, 1.0);
}

// Test case for computeParamsOneChannel with a single channel image with maximum values
TEST(ComputeParamsOneChannelTest, SingleChannelImageMaxValues) {
    cv::Mat img = cv::Mat::ones(10, 10, CV_8U) * 255;
    auto [shadows, midtones, highlights] = computeParamsOneChannel(img);
    EXPECT_DOUBLE_EQ(shadows, 0.0);
    EXPECT_DOUBLE_EQ(midtones, 0.0);
    EXPECT_DOUBLE_EQ(highlights, 1.0);
}

// Test case for computeParamsOneChannel with a single channel image with minimum values
TEST(ComputeParamsOneChannelTest, SingleChannelImageMinValues) {
    cv::Mat img = cv::Mat::zeros(10, 10, CV_8U);
    auto [shadows, midtones, highlights] = computeParamsOneChannel(img);
    EXPECT_DOUBLE_EQ(shadows, 0.0);
    EXPECT_DOUBLE_EQ(midtones, 0.0);
    EXPECT_DOUBLE_EQ(highlights, 1.0);
}

// Test case for computeParamsOneChannel with a single channel image with gradient values
TEST(ComputeParamsOneChannelTest, SingleChannelImageGradientValues) {
    cv::Mat img = cv::Mat::zeros(10, 10, CV_8U);
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            img.at<uchar>(i, j) = static_cast<uchar>(i * 25 + j * 25);
        }
    }
    auto [shadows, midtones, highlights] = computeParamsOneChannel(img);
    EXPECT_GE(shadows, 0.0);
    EXPECT_LE(shadows, 1.0);
    EXPECT_GE(midtones, 0.0);
    EXPECT_LE(midtones, 1.0);
    EXPECT_GE(highlights, 0.0);
    EXPECT_LE(highlights, 1.0);
}