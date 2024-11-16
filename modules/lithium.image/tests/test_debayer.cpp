#include <gtest/gtest.h>
#include <opencv2/core/hal/interface.h>
#include <opencv2/opencv.hpp>

#include "debayer.hpp"

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

class DebayerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试目录
        fs::create_directories("test_images");

        // 创建测试图像
        createTestImages();
    }

    void TearDown() override {
        // 清理测试文件
        fs::remove_all("test_images");
    }

    void createTestImages() {
        // 创建单通道测试图像
        cv::Mat mono(100, 100, CV_8UC1, cv::Scalar(128));
        cv::imwrite("test_images/mono.png", mono);

        // 创建RGB测试图像
        cv::Mat rgb(100, 100, CV_8UC3, cv::Scalar(64, 128, 192));
        cv::imwrite("test_images/rgb.png", rgb);

        // 创建测试FITS文件
        createTestFits("test_images/rggb.fits", "RGGB");
        createTestFits("test_images/bggr.fits", "BGGR");
    }

    void createTestFits(const std::string& filename,
                        const std::string& pattern) {
        // 模拟创建FITS文件的实现
        cv::Mat raw(100, 100, CV_16UC1, cv::Scalar(4096));
        // 这里需要实现具体的FITS文件创建逻辑
    }
};

// 测试处理单通道图像
TEST_F(DebayerTest, ProcessMonoImage) {
    fs::path monoPath("test_images/mono.png");
    auto result = debayer(monoPath);

    EXPECT_TRUE(result.continueProcessing);
    EXPECT_FALSE(result.debayeredImage.empty());
    EXPECT_EQ(result.debayeredImage.channels(), 1);
}

// 测试处理RGB图像
TEST_F(DebayerTest, ProcessRGBImage) {
    fs::path rgbPath("test_images/rgb.png");
    auto result = debayer(rgbPath);

    EXPECT_TRUE(result.continueProcessing);
    EXPECT_FALSE(result.debayeredImage.empty());
    EXPECT_EQ(result.debayeredImage.channels(), 3);
}

// 测试RGGB FITS文件处理
TEST_F(DebayerTest, ProcessRGGBFits) {
    fs::path fitsPath("test_images/rggb.fits");
    auto result = debayer(fitsPath);

    EXPECT_TRUE(result.continueProcessing);
    EXPECT_FALSE(result.debayeredImage.empty());
    EXPECT_EQ(result.debayeredImage.channels(), 3);
    EXPECT_EQ(result.header["BayerPattern"], "RGGB");
}

// 测试无效文件路径
TEST_F(DebayerTest, InvalidFilePath) {
    fs::path invalidPath("non_existent.png");
    EXPECT_THROW(debayer(invalidPath), std::runtime_error);
}

// 测试无效的Bayer模式
TEST_F(DebayerTest, InvalidBayerPattern) {
    fs::path fitsPath("test_images/invalid_pattern.fits");
    createTestFits(fitsPath.string(), "INVALID");

    auto result = debayer(fitsPath);
    EXPECT_FALSE(result.continueProcessing);
}

// 测试空文件
TEST_F(DebayerTest, EmptyFile) {
    fs::path emptyPath("test_images/empty.png");
    std::ofstream ofs(emptyPath);
    ofs.close();

    EXPECT_THROW(debayer(emptyPath), std::runtime_error);
}

// 测试不支持的通道数
TEST_F(DebayerTest, UnsupportedChannels) {
    cv::Mat unsupported(100, 100, CV_8UC4, cv::Scalar(0));
    cv::imwrite("test_images/unsupported.png", unsupported);

    fs::path unsupportedPath("test_images/unsupported.png");
    EXPECT_THROW(debayer(unsupportedPath), std::invalid_argument);
}

// 测试文件扩展名处理
TEST_F(DebayerTest, FileExtensionHandling) {
    fs::path upperPath("test_images/TEST.FITS");
    fs::path lowerPath("test_images/test.fits");

    createTestFits(upperPath.string(), "RGGB");
    createTestFits(lowerPath.string(), "RGGB");

    auto resultUpper = debayer(upperPath);
    auto resultLower = debayer(lowerPath);

    EXPECT_TRUE(resultUpper.continueProcessing);
    EXPECT_TRUE(resultLower.continueProcessing);
}