#pragma once

#include <vector>
#include <fstream>
#include <cstdint>
#include <bit>
#include <stdexcept>
#include <type_traits>

enum class DataType {
    BYTE, SHORT, INT, LONG, FLOAT, DOUBLE
};

class FITSData {
public:
    virtual ~FITSData() = default;
    virtual void readData(std::ifstream& file, int64_t dataSize) = 0;
    virtual void writeData(std::ofstream& file) const = 0;
    virtual DataType getDataType() const = 0;
    virtual size_t getElementCount() const = 0;
};

template<typename T>
class TypedFITSData : public FITSData {
public:
    void readData(std::ifstream& file, int64_t dataSize) override;
    void writeData(std::ofstream& file) const override;
    DataType getDataType() const override;
    size_t getElementCount() const override;

    const std::vector<T>& getData() const { return data; }
    std::vector<T>& getData() { return data; }

private:
    std::vector<T> data;

    template<typename U>
    static void swapEndian(U& value);
};
