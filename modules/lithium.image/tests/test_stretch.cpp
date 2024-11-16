#include "stretch.hpp"
#include <gtest/gtest.h>
#include <opencv2/imgproc.hpp>

namespace {

// Helper function to create a synthetic histogram
cv::Mat createTestHistogram(float value, int size = 256) {
    cv::Mat hist = cv::Mat::zeros(size, 1, CV_32F);
    hist.at<float>(static_cast<int>(value)) = 1.0f;
    return hist;
}

// Helper function to create a test image plane
cv::Mat createTestPlane(int width, int height, uchar value) {
    return cv::Mat(height, width, CV_8UC1, cv::Scalar(value));
}

// Helper to verify basic image properties
void validateImageProperties(const cv::Mat& img, cv::Size expectedSize, int expectedType) {
    EXPECT_EQ(img.size(), expectedSize);
    EXPECT_EQ(img.type(), expectedType);
}

class StretchWhiteBalanceTest : public ::testing::Test {
protected:
    void SetUp() override {
        width = 100;
        height = 100;
    }

    int width;
    int height;
};

// Test input validation
TEST_F(StretchWhiteBalanceTest, EmptyInputs) {
    std::vector<cv::Mat> emptyHists;
    std::vector<cv::Mat> emptyPlanes;
    
    EXPECT_THROW(stretchWhiteBalance(emptyHists, emptyPlanes), std::invalid_argument);
}

TEST_F(StretchWhiteBalanceTest, WrongChannelCount) {
    std::vector<cv::Mat> hists = {createTestHistogram(128)};
    std::vector<cv::Mat> planes = {createTestPlane(width, height, 128)};
    
    EXPECT_THROW(stretchWhiteBalance(hists, planes), std::invalid_argument);
}

// Test normal operation
TEST_F(StretchWhiteBalanceTest, NormalOperation) {
    std::vector<cv::Mat> hists = {
        createTestHistogram(100),
        createTestHistogram(128),
        createTestHistogram(150)
    };
    
    std::vector<cv::Mat> planes = {
        createTestPlane(width, height, 100),
        createTestPlane(width, height, 128),
        createTestPlane(width, height, 150)
    };
    
    cv::Mat result = stretchWhiteBalance(hists, planes);
    
    validateImageProperties(result, cv::Size(width, height), CV_16UC3);
    EXPECT_GT(cv::mean(result)[0], 0);
}

// Test zero values
TEST_F(StretchWhiteBalanceTest, ZeroValues) {
    std::vector<cv::Mat> hists = {
        createTestHistogram(0),
        createTestHistogram(0),
        createTestHistogram(0)
    };
    
    std::vector<cv::Mat> planes = {
        createTestPlane(width, height, 0),
        createTestPlane(width, height, 0),
        createTestPlane(width, height, 0)
    };
    
    cv::Mat result = stretchWhiteBalance(hists, planes);
    validateImageProperties(result, cv::Size(width, height), CV_16UC3);
}

// Test maximum values
TEST_F(StretchWhiteBalanceTest, MaxValues) {
    std::vector<cv::Mat> hists = {
        createTestHistogram(255),
        createTestHistogram(255),
        createTestHistogram(255)
    };
    
    std::vector<cv::Mat> planes = {
        createTestPlane(width, height, 255),
        createTestPlane(width, height, 255),
        createTestPlane(width, height, 255)
    };
    
    cv::Mat result = stretchWhiteBalance(hists, planes);
    validateImageProperties(result, cv::Size(width, height), CV_16UC3);
    EXPECT_LE(cv::mean(result)[0], 65535);
}

// Test color balance correction
TEST_F(StretchWhiteBalanceTest, ColorBalanceCorrection) {
    // Create imbalanced color channels
    std::vector<cv::Mat> hists = {
        createTestHistogram(50),   // Blue - dark
        createTestHistogram(128),  // Green - mid
        createTestHistogram(200)   // Red - bright
    };
    
    std::vector<cv::Mat> planes = {
        createTestPlane(width, height, 50),
        createTestPlane(width, height, 128),
        createTestPlane(width, height, 200)
    };
    
    cv::Mat result = stretchWhiteBalance(hists, planes);
    
    // After white balance, channels should be more balanced
    std::vector<cv::Mat> channels;
    cv::split(result, channels);
    
    double meanB = cv::mean(channels[0])[0];
    double meanG = cv::mean(channels[1])[0];
    double meanR = cv::mean(channels[2])[0];
    
    // Verify channels are more balanced after correction
    double maxDiff = std::max({std::abs(meanB - meanG), 
                              std::abs(meanG - meanR), 
                              std::abs(meanB - meanR)});
    
    EXPECT_LT(maxDiff / 65535.0, 0.2); // Channels should be within 20% of each other
}

// Test output range
TEST_F(StretchWhiteBalanceTest, OutputRange) {
    std::vector<cv::Mat> hists = {
        createTestHistogram(128),
        createTestHistogram(128),
        createTestHistogram(128)
    };
    
    std::vector<cv::Mat> planes = {
        createTestPlane(width, height, 128),
        createTestPlane(width, height, 128),
        createTestPlane(width, height, 128)
    };
    
    cv::Mat result = stretchWhiteBalance(hists, planes);
    
    double minVal, maxVal;
    cv::minMaxLoc(result, &minVal, &maxVal);
    
    EXPECT_GE(minVal, 0);
    EXPECT_LE(maxVal, 65535);
}

} // namespace