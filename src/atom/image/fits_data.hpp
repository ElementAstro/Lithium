#ifndef ATOM_IMAGE_FITS_DATA_HPP
#define ATOM_IMAGE_FITS_DATA_HPP

#include <cstdint>
#include <fstream>
#include <vector>

/**
 * @enum DataType
 * @brief Enum representing different data types that can be stored in FITS
 * files.
 */
enum class DataType { BYTE, SHORT, INT, LONG, FLOAT, DOUBLE };

/**
 * @class FITSData
 * @brief Abstract base class for handling FITS data.
 */
class FITSData {
public:
    /**
     * @brief Virtual destructor for FITSData.
     */
    virtual ~FITSData() = default;

    /**
     * @brief Pure virtual function to read data from a file.
     * @param file The input file stream to read data from.
     * @param dataSize The size of the data to read.
     */
    virtual void readData(std::ifstream& file, int64_t dataSize) = 0;

    /**
     * @brief Pure virtual function to write data to a file.
     * @param file The output file stream to write data to.
     */
    virtual void writeData(std::ofstream& file) const = 0;

    /**
     * @brief Pure virtual function to get the data type.
     * @return The data type of the FITS data.
     */
    virtual DataType getDataType() const = 0;

    /**
     * @brief Pure virtual function to get the number of elements in the data.
     * @return The number of elements in the data.
     */
    virtual size_t getElementCount() const = 0;
};

/**
 * @class TypedFITSData
 * @brief Template class for handling typed FITS data.
 * @tparam T The data type of the elements.
 */
template <typename T>
class TypedFITSData : public FITSData {
public:
    /**
     * @brief Reads data from a file.
     * @param file The input file stream to read data from.
     * @param dataSize The size of the data to read.
     */
    void readData(std::ifstream& file, int64_t dataSize) override;

    /**
     * @brief Writes data to a file.
     * @param file The output file stream to write data to.
     */
    void writeData(std::ofstream& file) const override;

    /**
     * @brief Gets the data type.
     * @return The data type of the FITS data.
     */
    DataType getDataType() const override;

    /**
     * @brief Gets the number of elements in the data.
     * @return The number of elements in the data.
     */
    size_t getElementCount() const override;

    /**
     * @brief Gets the data as a constant reference.
     * @return A constant reference to the data vector.
     */
    const std::vector<T>& getData() const { return data; }

    /**
     * @brief Gets the data as a reference.
     * @return A reference to the data vector.
     */
    std::vector<T>& getData() { return data; }

private:
    std::vector<T> data;  ///< The data vector.

    /**
     * @brief Swaps the endianness of a value.
     * @tparam U The type of the value.
     * @param value The value to swap endianness.
     */
    template <typename U>
    static void swapEndian(U& value);
};

#endif