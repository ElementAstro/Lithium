#include "bmp.hpp"

#include <algorithm>
#include <stdexcept>

#include "atom/log/loguru.hpp"

// 转换小端到本地字节序
uint32_t littleToNative(uint32_t little) {
    ByteUnion byteUnion;
    byteUnion.value = little;
#ifdef _WIN32
    return byteUnion.value;
#else
    return __builtin_bswap32(byteUnion.value);
#endif
}

uint16_t littleToNative(uint16_t little) {
    ByteUnion byteUnion;
    byteUnion.value = little;
#ifdef _WIN32
    return static_cast<uint16_t>(byteUnion.value >> 16);
#else
    return __builtin_bswap16(static_cast<uint16_t>(byteUnion.value));
#endif
}

Image::Image() : sizeX(0), sizeY(0) {}

uint32_t readEndianInt(std::ifstream& file) {
    uint32_t value;
    file.read(reinterpret_cast<char*>(&value), sizeof(value));
    if (!file) {
        LOG_F(ERROR, "Failed to read uint32_t from file.");
        throw std::runtime_error("Failed to read uint32_t from file.");
    }
    return littleToNative(value);
}

uint16_t readEndianShort(std::ifstream& file) {
    uint16_t value;
    file.read(reinterpret_cast<char*>(&value), sizeof(value));
    if (!file) {
        LOG_F(ERROR, "Failed to read uint16_t from file.");
        throw std::runtime_error("Failed to read uint16_t from file.");
    }
    return littleToNative(value);
}

bool loadBMPImage(const std::string& filename, Image& image) {
    try {
        LOG_F(INFO, "Loading BMP image: %s", filename.c_str());
        std::ifstream file(filename, std::ios::binary);
        if (!file) {
            LOG_F(ERROR, "Failed to open file: %s", filename.c_str());
            return false;
        }

        constexpr int BMP_HEADER_SIZE = 18;
        constexpr int BMP_HEADER_REST = 24;
        constexpr int BPP_24 = 24;
        file.ignore(BMP_HEADER_SIZE);  // Skip BMP header

        image.sizeX = readEndianInt(file);
        LOG_F(INFO, "Image width: %u", image.sizeX);
        image.sizeY = readEndianInt(file);
        LOG_F(INFO, "Image height: %u", image.sizeY);

        uint16_t planes = readEndianShort(file);
        LOG_F(INFO, "Planes: %u", planes);
        if (planes != 1) {
            LOG_F(ERROR, "Planes from %s is not 1: %u", filename.c_str(),
                  planes);
            return false;
        }

        uint16_t bpp = readEndianShort(file);
        LOG_F(INFO, "Bits per pixel: %u", bpp);
        if (bpp != BPP_24) {
            LOG_F(ERROR, "Bpp from %s is not 24: %u", filename.c_str(), bpp);
            return false;
        }

        file.ignore(BMP_HEADER_REST);  // Skip the rest of the BMP header

        uint32_t size = image.sizeX * image.sizeY * 3;
        image.data.resize(size);
        if (!file.read(reinterpret_cast<char*>(image.data.data()), size)) {
            LOG_F(ERROR, "Error reading image data from %s", filename.c_str());
            return false;
        }
        LOG_F(INFO, "Image data read successfully.");

        // Convert BGR to RGB
        for (uint32_t i = 0; i < size; i += 3) {
            std::swap(image.data[i], image.data[i + 2]);
        }
        LOG_F(INFO, "Converted BGR to RGB.");

        uint32_t greySize = image.sizeX * image.sizeY;
        image.greyData.resize(greySize);

        // Convert to grayscale
        constexpr int RED_WEIGHT = 299;
        constexpr int GREEN_WEIGHT = 587;
        constexpr int BLUE_WEIGHT = 114;
        constexpr int ROUNDING_OFFSET = 500;
        constexpr int SCALE = 1000;

        for (uint32_t i = 0; i < greySize; ++i) {
            uint32_t offset = i * 3;
            auto red = static_cast<uint8_t>(image.data[offset + 2]);
            auto green = static_cast<uint8_t>(image.data[offset + 1]);
            auto blue = static_cast<uint8_t>(image.data[offset]);
            uint8_t grey = (RED_WEIGHT * red + GREEN_WEIGHT * green +
                            BLUE_WEIGHT * blue + ROUNDING_OFFSET) /
                           SCALE;
            image.greyData[i] = static_cast<std::byte>(grey);
        }
        LOG_F(INFO, "Converted image to grayscale.");

        return true;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception occurred: %s", e.what());
        return false;
    }
}

bool saveGrayImage(const std::string& filename, const Image& image) {
    try {
        LOG_F(INFO, "Saving grayscale image to: %s", filename.c_str());
        std::ofstream outFile(filename, std::ios::binary);
        if (!outFile) {
            LOG_F(ERROR, "Failed to open file for writing: %s",
                  filename.c_str());
            return false;
        }

        // 简单的灰度图保存为原始数据
        outFile.write(reinterpret_cast<const char*>(image.greyData.data()),
                      image.greyData.size());
        if (!outFile) {
            LOG_F(ERROR, "Failed to write grayscale data to: %s",
                  filename.c_str());
            return false;
        }

        LOG_F(INFO, "Grayscale image saved successfully.");
        return true;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception while saving grayscale image: %s", e.what());
        return false;
    }
}

int main(int argc, char* argv[]) {
    loguru::init(argc, argv);
    if (argc < 3) {
        LOG_F(ERROR, "Usage: %s <input_filename> <output_grey_filename>",
              argv[0]);
        return 1;
    }

    Image image;
    if (loadBMPImage(argv[1], image)) {
        LOG_F(INFO, "BMP image loaded successfully.");
        if (saveGrayImage(argv[2], image)) {
            LOG_F(INFO, "Grayscale image saved successfully.");
        } else {
            LOG_F(ERROR, "Failed to save grayscale image.");
            return 1;
        }
    } else {
        LOG_F(ERROR, "Failed to load BMP image.");
        return 1;
    }

    return 0;
}