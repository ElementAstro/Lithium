#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>
#include "src/binning.cpp"

class BinningTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test images
        smallImage = cv::Mat::ones(100, 100, CV_8UC1) * 255;
        largeImage = cv::Mat::ones(3000, 3000, CV_8UC1) * 255;
        colorImage = cv::Mat::ones(100, 100, CV_8UC3);
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j) {
                colorImage.at<cv::Vec3b>(i, j) = cv::Vec3b(i, j, (i + j) % 256);
            }
        }
    }

    cv::Mat smallImage;
    cv::Mat largeImage;
    cv::Mat colorImage;
};

// Test mergeImageBasedOnSize with small image
TEST_F(BinningTest, MergeImageBasedOnSizeSmallImage) {
    CamBin result = Tools::mergeImageBasedOnSize(smallImage);
    EXPECT_EQ(result.camxbin, 1);
    EXPECT_EQ(result.camybin, 1);
}

// Test mergeImageBasedOnSize with large image
TEST_F(BinningTest, MergeImageBasedOnSizeLargeImage) {
    CamBin result = Tools::mergeImageBasedOnSize(largeImage);
    EXPECT_EQ(result.camxbin, 2);
    EXPECT_EQ(result.camybin, 2);
}

// Test processMatWithBinAvg with small image and averaging
TEST_F(BinningTest, ProcessMatWithBinAvgSmallImageAvg) {
    cv::Mat result = Tools::processMatWithBinAvg(smallImage, 2, 2, false, true);
    EXPECT_EQ(result.rows, 50);
    EXPECT_EQ(result.cols, 50);
    EXPECT_EQ(result.at<uint8_t>(0, 0), 255);
}

// Test processMatWithBinAvg with large image and averaging
TEST_F(BinningTest, ProcessMatWithBinAvgLargeImageAvg) {
    cv::Mat result = Tools::processMatWithBinAvg(largeImage, 2, 2, false, true);
    EXPECT_EQ(result.rows, 1500);
    EXPECT_EQ(result.cols, 1500);
    EXPECT_EQ(result.at<uint8_t>(0, 0), 255);
}

// Test processMatWithBinAvg with color image and averaging
TEST_F(BinningTest, ProcessMatWithBinAvgColorImageAvg) {
    cv::Mat result = Tools::processMatWithBinAvg(colorImage, 2, 2, true, true);
    EXPECT_EQ(result.rows, 50);
    EXPECT_EQ(result.cols, 50);
    EXPECT_EQ(result.channels(), 3);
}

// Test processMatWithBinAvg with small image and binning
TEST_F(BinningTest, ProcessMatWithBinAvgSmallImageBin) {
    cv::Mat result = Tools::processMatWithBinAvg(smallImage, 2, 2, false, false);
    EXPECT_EQ(result.rows, 50);
    EXPECT_EQ(result.cols, 50);
    EXPECT_EQ(result.at<uint8_t>(0, 0), 255);
}

// Test processMatWithBinAvg with large image and binning
TEST_F(BinningTest, ProcessMatWithBinAvgLargeImageBin) {
    cv::Mat result = Tools::processMatWithBinAvg(largeImage, 2, 2, false, false);
    EXPECT_EQ(result.rows, 1500);
    EXPECT_EQ(result.cols, 1500);
    EXPECT_EQ(result.at<uint8_t>(0, 0), 255);
}

// Test processMatWithBinAvg with color image and binning
TEST_F(BinningTest, ProcessMatWithBinAvgColorImageBin) {
    cv::Mat result = Tools::processMatWithBinAvg(colorImage, 2, 2, true, false);
    EXPECT_EQ(result.rows, 50);
    EXPECT_EQ(result.cols, 50);
    EXPECT_EQ(result.channels(), 3);
}

// Test processMatWithBinAvg with invalid bin sizes
TEST_F(BinningTest, ProcessMatWithBinAvgInvalidBinSizes) {
    EXPECT_THROW(Tools::processMatWithBinAvg(smallImage, 0, 2, false, true), cv::Exception);
    EXPECT_THROW(Tools::processMatWithBinAvg(smallImage, 2, 0, false, true), cv::Exception);
}

// Test processMatWithBinAvg with empty image
TEST_F(BinningTest, ProcessMatWithBinAvgEmptyImage) {
    cv::Mat emptyImage;
    EXPECT_THROW(Tools::processMatWithBinAvg(emptyImage, 2, 2, false, true), cv::Exception);
}

// Test processMatWithBinAvg with non-divisible dimensions
TEST_F(BinningTest, ProcessMatWithBinAvgNonDivisibleDimensions) {
    cv::Mat nonDivisibleImage = cv::Mat::ones(101, 101, CV_8UC1) * 255;
    cv::Mat result = Tools::processMatWithBinAvg(nonDivisibleImage, 2, 2, false, true);
    EXPECT_EQ(result.rows, 50);
    EXPECT_EQ(result.cols, 50);
    EXPECT_EQ(result.at<uint8_t>(0, 0), 255);
}

// Test processMatWithBinAvg with maximum possible image sizes
TEST_F(BinningTest, ProcessMatWithBinAvgMaxImageSize) {
    cv::Mat maxImage = cv::Mat::ones(2000, 2000, CV_8UC1) * 255;
    cv::Mat result = Tools::processMatWithBinAvg(maxImage, 2, 2, false, true);
    EXPECT_EQ(result.rows, 1000);
    EXPECT_EQ(result.cols, 1000);
    EXPECT_EQ(result.at<uint8_t>(0, 0), 255);
}

// Test processMatWithBinAvg with minimum bin sizes
TEST_F(BinningTest, ProcessMatWithBinAvgMinBinSize) {
    cv::Mat result = Tools::processMatWithBinAvg(smallImage, 1, 1, false, true);
    EXPECT_EQ(result.rows, 100);
    EXPECT_EQ(result.cols, 100);
    EXPECT_EQ(result.at<uint8_t>(0, 0), 255);
}