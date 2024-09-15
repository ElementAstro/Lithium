#include "fits_data.hpp"
#include <algorithm>

template<typename T>
void TypedFITSData<T>::readData(std::ifstream& file, int64_t dataSize) {
    data.resize(dataSize / sizeof(T));
    file.read(reinterpret_cast<char*>(data.data()), dataSize);

    if (std::endian::native == std::endian::little) {
        for (auto& value : data) {
            swapEndian(value);
        }
    }
}

template<typename T>
void TypedFITSData<T>::writeData(std::ofstream& file) const {
    std::vector<T> tempData = data;
    if (std::endian::native == std::endian::little) {
        for (auto& value : tempData) {
            swapEndian(value);
        }
    }

    file.write(reinterpret_cast<const char*>(tempData.data()), tempData.size() * sizeof(T));

    // Pad the data to a multiple of 2880 bytes
    size_t padding = (2880 - (tempData.size() * sizeof(T)) % 2880) % 2880;
    std::vector<char> paddingData(padding, 0);
    file.write(paddingData.data(), padding);
}

template<typename T>
DataType TypedFITSData<T>::getDataType() const {
    if constexpr (std::is_same_v<T, uint8_t>) return DataType::BYTE;
    else if constexpr (std::is_same_v<T, int16_t>) return DataType::SHORT;
    else if constexpr (std::is_same_v<T, int32_t>) return DataType::INT;
    else if constexpr (std::is_same_v<T, int64_t>) return DataType::LONG;
    else if constexpr (std::is_same_v<T, float>) return DataType::FLOAT;
    else if constexpr (std::is_same_v<T, double>) return DataType::DOUBLE;
    else throw std::runtime_error("Unsupported data type");
}

template<typename T>
size_t TypedFITSData<T>::getElementCount() const {
    return data.size();
}

template<typename T>
template<typename U>
void TypedFITSData<T>::swapEndian(U& value) {
    uint8_t* bytes = reinterpret_cast<uint8_t*>(&value);
    std::reverse(bytes, bytes + sizeof(U));
}

// Explicit template instantiations
template class TypedFITSData<uint8_t>;
template class TypedFITSData<int16_t>;
template class TypedFITSData<int32_t>;
template class TypedFITSData<int64_t>;
template class TypedFITSData<float>;
template class TypedFITSData<double>;
