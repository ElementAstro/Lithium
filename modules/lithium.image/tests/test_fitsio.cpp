#include "fitsio.hpp"

#include <fitsio.h>
#include <gtest/gtest.h>
#include <opencv2/imgcodecs.hpp>
#include <filesystem>
#include <fstream>

class FitsIOTest : public ::testing::Test {
protected:
    void SetUp() override {
        testDir = std::filesystem::temp_directory_path() / "fits_test";
        std::filesystem::create_directories(testDir);
    }

    void TearDown() override {
        std::filesystem::remove_all(testDir);
    }

    // Helper to create a test FITS file
    void createTestFits(const std::string& filename, int width, int height, 
                       int bitpix, int naxis, bool isRGB = false) {
        fitsfile* fptr;
        int status = 0;
        std::array<long, 3> naxes = {width, height, isRGB ? 3 : 1};
        
        std::filesystem::path filepath = testDir / filename;
        fits_create_file(&fptr, filepath.string().c_str(), &status);
        fits_create_img(fptr, bitpix, isRGB ? 3 : 2, naxes.data(), &status);
        
        // Add test header
        fits_write_key(fptr, TSTRING, "INSTRUME", const_cast<char*>("TEST_CAMERA"), 
                      "Test instrument", &status);
        
        // Write test data
        std::vector<unsigned short> data(width * height, 1000);
        fits_write_img(fptr, TUSHORT, 1, width * height, data.data(), &status);
        
        fits_close_file(fptr, &status);
    }

    std::filesystem::path testDir;
};

// Test reading non-existent file
TEST_F(FitsIOTest, NonExistentFile) {
    std::map<std::string, std::string> header;
    EXPECT_THROW(readFitsToMat(testDir / "nonexistent.fits", header), 
                 std::runtime_error);
}

// Test reading 8-bit grayscale
TEST_F(FitsIOTest, Read8BitGrayscale) {
    createTestFits("test8bit.fits", 100, 100, BYTE_IMG, 2);
    std::map<std::string, std::string> header;
    
    cv::Mat result = readFitsToMat(testDir / "test8bit.fits", header);
    
    EXPECT_EQ(result.type(), CV_8UC1);
    EXPECT_EQ(result.size(), cv::Size(100, 100));
}

// Test reading 16-bit grayscale
TEST_F(FitsIOTest, Read16BitGrayscale) {
    createTestFits("test16bit.fits", 100, 100, USHORT_IMG, 2);
    std::map<std::string, std::string> header;
    
    cv::Mat result = readFitsToMat(testDir / "test16bit.fits", header);
    
    EXPECT_EQ(result.type(), CV_16UC1);
    EXPECT_EQ(result.size(), cv::Size(100, 100));
}

// Test reading 32-bit float
TEST_F(FitsIOTest, Read32BitFloat) {
    createTestFits("test32bit.fits", 100, 100, FLOAT_IMG, 2);
    std::map<std::string, std::string> header;
    
    cv::Mat result = readFitsToMat(testDir / "test32bit.fits", header);
    
    EXPECT_EQ(result.type(), CV_32FC1);
    EXPECT_EQ(result.size(), cv::Size(100, 100));
}

// Test reading RGB image
TEST_F(FitsIOTest, ReadRGBImage) {
    createTestFits("testrgb.fits", 100, 100, USHORT_IMG, 3, true);
    std::map<std::string, std::string> header;
    
    cv::Mat result = readFitsToMat(testDir / "testrgb.fits", header);
    
    EXPECT_EQ(result.channels(), 3);
    EXPECT_EQ(result.size(), cv::Size(100, 100));
}

// Test header extraction
TEST_F(FitsIOTest, HeaderExtraction) {
    createTestFits("testheader.fits", 100, 100, USHORT_IMG, 2);
    std::map<std::string, std::string> header;
    
    readFitsToMat(testDir / "testheader.fits", header);
    
    EXPECT_FALSE(header.empty());
    EXPECT_EQ(header["INSTRUME"], "TEST_CAMERA");
}

// Test invalid FITS file
TEST_F(FitsIOTest, InvalidFitsFile) {
    std::ofstream badFile(testDir / "bad.fits");
    badFile << "Not a FITS file";
    badFile.close();
    
    std::map<std::string, std::string> header;
    EXPECT_THROW(readFitsToMat(testDir / "bad.fits", header), 
                 std::runtime_error);
}

// Test different image sizes
TEST_F(FitsIOTest, DifferentSizes) {
    createTestFits("small.fits", 10, 10, USHORT_IMG, 2);
    createTestFits("large.fits", 1000, 1000, USHORT_IMG, 2);
    
    std::map<std::string, std::string> header;
    
    cv::Mat small = readFitsToMat(testDir / "small.fits", header);
    cv::Mat large = readFitsToMat(testDir / "large.fits", header);
    
    EXPECT_EQ(small.size(), cv::Size(10, 10));
    EXPECT_EQ(large.size(), cv::Size(1000, 1000));
}

// Test unsupported bit depth
TEST_F(FitsIOTest, UnsupportedBitDepth) {
    createTestFits("unsupported.fits", 100, 100, DOUBLE_IMG, 2);
    std::map<std::string, std::string> header;
    
    EXPECT_THROW(readFitsToMat(testDir / "unsupported.fits", header), 
                 std::runtime_error);
}

// Test empty header map
TEST_F(FitsIOTest, EmptyHeader) {
    createTestFits("testempty.fits", 100, 100, USHORT_IMG, 2);
    std::map<std::string, std::string> header;
    
    cv::Mat result = readFitsToMat(testDir / "testempty.fits", header);
    
    EXPECT_FALSE(header.empty());
}

// Test invalid dimensions
TEST_F(FitsIOTest, InvalidDimensions) {
    fitsfile* fptr;
    int status = 0;
    std::array<long, 4> naxes = {100, 100, 100, 100};
    
    std::filesystem::path filepath = testDir / "invalid_dim.fits";
    fits_create_file(&fptr, filepath.string().c_str(), &status);
    fits_create_img(fptr, USHORT_IMG, 4, naxes.data(), &status);
    fits_close_file(fptr, &status);
    
    std::map<std::string, std::string> header;
    EXPECT_THROW(readFitsToMat(filepath, header), std::runtime_error);
}