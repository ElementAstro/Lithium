#ifndef TEST_BMP_HPP
#define TEST_BMP_HPP

#include <gtest/gtest.h>

#include "bmp.hpp"

#include "atom/type/uint.hpp"

// Mock class to simulate file reading
class MockFile {
public:
    MockFile(const std::vector<uint8_t>& data) : data(data), pos(0) {}

    void read(char* buffer, std::streamsize size) {
        if (pos + size > data.size()) {
            throw std::runtime_error("Read beyond end of file");
        }
        std::copy(data.begin() + pos, data.begin() + pos + size, buffer);
        pos += size;
    }

    void ignore(std::streamsize size) {
        if (pos + size > data.size()) {
            throw std::runtime_error("Ignore beyond end of file");
        }
        pos += size;
    }

    explicit operator bool() const { return pos < data.size(); }

private:
    std::vector<uint8_t> data;
    std::size_t pos;
};

TEST(BMPTest, LittleToNative32) {
    EXPECT_EQ(littleToNative(0x12345678_u32), 0x78563412);
}

TEST(BMPTest, LittleToNative16) {
    EXPECT_EQ(littleToNative(static_cast<uint16_t>(0x1234)), 0x3412);
}

TEST(BMPTest, ReadEndianInt) {
    const std::vector<uint8_t> data = {0x78, 0x56, 0x34, 0x12};
    MockFile file(data);
    EXPECT_EQ(readEndianInt(reinterpret_cast<std::ifstream&>(file)),
              0x12345678);
}

TEST(BMPTest, ReadEndianShort) {
    const std::vector<uint8_t> data = {0x34, 0x12};
    MockFile file(data);
    EXPECT_EQ(readEndianShort(reinterpret_cast<std::ifstream&>(file)), 0x1234);
}

TEST(BMPTest, LoadBMPImage) {
    // Mock BMP data
    std::vector<uint8_t> bmpData = {
        // BMP header (18 bytes)
        0x42, 0x4D, 0x36, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x00,
        0x00, 0x00, 0x28, 0x00, 0x00, 0x00,
        // Width (4 bytes)
        0x02, 0x00, 0x00, 0x00,
        // Height (4 bytes)
        0x02, 0x00, 0x00, 0x00,
        // Planes (2 bytes)
        0x01, 0x00,
        // Bits per pixel (2 bytes)
        0x18, 0x00,
        // Rest of BMP header (24 bytes)
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        // Pixel data (2x2 pixels, 24 bits per pixel, BGR format)
        0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF};

    MockFile file(bmpData);
    Image image;
    EXPECT_TRUE(loadBMPImage("mock.bmp", image));
    EXPECT_EQ(image.sizeX, 2);
    EXPECT_EQ(image.sizeY, 2);
    EXPECT_EQ(image.data.size(), 12);
    EXPECT_EQ(image.greyData.size(), 4);
}

TEST(BMPTest, SaveGrayImage) {
    Image image;
    image.sizeX = 2;
    image.sizeY = 2;
    image.greyData = {std::byte{0x80}, std::byte{0x80}, std::byte{0x80},
                      std::byte{0x80}};

    std::ofstream outFile("test_gray_image.raw", std::ios::binary);
    EXPECT_TRUE(saveGrayImage("test_gray_image.raw", image));
    outFile.close();

    std::ifstream inFile("test_gray_image.raw", std::ios::binary);
    std::vector<char> readData((std::istreambuf_iterator<char>(inFile)),
                               std::istreambuf_iterator<char>());
    EXPECT_EQ(readData, image.greyData);
}

#endif  // TEST_BMP_HPP