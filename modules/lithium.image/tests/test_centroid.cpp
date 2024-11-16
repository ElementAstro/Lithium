#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>
#include "centroid.hpp"

class StarCentroidTest : public ::testing::Test {
protected:
    StarCentroid starCentroid;

    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }
};

// Test calcSubPixelCenter with a valid ROI and known sub-pixel center
TEST_F(StarCentroidTest, CalcSubPixelCenterValidROI) {
    cv::Mat roi =
        (cv::Mat_<float>(3, 3) << 0.0, 1.0, 0.0, 1.0, 4.0, 1.0, 0.0, 1.0, 0.0);
    cv::Point2f initCenter(1.0F, 1.0F);
    float epsilon = 1e-3;
    int maxIterations = 10;

    cv::Point2f result = starCentroid.calcSubPixelCenter(
        roi, std::move(initCenter), epsilon, maxIterations);

    EXPECT_NEAR(result.x, 1.0F, epsilon);
    EXPECT_NEAR(result.y, 1.0F, epsilon);
}

// Test calcSubPixelCenter convergence within a few iterations
TEST_F(StarCentroidTest, CalcSubPixelCenterConvergence) {
    cv::Mat roi =
        (cv::Mat_<float>(3, 3) << 0.0, 1.0, 0.0, 1.0, 4.0, 1.0, 0.0, 1.0, 0.0);
    cv::Point2f initCenter(0.5F, 0.5F);
    float epsilon = 1e-3;
    int maxIterations = 10;

    cv::Point2f result = starCentroid.calcSubPixelCenter(
        roi, std::move(initCenter), epsilon, maxIterations);

    EXPECT_NEAR(result.x, 1.0F, epsilon);
    EXPECT_NEAR(result.y, 1.0F, epsilon);
}

// Test calcSubPixelCenter with zero intensity ROI
TEST_F(StarCentroidTest, CalcSubPixelCenterZeroIntensity) {
    cv::Mat roi = cv::Mat::zeros(3, 3, CV_32F);
    cv::Point2f initCenter(1.0F, 1.0F);
    float epsilon = 1e-3;
    int maxIterations = 10;

    cv::Point2f result = starCentroid.calcSubPixelCenter(
        roi, std::move(initCenter), epsilon, maxIterations);

    EXPECT_EQ(result.x, 1.0F);
    EXPECT_EQ(result.y, 1.0F);
}

// Test calcSubPixelCenter with non-square ROI (should assert)
TEST_F(StarCentroidTest, CalcSubPixelCenterNonSquareROI) {
    cv::Mat roi = cv::Mat::ones(3, 4, CV_32F);  // Non-square ROI
    cv::Point2f initCenter(1.0F, 1.0F);
    float epsilon = 1e-3;
    int maxIterations = 10;

    EXPECT_ANY_THROW(starCentroid.calcSubPixelCenter(roi, std::move(initCenter),
                                                     epsilon, maxIterations));
}

// Test calcSubPixelCenter with invalid ROI size
TEST_F(StarCentroidTest, CalcSubPixelCenterInvalidROISize) {
    cv::Mat roi = cv::Mat::ones(2, 2, CV_32F);  // Invalid ROI size
    cv::Point2f initCenter(1.0F, 1.0F);
    float epsilon = 1e-3;
    int maxIterations = 10;

    EXPECT_ANY_THROW(starCentroid.calcSubPixelCenter(roi, std::move(initCenter),
                                                     epsilon, maxIterations));
}