#include "fits_data.hpp"
#include <algorithm>
#include <bit>
#include <fstream>
#include <stdexcept>
#include <vector>

namespace {
constexpr size_t FITS_BLOCK_SIZE = 2880;
}

class UnsupportedDataTypeException : public std::runtime_error {
public:
    explicit UnsupportedDataTypeException(const std::string& message)
        : std::runtime_error(message) {}
};

template <typename T>
void TypedFITSData<T>::readData(std::ifstream& file, int64_t dataSize) {
    data.resize(dataSize / sizeof(T));
    file.read(reinterpret_cast<char*>(std::bit_cast<std::byte*>(data.data())),
              dataSize);

    if (std::endian::native == std::endian::little) {
#pragma unroll
        for (auto& value : data) {
            swapEndian(value);
        }
    }
}

template <typename T>
void TypedFITSData<T>::writeData(std::ofstream& file) const {
    std::vector<T> tempData = data;
    if (std::endian::native == std::endian::little) {
#pragma unroll
        for (auto& value : tempData) {
            swapEndian(value);
        }
    }

    file.write(reinterpret_cast<const char*>(
                   std::bit_cast<const std::byte*>(tempData.data())),
               tempData.size() * sizeof(T));

    // Pad the data to a multiple of FITS_BLOCK_SIZE bytes
    size_t padding =
        (FITS_BLOCK_SIZE - (tempData.size() * sizeof(T)) % FITS_BLOCK_SIZE) %
        FITS_BLOCK_SIZE;
    std::vector<std::byte> paddingData(padding, std::byte{0});
    file.write(reinterpret_cast<const char*>(paddingData.data()),
               static_cast<std::streamsize>(padding));
}

template <typename T>
auto TypedFITSData<T>::getDataType() const -> DataType {
    using enum DataType;
    if constexpr (std::is_same_v<T, uint8_t>) {
        return BYTE;
    } else if constexpr (std::is_same_v<T, int16_t>) {
        return SHORT;
    } else if constexpr (std::is_same_v<T, int32_t>) {
        return INT;
    } else if constexpr (std::is_same_v<T, int64_t>) {
        return LONG;
    } else if constexpr (std::is_same_v<T, float>) {
        return FLOAT;
    } else if constexpr (std::is_same_v<T, double>) {
        return DOUBLE;
    } else {
        throw UnsupportedDataTypeException("Unsupported data type");
    }
}

template <typename T>
auto TypedFITSData<T>::getElementCount() const -> size_t {
    return data.size();
}

template <typename T>
template <typename U>
void TypedFITSData<T>::swapEndian(U& value) {
    auto* bytes = std::bit_cast<std::byte*>(&value);
    std::reverse(bytes, bytes + sizeof(U));
}

// Explicit template instantiations
template class TypedFITSData<uint8_t>;
template class TypedFITSData<int16_t>;
template class TypedFITSData<int32_t>;
template class TypedFITSData<int64_t>;
template class TypedFITSData<float>;
template class TypedFITSData<double>;