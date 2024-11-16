#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>
#include "fwhm.hpp"

class GaussianFitTest : public ::testing::Test {
protected:
    GaussianFit gaussianFit;

    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }
};

// Test GaussianFit::fit with valid data points
TEST_F(GaussianFitTest, FitValidDataPoints) {
    std::vector<DataPoint> points = {
        {0.0, 1.0}, {1.0, 2.0}, {2.0, 3.0}, {3.0, 4.0}, {4.0, 3.0}, {5.0, 2.0}, {6.0, 1.0}
    };
    double epsilon = 1e-6;
    int maxIterations = 100;

    auto result = gaussianFit.fit(points, epsilon, maxIterations);

    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->base, 1.0, epsilon);
    EXPECT_NEAR(result->peak, 3.0, epsilon);
    EXPECT_NEAR(result->center, 3.0, epsilon);
    EXPECT_NEAR(result->width, 2.0, epsilon);
}

// Test GaussianFit::fit with empty data points
TEST_F(GaussianFitTest, FitEmptyDataPoints) {
    std::vector<DataPoint> points;
    double epsilon = 1e-6;
    int maxIterations = 100;

    auto result = gaussianFit.fit(points, epsilon, maxIterations);

    EXPECT_FALSE(result.has_value());
}

// Test GaussianFit::fit convergence within a few iterations
TEST_F(GaussianFitTest, FitConvergence) {
    std::vector<DataPoint> points = {
        {0.0, 1.0}, {1.0, 2.0}, {2.0, 3.0}, {3.0, 4.0}, {4.0, 3.0}, {5.0, 2.0}, {6.0, 1.0}
    };
    double epsilon = 1e-6;
    int maxIterations = 10;

    auto result = gaussianFit.fit(points, epsilon, maxIterations);

    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->base, 1.0, epsilon);
    EXPECT_NEAR(result->peak, 3.0, epsilon);
    EXPECT_NEAR(result->center, 3.0, epsilon);
    EXPECT_NEAR(result->width, 2.0, epsilon);
}

// Test GaussianFit::fit with identical data points
TEST_F(GaussianFitTest, FitIdenticalDataPoints) {
    std::vector<DataPoint> points = {
        {1.0, 2.0}, {1.0, 2.0}, {1.0, 2.0}, {1.0, 2.0}, {1.0, 2.0}
    };
    double epsilon = 1e-6;
    int maxIterations = 100;

    auto result = gaussianFit.fit(points, epsilon, maxIterations);

    EXPECT_FALSE(result.has_value());
}

// Test GaussianFit::fit with noisy data points
TEST_F(GaussianFitTest, FitNoisyDataPoints) {
    std::vector<DataPoint> points = {
        {0.0, 1.1}, {1.0, 2.1}, {2.0, 3.0}, {3.0, 4.1}, {4.0, 3.0}, {5.0, 2.1}, {6.0, 1.1}
    };
    double epsilon = 1e-6;
    int maxIterations = 100;

    auto result = gaussianFit.fit(points, epsilon, maxIterations);

    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->base, 1.0, epsilon);
    EXPECT_NEAR(result->peak, 3.0, epsilon);
    EXPECT_NEAR(result->center, 3.0, epsilon);
    EXPECT_NEAR(result->width, 2.0, epsilon);
}

// Test GaussianFit::fit with a single data point
TEST_F(GaussianFitTest, FitSingleDataPoint) {
    std::vector<DataPoint> points = {
        {1.0, 2.0}
    };
    double epsilon = 1e-6;
    int maxIterations = 100;

    auto result = gaussianFit.fit(points, epsilon, maxIterations);

    EXPECT_FALSE(result.has_value());
}