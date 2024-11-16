#include "hist.hpp"

#include <gtest/gtest.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

// Test case for calculateHist with an empty image
TEST(CalculateHistTest, EmptyImage) {
    cv::Mat img;
    EXPECT_THROW(calculateHist(img), std::invalid_argument);
}

// Test case for calculateHist with a single channel image
TEST(CalculateHistTest, SingleChannelImage) {
    cv::Mat img = cv::Mat::zeros(10, 10, CV_8U);
    EXPECT_THROW(calculateHist(img), std::invalid_argument);
}

// Test case for calculateHist with a 3-channel image
TEST(CalculateHistTest, ThreeChannelImage) {
    cv::Mat img = cv::Mat::zeros(10, 10, CV_8UC3);
    auto histograms = calculateHist(img);
    EXPECT_EQ(histograms.size(), 3);
    for (const auto& hist : histograms) {
        EXPECT_EQ(hist.rows, 256);
        EXPECT_EQ(hist.cols, 1);
    }
}

// Test case for calculateHist with a 3-channel image and normalization
TEST(CalculateHistTest, ThreeChannelImageWithNormalization) {
    cv::Mat img = cv::Mat::zeros(10, 10, CV_8UC3);
    auto histograms = calculateHist(img, 256, true);
    EXPECT_EQ(histograms.size(), 3);
    for (const auto& hist : histograms) {
        EXPECT_EQ(hist.rows, 256);
        EXPECT_EQ(hist.cols, 1);
        double minVal, maxVal;
        cv::minMaxLoc(hist, &minVal, &maxVal);
        EXPECT_GE(minVal, 0.0);
        EXPECT_LE(maxVal, 1.0);
    }
}

// Test case for calculateHist with a 3-channel image with random values
TEST(CalculateHistTest, ThreeChannelImageRandomValues) {
    cv::Mat img = cv::Mat::zeros(10, 10, CV_8UC3);
    cv::randu(img, cv::Scalar::all(0), cv::Scalar::all(255));
    auto histograms = calculateHist(img);
    EXPECT_EQ(histograms.size(), 3);
    for (const auto& hist : histograms) {
        EXPECT_EQ(hist.rows, 256);
        EXPECT_EQ(hist.cols, 1);
    }
}

// Test case for calculateHist with a 3-channel image with maximum values
TEST(CalculateHistTest, ThreeChannelImageMaxValues) {
    cv::Mat img = cv::Mat::ones(10, 10, CV_8UC3) * 255;
    auto histograms = calculateHist(img);
    EXPECT_EQ(histograms.size(), 3);
    for (const auto& hist : histograms) {
        EXPECT_EQ(hist.rows, 256);
        EXPECT_EQ(hist.cols, 1);
    }
}

// Test case for calculateHist with a 3-channel image with minimum values
TEST(CalculateHistTest, ThreeChannelImageMinValues) {
    cv::Mat img = cv::Mat::zeros(10, 10, CV_8UC3);
    auto histograms = calculateHist(img);
    EXPECT_EQ(histograms.size(), 3);
    for (const auto& hist : histograms) {
        EXPECT_EQ(hist.rows, 256);
        EXPECT_EQ(hist.cols, 1);
    }
}

// Test case for calculateHist with a 3-channel image with gradient values
TEST(CalculateHistTest, ThreeChannelImageGradientValues) {
    cv::Mat img = cv::Mat::zeros(10, 10, CV_8UC3);
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            img.at<cv::Vec3b>(i, j) = cv::Vec3b(i * 25, j * 25, (i + j) * 12);
        }
    }
    auto histograms = calculateHist(img);
    EXPECT_EQ(histograms.size(), 3);
    for (const auto& hist : histograms) {
        EXPECT_EQ(hist.rows, 256);
        EXPECT_EQ(hist.cols, 1);
    }
}

// Test case for calculateHist with a 3-channel image and different histogram
// size
TEST(CalculateHistTest, ThreeChannelImageDifferentHistSize) {
    cv::Mat img = cv::Mat::zeros(10, 10, CV_8UC3);
    auto histograms = calculateHist(img, 128);
    EXPECT_EQ(histograms.size(), 3);
    for (const auto& hist : histograms) {
        EXPECT_EQ(hist.rows, 128);
        EXPECT_EQ(hist.cols, 1);
    }
}

// Test case for calculateHist with a 3-channel image and normalization with
// different histogram size
TEST(CalculateHistTest, ThreeChannelImageNormalizationDifferentHistSize) {
    cv::Mat img = cv::Mat::zeros(10, 10, CV_8UC3);
    auto histograms = calculateHist(img, 128, true);
    EXPECT_EQ(histograms.size(), 3);
    for (const auto& hist : histograms) {
        EXPECT_EQ(hist.rows, 128);
        EXPECT_EQ(hist.cols, 1);
        double minVal, maxVal;
        cv::minMaxLoc(hist, &minVal, &maxVal);
        EXPECT_GE(minVal, 0.0);
        EXPECT_LE(maxVal, 1.0);
    }
}