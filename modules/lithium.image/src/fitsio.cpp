#include "fitsio.hpp"
#include "atom/log/loguru.hpp"
#include "base64.hpp"

#include <fitsio.h>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

/**
 * @brief Helper function to handle FITS errors.
 *
 * Logs the error message and throws an exception if CFITSIO returns an error
 * status.
 *
 * @param status CFITSIO status code.
 * @param errorMessage Custom error message.
 * @throws std::runtime_error Exception containing detailed error information.
 */
void checkFitsStatus(int status, const std::string& errorMessage) {
    if (status != 0) {
        fits_report_error(stderr, status);
        throw std::runtime_error(
            errorMessage + " CFITSIO error code: " + std::to_string(status));
    }
}

/**
 * @brief Reads a FITS file and converts it to cv::Mat.
 *
 * Uses the CFITSIO library to read a FITS file and automatically selects the
 * appropriate OpenCV type based on image dimensions and bit depth.
 *
 * @param filepath Path to the FITS file.
 * @param header Map to store FITS header information.
 * @return cv::Mat OpenCV matrix containing the image data.
 * @throws std::runtime_error If an error occurs during reading or the image
 * format is unsupported.
 */
auto readFitsToMat(const std::filesystem::path& filepath,
                   std::map<std::string, std::string>& header) -> cv::Mat {
    fitsfile* fptr;  // FITS file pointer
    int status = 0;  // CFITSIO status value, must be initialized to 0
    int bitpix;
    int naxis;
    std::array<long, 3> naxes = {1, 1, 1};

    LOG_F(INFO, "Opening FITS file: {}", filepath.string());

    // Open FITS file
    if (fits_open_file(&fptr, filepath.string().c_str(), READONLY, &status)) {
        checkFitsStatus(status, "Unable to open FITS file");
    }

    // Read image parameters
    if (fits_get_img_param(fptr, 3, &bitpix, &naxis, naxes.data(), &status)) {
        checkFitsStatus(status, "Unable to read FITS image parameters");
    }

    // Read FITS header information
    int nkeys;
    std::array<char, FLEN_CARD> card;
    if (fits_get_hdrspace(fptr, &nkeys, nullptr, &status)) {
        checkFitsStatus(status, "Unable to get FITS header space");
    }

    for (int i = 1; i <= nkeys; ++i) {
        if (fits_read_record(fptr, i, card.data(), &status)) {
            checkFitsStatus(status, "Unable to read FITS header record");
        }
        std::string line = card.data();
        size_t eqPos = line.find('=');
        if (eqPos != std::string::npos) {
            std::string key = line.substr(0, eqPos);
            std::string value = line.substr(eqPos + 1);
            // Clean up key and value
            key.erase(std::remove_if(key.begin(), key.end(), ::isspace),
                      key.end());
            value.erase(std::remove_if(value.begin(), value.end(), ::isspace),
                        value.end());
            // Remove quotes
            value.erase(std::remove(value.begin(), value.end(), '\''),
                        value.end());
            header[key] = value;
        }
    }

    // Determine image type
    int width = static_cast<int>(naxes[0]);
    int height = static_cast<int>(naxes[1]);
    int depth;
    if (bitpix == SHORT_IMG || bitpix == USHORT_IMG) {
        depth = CV_16U;
    } else if (bitpix == BYTE_IMG) {
        depth = CV_8U;
    } else if (bitpix == FLOAT_IMG) {
        depth = CV_32F;
    } else {
        fits_close_file(fptr, &status);
        throw std::runtime_error("Unsupported FITS bit depth");
    }

    // Prepare to read image data
    std::vector<long> fpixel(naxis, 1);  // First pixel to read
    long nelements =
        static_cast<long>(width) * height;  // Number of pixels to read

    cv::Mat image;

    // Handle 2D images
    if (naxis == 2 || (naxis == 3 && naxes[2] == 1)) {  // Grayscale image
        image = cv::Mat(height, width, CV_MAKETYPE(depth, 1));
        int datatype = (depth == CV_16U) ? TUSHORT : TBYTE;
        if (fits_read_pix(fptr, datatype, fpixel.data(), nelements, nullptr,
                          image.data, nullptr, &status)) {
            checkFitsStatus(status, "Unable to read FITS grayscale image data");
        }
    }
    // Handle 3D images (RGB)
    else if (naxis == 3 && naxes[2] == 3) {  // RGB image
        image = cv::Mat(height, width, CV_MAKETYPE(depth, 3));
        std::vector<cv::Mat> channels(3);
        for (int i = 0; i < 3; ++i) {
            channels[i] = cv::Mat(height, width, CV_MAKETYPE(depth, 1));
            fpixel[2] = i + 1;  // Set the correct channel (plane)
            if (fits_read_pix(fptr, (depth == CV_16U) ? TUSHORT : TBYTE,
                              fpixel.data(), nelements, nullptr,
                              channels[i].data, nullptr, &status)) {
                checkFitsStatus(status,
                                "Unable to read FITS image data for channel " +
                                    std::to_string(i));
            }
        }
        cv::merge(channels, image);
    } else {
        fits_close_file(fptr, &status);
        throw std::runtime_error("Unsupported FITS image format");
    }

    // Close FITS file
    if (fits_close_file(fptr, &status)) {
        checkFitsStatus(status, "Unable to close FITS file");
    }

    LOG_F(INFO, "FITS file read successfully: {}x{}, depth: {}", width, height,
          depth);
    return image;
}

