#include "hfr.hpp"
#include "imgutils.hpp"

#include <gtest/gtest.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

using namespace cv;
using namespace std;

// Helper function to create a synthetic image with contours
cv::Mat createSyntheticImageWithContours() {
    cv::Mat img = cv::Mat::zeros(100, 100, CV_8UC1);
    cv::circle(img, cv::Point(50, 50), 20, cv::Scalar(255), -1);
    cv::rectangle(img, cv::Point(10, 10), cv::Point(30, 30), cv::Scalar(255),
                  -1);
    return img;
}

// Test case for processContours with an empty image
TEST(ProcessContoursTest, EmptyImage) {
    cv::Mat grayImg = cv::Mat::zeros(100, 100, CV_8UC1);
    cv::Mat rgbImg = cv::Mat::zeros(100, 100, CV_8UC3);
    cv::Mat markImg;
    std::vector<std::vector<cv::Point>> contours;
    bool doStarMark = false;

    auto [starnum, avghfr, hfrList, arelist] =
        processContours(grayImg, rgbImg, markImg, contours, doStarMark);
    EXPECT_EQ(starnum, 0);
    EXPECT_EQ(avghfr, 0.0);
    EXPECT_TRUE(hfrList.empty());
    EXPECT_TRUE(arelist.empty());
}

// Test case for processContours with a synthetic image with contours
TEST(ProcessContoursTest, SyntheticImageWithContours) {
    cv::Mat grayImg = createSyntheticImageWithContours();
    cv::Mat rgbImg;
    cv::cvtColor(grayImg, rgbImg, cv::COLOR_GRAY2BGR);
    cv::Mat markImg;
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(grayImg, contours, cv::RETR_EXTERNAL,
                     cv::CHAIN_APPROX_SIMPLE);
    bool doStarMark = false;

    auto [starnum, avghfr, hfrList, arelist] =
        processContours(grayImg, rgbImg, markImg, contours, doStarMark);
    EXPECT_GT(starnum, 0);
    EXPECT_GT(avghfr, 0.0);
    EXPECT_FALSE(hfrList.empty());
    EXPECT_FALSE(arelist.empty());
}

// Test case for processContours with a synthetic image and star marking enabled
TEST(ProcessContoursTest, SyntheticImageWithContoursAndStarMarking) {
    cv::Mat grayImg = createSyntheticImageWithContours();
    cv::Mat rgbImg;
    cv::cvtColor(grayImg, rgbImg, cv::COLOR_GRAY2BGR);
    cv::Mat markImg;
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(grayImg, contours, cv::RETR_EXTERNAL,
                     cv::CHAIN_APPROX_SIMPLE);
    bool doStarMark = true;

    auto [starnum, avghfr, hfrList, arelist] =
        processContours(grayImg, rgbImg, markImg, contours, doStarMark);
    EXPECT_GT(starnum, 0);
    EXPECT_GT(avghfr, 0.0);
    EXPECT_FALSE(hfrList.empty());
    EXPECT_FALSE(arelist.empty());
    EXPECT_FALSE(markImg.empty());
}

// Test case for processContours with a single contour
TEST(ProcessContoursTest, SingleContour) {
    cv::Mat grayImg = cv::Mat::zeros(100, 100, CV_8UC1);
    cv::circle(grayImg, cv::Point(50, 50), 20, cv::Scalar(255), -1);
    cv::Mat rgbImg;
    cv::cvtColor(grayImg, rgbImg, cv::COLOR_GRAY2BGR);
    cv::Mat markImg;
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(grayImg, contours, cv::RETR_EXTERNAL,
                     cv::CHAIN_APPROX_SIMPLE);
    bool doStarMark = false;

    auto [starnum, avghfr, hfrList, arelist] =
        processContours(grayImg, rgbImg, markImg, contours, doStarMark);
    EXPECT_EQ(starnum, 1);
    EXPECT_GT(avghfr, 0.0);
    EXPECT_EQ(hfrList.size(), 1);
    EXPECT_EQ(arelist.size(), 1);
}

// Test case for processContours with multiple contours
TEST(ProcessContoursTest, MultipleContours) {
    cv::Mat grayImg = cv::Mat::zeros(100, 100, CV_8UC1);
    cv::circle(grayImg, cv::Point(50, 50), 20, cv::Scalar(255), -1);
    cv::rectangle(grayImg, cv::Point(10, 10), cv::Point(30, 30),
                  cv::Scalar(255), -1);
    cv::Mat rgbImg;
    cv::cvtColor(grayImg, rgbImg, cv::COLOR_GRAY2BGR);
    cv::Mat markImg;
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(grayImg, contours, cv::RETR_EXTERNAL,
                     cv::CHAIN_APPROX_SIMPLE);
    bool doStarMark = false;

    auto [starnum, avghfr, hfrList, arelist] =
        processContours(grayImg, rgbImg, markImg, contours, doStarMark);
    EXPECT_EQ(starnum, 2);
    EXPECT_GT(avghfr, 0.0);
    EXPECT_EQ(hfrList.size(), 2);
    EXPECT_EQ(arelist.size(), 2);
}

// Test case for processContours with elongated contour
TEST(ProcessContoursTest, ElongatedContour) {
    cv::Mat grayImg = cv::Mat::zeros(100, 100, CV_8UC1);
    cv::rectangle(grayImg, cv::Point(10, 10), cv::Point(90, 20),
                  cv::Scalar(255), -1);
    cv::Mat rgbImg;
    cv::cvtColor(grayImg, rgbImg, cv::COLOR_GRAY2BGR);
    cv::Mat markImg;
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(grayImg, contours, cv::RETR_EXTERNAL,
                     cv::CHAIN_APPROX_SIMPLE);
    bool doStarMark = false;

    auto [starnum, avghfr, hfrList, arelist] =
        processContours(grayImg, rgbImg, markImg, contours, doStarMark);
    EXPECT_EQ(starnum, 0);
    EXPECT_EQ(avghfr, 0.0);
    EXPECT_TRUE(hfrList.empty());
    EXPECT_TRUE(arelist.empty());
}

// Test case for processContours with contour out of bounds
TEST(ProcessContoursTest, ContourOutOfBounds) {
    cv::Mat grayImg = cv::Mat::zeros(100, 100, CV_8UC1);
    cv::circle(grayImg, cv::Point(95, 95), 10, cv::Scalar(255), -1);
    cv::Mat rgbImg;
    cv::cvtColor(grayImg, rgbImg, cv::COLOR_GRAY2BGR);
    cv::Mat markImg;
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(grayImg, contours, cv::RETR_EXTERNAL,
                     cv::CHAIN_APPROX_SIMPLE);
    bool doStarMark = false;

    auto [starnum, avghfr, hfrList, arelist] =
        processContours(grayImg, rgbImg, markImg, contours, doStarMark);
    EXPECT_EQ(starnum, 0);
    EXPECT_EQ(avghfr, 0.0);
    EXPECT_TRUE(hfrList.empty());
    EXPECT_TRUE(arelist.empty());
}
