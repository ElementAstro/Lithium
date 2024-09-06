#include "hdu.hpp"
#include <cmath>
#include <numeric>
#include <stdexcept>

void HDU::setHeaderKeyword(const std::string& keyword,
                           const std::string& value) {
    header.addKeyword(keyword, value);
}

std::string HDU::getHeaderKeyword(const std::string& keyword) const {
    return header.getKeywordValue(keyword);
}

void ImageHDU::readHDU(std::ifstream& file) {
    std::vector<char> headerData(FITSHeader::FITS_HEADER_UNIT_SIZE);
    file.read(headerData.data(), headerData.size());
    header.deserialize(headerData);

    width = std::stoi(header.getKeywordValue("NAXIS1"));
    height = std::stoi(header.getKeywordValue("NAXIS2"));
    channels = std::stoi(header.getKeywordValue("NAXIS3"));
    int bitpix = std::stoi(header.getKeywordValue("BITPIX"));

    switch (bitpix) {
        case 8:
            initializeData<uint8_t>();
            break;
        case 16:
            initializeData<int16_t>();
            break;
        case 32:
            initializeData<int32_t>();
            break;
        case 64:
            initializeData<int64_t>();
            break;
        case -32:
            initializeData<float>();
            break;
        case -64:
            initializeData<double>();
            break;
        default:
            throw std::runtime_error("Unsupported BITPIX value");
    }

    int64_t dataSize =
        static_cast<int64_t>(width) * height * channels * std::abs(bitpix) / 8;
    data->readData(file, dataSize);
}

void ImageHDU::writeHDU(std::ofstream& file) const {
    auto headerData = header.serialize();
    file.write(headerData.data(), headerData.size());
    data->writeData(file);
}

void ImageHDU::setImageSize(int w, int h, int c) {
    width = w;
    height = h;
    channels = c;
    header.addKeyword("NAXIS1", std::to_string(width));
    header.addKeyword("NAXIS2", std::to_string(height));
    if (channels > 1) {
        header.addKeyword("NAXIS", "3");
        header.addKeyword("NAXIS3", std::to_string(channels));
    } else {
        header.addKeyword("NAXIS", "2");
    }
}

std::tuple<int, int, int> ImageHDU::getImageSize() const {
    return {width, height, channels};
}

template <typename T>
void ImageHDU::setPixel(int x, int y, T value, int channel) {
    if (x < 0 || x >= width || y < 0 || y >= height || channel < 0 ||
        channel >= channels) {
        throw std::out_of_range("Pixel coordinates or channel out of range");
    }
    auto& typedData = static_cast<TypedFITSData<T>&>(*data);
    typedData.getData()[(y * width + x) * channels + channel] = value;
}

template <typename T>
T ImageHDU::getPixel(int x, int y, int channel) const {
    if (x < 0 || x >= width || y < 0 || y >= height || channel < 0 ||
        channel >= channels) {
        throw std::out_of_range("Pixel coordinates or channel out of range");
    }
    const auto& typedData = static_cast<const TypedFITSData<T>&>(*data);
    return typedData.getData()[(y * width + x) * channels + channel];
}

template <typename T>
typename ImageHDU::template ImageStats<T> ImageHDU::computeImageStats(
    int channel) const {
    const auto& typedData = static_cast<const TypedFITSData<T>&>(*data);
    const auto& pixelData = typedData.getData();

    T min = std::numeric_limits<T>::max();
    T max = std::numeric_limits<T>::lowest();
    double sum = 0.0;

    for (int i = channel; i < pixelData.size(); i += channels) {
        T pixel = pixelData[i];
        min = std::min(min, pixel);
        max = std::max(max, pixel);
        sum += static_cast<double>(pixel);
    }

    size_t pixelCount = width * height;
    double mean = sum / pixelCount;
    double variance = 0.0;

    for (int i = channel; i < pixelData.size(); i += channels) {
        double diff = static_cast<double>(pixelData[i]) - mean;
        variance += diff * diff;
    }

    variance /= pixelCount;
    double stddev = std::sqrt(variance);

    return {min, max, mean, stddev};
}

