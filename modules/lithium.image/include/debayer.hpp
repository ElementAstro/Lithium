#ifndef DEBAYER_HPP
#define DEBAYER_HPP

#include <filesystem>
#include <map>
#include <opencv2/core.hpp>
#include <string>

#include "atom/macro.hpp"

/**
 * @brief Struct to hold the result of the Debayering process.
 */
struct DebayerResult {
    cv::Mat debayeredImage;   ///< The debayered image.
    bool continueProcessing;  ///< Flag indicating whether to continue
                              ///< processing.
    std::map<std::string, std::string> header;  ///< FITS header information.
} ATOM_ALIGNAS(128);

/**
 * @brief Reads a FITS file and converts it to a cv::Mat.
 *
 * This is a placeholder function. For actual FITS file reading, consider using
 * a library like CFITSIO.
 *
 * @param filepath The path to the FITS file.
 * @param header A map to store FITS header information.
 * @return cv::Mat The image data extracted from the FITS file.
 */
auto readFits(const std::filesystem::path& filepath,
              std::map<std::string, std::string>& header) -> cv::Mat;

/**
 * @brief Implements the Debayering process.
 *
 * This function reads an image file, determines if it's a FITS file or a
 * regular image, applies the appropriate Debayering technique based on the
 * Bayer pattern, and returns the processed image along with processing status
 * and header information.
 *
 * @param filepath The path to the image file.
 * @return DebayerResult A struct containing the debayered image, a boolean
 * indicating whether to continue processing, and a map containing header
 * information.
 */
auto debayer(const std::filesystem::path& filepath) -> DebayerResult;

#endif  // DEBAYER_HPP