#ifndef ATOM_IMAGE_FITS_FILE_HPP
#define ATOM_IMAGE_FITS_FILE_HPP

#include <memory>
#include <string>
#include <vector>

#include "hdu.hpp"

/**
 * @class FITSFile
 * @brief Class for handling FITS files.
 */
class FITSFile {
public:
    /**
     * @brief Reads a FITS file from the specified filename.
     * @param filename The name of the file to read.
     */
    void readFITS(const std::string& filename);

    /**
     * @brief Writes the FITS file to the specified filename.
     * @param filename The name of the file to write.
     */
    void writeFITS(const std::string& filename) const;

    /**
     * @brief Gets the number of HDUs (Header Data Units) in the FITS file.
     * @return The number of HDUs.
     */
    size_t getHDUCount() const;

    /**
     * @brief Gets a constant reference to the HDU at the specified index.
     * @param index The index of the HDU to retrieve.
     * @return A constant reference to the HDU.
     */
    const HDU& getHDU(size_t index) const;

    /**
     * @brief Gets a reference to the HDU at the specified index.
     * @param index The index of the HDU to retrieve.
     * @return A reference to the HDU.
     */
    HDU& getHDU(size_t index);

    /**
     * @brief Adds an HDU to the FITS file.
     * @param hdu A unique pointer to the HDU to add.
     */
    void addHDU(std::unique_ptr<HDU> hdu);

private:
    std::vector<std::unique_ptr<HDU>>
        hdus;  ///< Vector of unique pointers to HDUs.
};

#endif
