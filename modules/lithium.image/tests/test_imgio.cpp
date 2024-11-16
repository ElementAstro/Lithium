#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <opencv2/opencv.hpp>
#include "imgio.hpp"

namespace fs = std::filesystem;

class LoadImagesTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test directory and sample images
        fs::create_directory(test_dir);

        // Create valid test images
        cv::Mat img1(100, 100, CV_8UC3, cv::Scalar(255, 0, 0));
        cv::Mat img2(100, 100, CV_8UC3, cv::Scalar(0, 255, 0));
        cv::imwrite(test_dir + "/valid1.png", img1);
        cv::imwrite(test_dir + "/valid2.png", img2);

        // Create invalid/corrupt file
        std::ofstream corrupt_file(test_dir + "/corrupt.png");
        corrupt_file << "This is not a valid image file";
        corrupt_file.close();
    }

    void TearDown() override { fs::remove_all(test_dir); }

    const std::string test_dir = "test_images_temp";
};

// Test empty folder path
TEST_F(LoadImagesTest, EmptyFolderPath) {
    auto result = loadImages("", {}, cv::IMREAD_COLOR);
    EXPECT_TRUE(result.empty());
}

// Test non-existent folder
TEST_F(LoadImagesTest, NonExistentFolder) {
    auto result = loadImages("nonexistent_folder", {}, cv::IMREAD_COLOR);
    EXPECT_TRUE(result.empty());
}

// Test empty folder
TEST_F(LoadImagesTest, EmptyFolder) {
    fs::create_directory("empty_test_dir");
    auto result = loadImages("empty_test_dir", {}, cv::IMREAD_COLOR);
    EXPECT_TRUE(result.empty());
    fs::remove("empty_test_dir");
}

// Test loading valid images with no specific filenames
TEST_F(LoadImagesTest, LoadAllValidImages) {
    auto result = loadImages(test_dir, {}, cv::IMREAD_COLOR);
    EXPECT_EQ(result.size(), 2);  // Should find 2 valid images

    for (const auto& [filepath, img] : result) {
        EXPECT_FALSE(img.empty());
        EXPECT_EQ(img.rows, 100);
        EXPECT_EQ(img.cols, 100);
        EXPECT_EQ(img.channels(), 3);
    }
}

// Test loading with specific filenames
TEST_F(LoadImagesTest, LoadSpecificFiles) {
    std::vector<std::string> filenames = {"valid1.png"};
    auto result = loadImages(test_dir, filenames, cv::IMREAD_COLOR);
    EXPECT_EQ(result.size(), 1);
    EXPECT_FALSE(result[0].second.empty());
}

// Test loading with invalid filenames
TEST_F(LoadImagesTest, LoadInvalidFiles) {
    std::vector<std::string> filenames = {"nonexistent.png"};
    auto result = loadImages(test_dir, filenames, cv::IMREAD_COLOR);
    EXPECT_TRUE(result.empty());
}

// Test loading with mixed valid/invalid files
TEST_F(LoadImagesTest, LoadMixedFiles) {
    std::vector<std::string> filenames = {"valid1.png", "nonexistent.png",
                                          "valid2.png"};
    auto result = loadImages(test_dir, filenames, cv::IMREAD_COLOR);
    EXPECT_EQ(result.size(), 2);  // Should only load the two valid images
}

// Test loading with different flags
TEST_F(LoadImagesTest, LoadWithDifferentFlags) {
    auto result = loadImages(test_dir, {}, cv::IMREAD_GRAYSCALE);
    EXPECT_EQ(result.size(), 2);

    for (const auto& [filepath, img] : result) {
        EXPECT_FALSE(img.empty());
        EXPECT_EQ(img.channels(), 1);  // Should be grayscale
    }
}

// Test with corrupt image file
TEST_F(LoadImagesTest, LoadCorruptFile) {
    std::vector<std::string> filenames = {"corrupt.png"};
    auto result = loadImages(test_dir, filenames, cv::IMREAD_COLOR);
    EXPECT_TRUE(result.empty());
}

// Test with large number of files
TEST_F(LoadImagesTest, LoadManyFiles) {
    // Create many small test images
    for (int i = 0; i < 100; i++) {
        cv::Mat img(10, 10, CV_8UC3, cv::Scalar(i, i, i));
        cv::imwrite(test_dir + "/test" + std::to_string(i) + ".png", img);
    }

    auto result = loadImages(test_dir, {}, cv::IMREAD_COLOR);
    EXPECT_EQ(result.size(), 102);  // 100 new files + 2 original test files

    // Cleanup additional files
    for (int i = 0; i < 100; i++) {
        fs::remove(test_dir + "/test" + std::to_string(i) + ".png");
    }
}