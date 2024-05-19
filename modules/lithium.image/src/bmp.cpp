#include <bit>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

union ByteUnion {
    uint32_t value;
    std::byte bytes[4];
};

uint32_t littleToNative(uint32_t little) {
    ByteUnion u;
    u.value = little;
#ifdef _WIN32
    return u.value;
#else
    return __builtin_bswap32(u.value);
#endif
}

uint16_t littleToNative(uint16_t little) {
    ByteUnion u;
    u.value = little;
#ifdef _WIN32
    return u.value >> 16;
#else
    return __builtin_bswap16(u.value);
#endif
}

struct Image {
    std::vector<std::byte> data;
    std::vector<std::byte> greyData;
    uint32_t sizeX, sizeY;
};

bool loadBMPImage(const std::string& filename, Image& image) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }

    file.ignore(18);  // Skip BMP header

    image.sizeX = readEndianInt(file);
    image.sizeY = readEndianInt(file);

    uint16_t planes = readEndianShort(file);
    if (planes != 1) {
        std::cerr << "Planes from " << filename << " is not 1: " << planes
                  << std::endl;
        return false;
    }

    uint16_t bpp = readEndianShort(file);
    if (bpp != 24) {
        std::cerr << "Bpp from " << filename << " is not 24: " << bpp
                  << std::endl;
        return false;
    }

    file.ignore(24);  // Skip the rest of the BMP header

    uint32_t size = image.sizeX * image.sizeY * 3;
    image.data.resize(size);
    if (!file.read(reinterpret_cast<char*>(image.data.data()), size)) {
        std::cerr << "Error reading image data from " << filename << std::endl;
        return false;
    }

    // Convert BGR to RGB
    for (uint32_t i = 0; i < size; i += 3) {
        std::swap(image.data[i], image.data[i + 2]);
    }

    uint32_t greySize = image.sizeX * image.sizeY;
    image.greyData.resize(greySize);

    // Convert to grayscale
    for (uint32_t i = 0; i < greySize; ++i) {
        uint32_t offset = i * 3;
        uint8_t r = static_cast<uint8_t>(image.data[offset + 2]);
        uint8_t g = static_cast<uint8_t>(image.data[offset + 1]);
        uint8_t b = static_cast<uint8_t>(image.data[offset]);
        uint8_t grey = (299 * r + 587 * g + 114 * b + 500) / 1000;
        image.greyData[i] = static_cast<std::byte>(grey);
    }

    return true;
}

uint32_t readEndianInt(std::ifstream& file) {
    uint32_t value;
    file.read(reinterpret_cast<char*>(&value), sizeof(value));
    return littleToNative(value);
}

uint16_t readEndianShort(std::ifstream& file) {
    uint16_t value;
    file.read(reinterpret_cast<char*>(&value), sizeof(value));
    return littleToNative(value);
}