/**
 * @brief Writes cv::Mat image data to a FITS file.
 *
 * Uses the CFITSIO library to write OpenCV cv::Mat image data to a FITS file,
 * automatically selecting the appropriate FITS format based on the number of
 * channels and bit depth.
 *
 * @param image Image data to write.
 * @param filepath Path to the target FITS file.
 * @throws std::runtime_error If an error occurs during writing or the image
 * format is unsupported.
 */
void writeMatToFits(const cv::Mat& image,
                    const std::filesystem::path& filepath) {
    fitsfile* fptr;
    int status = 0;
    std::array<long, 3> naxes = {image.cols, image.rows, image.channels()};
    int bitpix;

    // Determine FITS bit depth
    if (image.depth() == CV_16U) {
        bitpix = USHORT_IMG;
    } else if (image.depth() == CV_8U) {
        bitpix = BYTE_IMG;
    } else if (image.depth() == CV_32F) {
        bitpix = FLOAT_IMG;
    } else {
        throw std::runtime_error("Unsupported cv::Mat bit depth");
    }

    LOG_F(INFO, "Creating FITS file: {}", filepath.string());

    // Create FITS file
    if (fits_create_file(&fptr, filepath.string().c_str(), &status)) {
        checkFitsStatus(status, "Unable to create FITS file");
    }

    // Create image
    int naxis = (image.channels() == 1) ? 2 : 3;
    if (fits_create_img(fptr, bitpix, naxis, naxes.data(), &status)) {
        checkFitsStatus(status, "Unable to create FITS image");
    }

    // Write image data
    if (image.channels() == 1) {  // Grayscale image
        if (fits_write_img(
                fptr, (image.depth() == CV_16U) ? TUSHORT : TBYTE, 1,
                static_cast<LONGLONG>(image.cols) * image.rows,
                const_cast<void*>(static_cast<const void*>(image.data)),
                &status)) {
            checkFitsStatus(status,
                            "Unable to write FITS grayscale image data");
        }
    } else if (image.channels() == 3) {  // RGB image
        std::vector<cv::Mat> channels(3);
        cv::split(image, channels);
        for (int i = 0; i < 3; ++i) {
            std::array<long, 3> fpixel = {1, 1, i + 1};
            if (fits_write_pix(fptr,
                               (image.depth() == CV_16U) ? TUSHORT : TBYTE,
                               fpixel.data(),
                               static_cast<LONGLONG>(image.cols) * image.rows,
                               channels[i].data, &status)) {
                checkFitsStatus(status,
                                "Unable to write FITS image data for channel " +
                                    std::to_string(i));
            }
        }
    } else {
        fits_close_file(fptr, &status);
        throw std::runtime_error("Unsupported cv::Mat number of channels");
    }

    // Close FITS file
    if (fits_close_file(fptr, &status)) {
        checkFitsStatus(status, "Unable to close FITS file");
    }

    LOG_F(INFO, "FITS file written successfully: {}", filepath.string());
}

