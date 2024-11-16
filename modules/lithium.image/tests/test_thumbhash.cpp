#include "thumbhash.hpp"

#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>

// Test case for converting black color (0, 0, 0)
TEST(RgbToYCbCrTest, BlackColor) {
    cv::Vec3b rgb(0, 0, 0);
    YCbCr yCbCr = rgbToYCbCr(rgb);
    EXPECT_NEAR(yCbCr.y, 0.0, 1e-6);
    EXPECT_NEAR(yCbCr.cb, 0.5, 1e-6);
    EXPECT_NEAR(yCbCr.cr, 0.5, 1e-6);
}

// Test case for converting white color (255, 255, 255)
TEST(RgbToYCbCrTest, WhiteColor) {
    cv::Vec3b rgb(255, 255, 255);
    YCbCr yCbCr = rgbToYCbCr(rgb);
    EXPECT_NEAR(yCbCr.y, 1.0, 1e-6);
    EXPECT_NEAR(yCbCr.cb, 0.0, 1e-6);
    EXPECT_NEAR(yCbCr.cr, 0.0, 1e-6);
}

// Test case for converting red color (255, 0, 0)
TEST(RgbToYCbCrTest, RedColor) {
    cv::Vec3b rgb(0, 0, 255);
    YCbCr yCbCr = rgbToYCbCr(rgb);
    EXPECT_NEAR(yCbCr.y, 0.299, 1e-6);
    EXPECT_NEAR(yCbCr.cb, 0.5, 1e-6);
    EXPECT_NEAR(yCbCr.cr, 1.0, 1e-6);
}

// Test case for converting green color (0, 255, 0)
TEST(RgbToYCbCrTest, GreenColor) {
    cv::Vec3b rgb(0, 255, 0);
    YCbCr yCbCr = rgbToYCbCr(rgb);
    EXPECT_NEAR(yCbCr.y, 0.587, 1e-6);
    EXPECT_NEAR(yCbCr.cb, -0.28886, 1e-6);
    EXPECT_NEAR(yCbCr.cr, -0.51499, 1e-6);
}

// Test case for converting blue color (0, 0, 255)
TEST(RgbToYCbCrTest, BlueColor) {
    cv::Vec3b rgb(255, 0, 0);
    YCbCr yCbCr = rgbToYCbCr(rgb);
    EXPECT_NEAR(yCbCr.y, 0.114, 1e-6);
    EXPECT_NEAR(yCbCr.cb, 0.436, 1e-6);
    EXPECT_NEAR(yCbCr.cr, -0.10001, 1e-6);
}

// Test case for converting gray color (128, 128, 128)
TEST(RgbToYCbCrTest, GrayColor) {
    cv::Vec3b rgb(128, 128, 128);
    YCbCr yCbCr = rgbToYCbCr(rgb);
    EXPECT_NEAR(yCbCr.y, 0.5, 1e-6);
    EXPECT_NEAR(yCbCr.cb, 0.0, 1e-6);
    EXPECT_NEAR(yCbCr.cr, 0.0, 1e-6);
}

// Test case for converting yellow color (255, 255, 0)
TEST(RgbToYCbCrTest, YellowColor) {
    cv::Vec3b rgb(0, 255, 255);
    YCbCr yCbCr = rgbToYCbCr(rgb);
    EXPECT_NEAR(yCbCr.y, 0.886, 1e-6);
    EXPECT_NEAR(yCbCr.cb, -0.34414, 1e-6);
    EXPECT_NEAR(yCbCr.cr, 0.0, 1e-6);
}

// Test case for converting cyan color (0, 255, 255)
TEST(RgbToYCbCrTest, CyanColor) {
    cv::Vec3b rgb(255, 255, 0);
    YCbCr yCbCr = rgbToYCbCr(rgb);
    EXPECT_NEAR(yCbCr.y, 0.701, 1e-6);
    EXPECT_NEAR(yCbCr.cb, 0.436, 1e-6);
    EXPECT_NEAR(yCbCr.cr, -0.10001, 1e-6);
}

// Test case for converting magenta color (255, 0, 255)
TEST(RgbToYCbCrTest, MagentaColor) {
    cv::Vec3b rgb(255, 0, 255);
    YCbCr yCbCr = rgbToYCbCr(rgb);
    EXPECT_NEAR(yCbCr.y, 0.413, 1e-6);
    EXPECT_NEAR(yCbCr.cb, 0.5, 1e-6);
    EXPECT_NEAR(yCbCr.cr, 0.5, 1e-6);
}

// Test case for converting a random color (123, 234, 45)
TEST(RgbToYCbCrTest, RandomColor) {
    cv::Vec3b rgb(45, 234, 123);
    YCbCr yCbCr = rgbToYCbCr(rgb);
    EXPECT_NEAR(yCbCr.y, 0.678, 1e-6);
    EXPECT_NEAR(yCbCr.cb, -0.253, 1e-6);
    EXPECT_NEAR(yCbCr.cr, -0.168, 1e-6);
}
