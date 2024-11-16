#include "stack.hpp"
#include <gtest/gtest.h>
#include <opencv2/imgproc.hpp>

// Helper function to create synthetic test images
cv::Mat createTestImage(int width, int height, uchar value) {
    cv::Mat img(height, width, CV_8UC1, cv::Scalar(value));
    return img;
}

// Helper function to create a set of test images
std::vector<cv::Mat> createTestImages(int width, int height, const std::vector<uchar>& values) {
    std::vector<cv::Mat> images;
    for (auto value : values) {
        images.push_back(createTestImage(width, height, value));
    }
    return images;
}

// Test empty input
TEST(StackImagesTest, EmptyInput) {
    std::vector<cv::Mat> images;
    std::vector<float> weights;
    EXPECT_THROW(stackImages(images, MEAN, 2.0f, weights), std::runtime_error);
}

// Test mean stacking
TEST(StackImagesTest, MeanStack) {
    std::vector<cv::Mat> images = createTestImages(10, 10, {100, 200});
    cv::Mat result = stackImages(images, MEAN, 2.0f, {});
    
    EXPECT_EQ(result.type(), CV_8UC1);
    EXPECT_EQ(result.size(), images[0].size());
    EXPECT_NEAR(cv::mean(result)[0], 150.0, 1.0);
}

// Test median stacking
TEST(StackImagesTest, MedianStack) {
    std::vector<cv::Mat> images = createTestImages(10, 10, {100, 150, 200});
    cv::Mat result = stackImages(images, MEDIAN, 2.0f, {});
    
    EXPECT_EQ(result.type(), CV_8UC1);
    EXPECT_EQ(result.size(), images[0].size());
    EXPECT_NEAR(cv::mean(result)[0], 150.0, 1.0);
}

// Test maximum stacking
TEST(StackImagesTest, MaximumStack) {
    std::vector<cv::Mat> images = createTestImages(10, 10, {100, 200});
    cv::Mat result = stackImages(images, MAXIMUM, 2.0f, {});
    
    EXPECT_EQ(result.type(), CV_8UC1);
    EXPECT_EQ(result.size(), images[0].size());
    EXPECT_NEAR(cv::mean(result)[0], 200.0, 1.0);
}

// Test minimum stacking
TEST(StackImagesTest, MinimumStack) {
    std::vector<cv::Mat> images = createTestImages(10, 10, {100, 200});
    cv::Mat result = stackImages(images, MINIMUM, 2.0f, {});
    
    EXPECT_EQ(result.type(), CV_8UC1);
    EXPECT_EQ(result.size(), images[0].size());
    EXPECT_NEAR(cv::mean(result)[0], 100.0, 1.0);
}

// Test sigma clipping stack
TEST(StackImagesTest, SigmaClippingStack) {
    std::vector<cv::Mat> images = createTestImages(10, 10, {100, 150, 200, 250});
    cv::Mat result = stackImages(images, SIGMA_CLIPPING, 2.0f, {});
    
    EXPECT_EQ(result.type(), CV_8UC1);
    EXPECT_EQ(result.size(), images[0].size());
    EXPECT_GT(cv::mean(result)[0], 100.0);
    EXPECT_LT(cv::mean(result)[0], 250.0);
}

// Test weighted mean stack
TEST(StackImagesTest, WeightedMeanStack) {
    std::vector<cv::Mat> images = createTestImages(10, 10, {100, 200});
    std::vector<float> weights = {1.0f, 2.0f};
    cv::Mat result = stackImages(images, WEIGHTED_MEAN, 2.0f, weights);
    
    EXPECT_EQ(result.type(), CV_8UC1);
    EXPECT_EQ(result.size(), images[0].size());
    EXPECT_NEAR(cv::mean(result)[0], 166.67, 1.0);
}

// Test weighted mean stack with invalid weights
TEST(StackImagesTest, WeightedMeanStackInvalidWeights) {
    std::vector<cv::Mat> images = createTestImages(10, 10, {100, 200});
    std::vector<float> weights = {1.0f}; // Wrong number of weights
    EXPECT_THROW(stackImages(images, WEIGHTED_MEAN, 2.0f, weights), std::runtime_error);
}

// Test lighten stack
TEST(StackImagesTest, LightenStack) {
    std::vector<cv::Mat> images = createTestImages(10, 10, {100, 200});
    cv::Mat result = stackImages(images, LIGHTEN, 2.0f, {});
    
    EXPECT_EQ(result.type(), CV_8UC1);
    EXPECT_EQ(result.size(), images[0].size());
    EXPECT_NEAR(cv::mean(result)[0], 200.0, 1.0);
}

// Test invalid stacking mode
TEST(StackImagesTest, InvalidStackingMode) {
    std::vector<cv::Mat> images = createTestImages(10, 10, {100, 200});
    EXPECT_THROW(stackImages(images, static_cast<StackMode>(-1), 2.0f, {}), std::invalid_argument);
}

// Test single image input
TEST(StackImagesTest, SingleImage) {
    std::vector<cv::Mat> images = createTestImages(10, 10, {100});
    cv::Mat result = stackImages(images, MEAN, 2.0f, {});
    
    EXPECT_EQ(result.type(), CV_8UC1);
    EXPECT_EQ(result.size(), images[0].size());
    EXPECT_NEAR(cv::mean(result)[0], 100.0, 1.0);
}

// Test large number of images
TEST(StackImagesTest, ManyImages) {
    std::vector<uchar> values(100, 100);
    std::vector<cv::Mat> images = createTestImages(10, 10, values);
    cv::Mat result = stackImages(images, MEAN, 2.0f, {});
    
    EXPECT_EQ(result.type(), CV_8UC1);
    EXPECT_EQ(result.size(), images[0].size());
    EXPECT_NEAR(cv::mean(result)[0], 100.0, 1.0);
}

// Test different image sizes
TEST(StackImagesTest, DifferentImageSizes) {
    std::vector<cv::Mat> images;
    images.push_back(createTestImage(10, 10, 100));
    images.push_back(createTestImage(20, 20, 200));
    
    EXPECT_THROW(stackImages(images, MEAN, 2.0f, {}), std::runtime_error);
}