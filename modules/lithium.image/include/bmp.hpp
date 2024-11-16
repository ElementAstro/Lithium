#ifndef BMP_H
#define BMP_H

#include <array>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

union ByteUnion {
    uint32_t value;
    std::array<std::byte, 4> bytes;
};

uint32_t littleToNative(uint32_t little);
uint16_t littleToNative(uint16_t little);

struct alignas(64) Image {
    std::vector<std::byte> data;
    std::vector<std::byte> greyData;
    uint32_t sizeX, sizeY;

    Image();
};

uint32_t readEndianInt(std::ifstream& file);
uint16_t readEndianShort(std::ifstream& file);

bool loadBMPImage(const std::string& filename, Image& image);
bool saveGrayImage(const std::string& filename, const Image& image);

int main(int argc, char* argv[]);

#endif  // BMP_H