template <typename T>
void ImageHDU::applyFilter(const std::vector<std::vector<double>>& kernel,
                           int channel) {
    auto& typedData = static_cast<TypedFITSData<T>&>(*data);
    auto& pixelData = typedData.getData();

    int kernelHeight = kernel.size();
    int kernelWidth = kernel[0].size();
    int kernelCenterY = kernelHeight / 2;
    int kernelCenterX = kernelWidth / 2;

    std::vector<T> newPixelData(pixelData.size());

    for (int c = 0; c < channels; ++c) {
        if (channel != -1 && c != channel)
            continue;

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                double sum = 0.0;
                for (int ky = 0; ky < kernelHeight; ++ky) {
                    for (int kx = 0; kx < kernelWidth; ++kx) {
                        int imgY = y + ky - kernelCenterY;
                        int imgX = x + kx - kernelCenterX;
                        if (imgY >= 0 && imgY < height && imgX >= 0 &&
                            imgX < width) {
                            sum +=
                                kernel[ky][kx] *
                                pixelData[(imgY * width + imgX) * channels + c];
                        }
                    }
                }
                newPixelData[(y * width + x) * channels + c] =
                    static_cast<T>(sum);
            }
        }
    }

    pixelData = std::move(newPixelData);
}

template <typename T>
void ImageHDU::initializeData() {
    data = std::make_unique<TypedFITSData<T>>();
    auto& typedData = static_cast<TypedFITSData<T>&>(*data);
    typedData.getData().resize(width * height * channels);
}

// Explicit template instantiations
template void ImageHDU::setPixel<uint8_t>(int, int, uint8_t, int);
template void ImageHDU::setPixel<int16_t>(int, int, int16_t, int);
template void ImageHDU::setPixel<int32_t>(int, int, int32_t, int);
template void ImageHDU::setPixel<int64_t>(int, int, int64_t, int);
template void ImageHDU::setPixel<float>(int, int, float, int);
template void ImageHDU::setPixel<double>(int, int, double, int);

template uint8_t ImageHDU::getPixel<uint8_t>(int, int, int) const;
template int16_t ImageHDU::getPixel<int16_t>(int, int, int) const;
template int32_t ImageHDU::getPixel<int32_t>(int, int, int) const;
template int64_t ImageHDU::getPixel<int64_t>(int, int, int) const;
template float ImageHDU::getPixel<float>(int, int, int) const;
template double ImageHDU::getPixel<double>(int, int, int) const;

template ImageHDU::ImageStats<uint8_t> ImageHDU::computeImageStats<uint8_t>(
    int) const;
template ImageHDU::ImageStats<int16_t> ImageHDU::computeImageStats<int16_t>(
    int) const;
template ImageHDU::ImageStats<int32_t> ImageHDU::computeImageStats<int32_t>(
    int) const;
template ImageHDU::ImageStats<int64_t> ImageHDU::computeImageStats<int64_t>(
    int) const;
template ImageHDU::ImageStats<float> ImageHDU::computeImageStats<float>(
    int) const;
template ImageHDU::ImageStats<double> ImageHDU::computeImageStats<double>(
    int) const;

template void ImageHDU::applyFilter<uint8_t>(
    const std::vector<std::vector<double>>&, int);
template void ImageHDU::applyFilter<int16_t>(
    const std::vector<std::vector<double>>&, int);
template void ImageHDU::applyFilter<int32_t>(
    const std::vector<std::vector<double>>&, int);
template void ImageHDU::applyFilter<int64_t>(
    const std::vector<std::vector<double>>&, int);
template void ImageHDU::applyFilter<float>(
    const std::vector<std::vector<double>>&, int);
template void ImageHDU::applyFilter<double>(
    const std::vector<std::vector<double>>&, int);