/**
 * @brief Converts cv::Mat image data to a Base64 string.
 *
 * Uses OpenCV to encode the image and then converts the encoded bytes to
 * Base64.
 *
 * @param image Image data to convert.
 * @param imgFormat Image encoding format (e.g., ".png", ".jpg").
 * @return std::string Base64 encoded string.
 * @throws std::runtime_error If an error occurs during encoding.
 */
auto matToBase64(const cv::Mat& image,
                 const std::string& imgFormat) -> std::string {
    std::vector<uchar> buf;
    LOG_F(INFO, "Starting to encode cv::Mat to image format: {}", imgFormat);
    if (!cv::imencode(imgFormat, image, buf)) {
        throw std::runtime_error("Image encoding failed");
    }
    LOG_F(INFO, "Image encoded successfully, encoded length: {}", buf.size());
    return base64Encode(buf.data(), buf.size());
}

/**
 * @brief Converts a FITS file to a Base64 string.
 *
 * Reads a FITS file and converts it to a Base64 string in the specified image
 * format.
 *
 * @param filepath Path to the FITS file.
 * @return std::string Base64 encoded string.
 * @throws std::runtime_error If an error occurs during conversion.
 */
auto fitsToBase64(const std::filesystem::path& filepath) -> std::string {
    LOG_F(INFO, "Starting to convert FITS file to Base64 string: {}",
          filepath.string());
    std::map<std::string, std::string> header;
    cv::Mat image = readFitsToMat(filepath, header);
    std::string base64Str = matToBase64(image, ".png");
    LOG_F(INFO, "FITS file converted to Base64 successfully");
    return base64Str;
}

/**
 * @brief Reads the device name (INSTRUME) from a FITS file.
 *
 * @param filepath Path to the FITS file.
 * @return std::optional<std::string> Returns the device name if present,
 * otherwise returns std::nullopt.
 * @throws std::runtime_error If an error occurs during reading.
 */
auto readFitsDeviceName(const std::filesystem::path& filepath)
    -> std::optional<std::string> {
    fitsfile* fptr;
    int status = 0;
    std::array<char, FLEN_CARD> card;
    std::optional<std::string> deviceName;

    LOG_F(INFO, "Starting to read device name from FITS file: {}",
          filepath.string());

    if (fits_open_file(&fptr, filepath.string().c_str(), READONLY, &status)) {
        checkFitsStatus(status, "Unable to open FITS file to read device name");
    }

    int nkeys;
    if (fits_get_hdrspace(fptr, &nkeys, nullptr, &status)) {
        checkFitsStatus(status,
                        "Unable to get FITS header space to read device name");
    }

    for (int i = 1; i <= nkeys; ++i) {
        if (fits_read_record(fptr, i, card.data(), &status)) {
            checkFitsStatus(
                status,
                "Unable to read FITS header record to find device name");
        }
        std::string line = card.data();
        if (line.find("INSTRUME") != std::string::npos) {
            size_t startPos = line.find('\'');
            size_t endPos = line.rfind('\'');
            if (startPos != std::string::npos && endPos != std::string::npos &&
                startPos != endPos && endPos > startPos) {
                deviceName = line.substr(startPos + 1, endPos - startPos - 1);
                LOG_F(INFO, "Found device name: {}", *deviceName);
                break;
            }
        }
    }

    if (!deviceName.has_value()) {
        LOG_F(WARNING, "Device name (INSTRUME) not found in FITS file");
    }

    if (fits_close_file(fptr, &status)) {
        checkFitsStatus(status, "Unable to close FITS file");
    }

    return deviceName;
}