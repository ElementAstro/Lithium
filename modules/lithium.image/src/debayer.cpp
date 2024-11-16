#include "debayer.hpp"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <filesystem>
#include <map>
#include <tuple>

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"

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
cv::Mat readFits(const std::filesystem::path& filepath,
                 std::map<std::string, std::string>& header) {
    // Placeholder implementation. Replace with actual FITS reading logic.
    LOG_F(INFO, "Reading FITS file: {}", filepath.string());

    // Simulate reading FITS header
    header["BayerPattern"] = "RGGB";  // Example Bayer pattern

    // Simulate image data loading
    cv::Mat img =
        cv::imread("path_to_a_simulated_fits_image_in_png_or_other_format",
                   cv::IMREAD_UNCHANGED);
    if (img.empty()) {
        LOG_F(ERROR,
              "Failed to read simulated FITS image from placeholder path.");
        THROW_INVALID_ARGUMENT("Failed to read simulated FITS image.");
    }

    LOG_F(INFO, "FITS file read successfully.");
    return img;
}

/**
 * @brief Implements the Debayering process.
 *
 * This function reads an image file, determines if it's a FITS file or a
 * regular image, applies the appropriate Debayering technique based on the
 * Bayer pattern, and returns the processed image along with processing status
 * and header information.
 *
 * @param filepath The path to the image file.
 * @return A tuple containing the debayered image, a boolean indicating whether
 * to continue processing, and a map containing header information.
 */
std::tuple<cv::Mat, bool, std::map<std::string, std::string>> Debayer(
    const std::filesystem::path& filepath) {
    cv::Mat img, fits_img;
    bool continue_process = true;
    std::map<std::string, std::string> header;

    LOG_F(INFO, "Starting Debayer process for file: {}", filepath.string());

    try {
        std::string fileExtension = filepath.extension().string();
        std::transform(fileExtension.begin(), fileExtension.end(),
                       fileExtension.begin(), ::tolower);
        LOG_F(INFO, "File extension detected: {}", fileExtension);

        if (fileExtension == ".fits" || fileExtension == ".fit") {
            LOG_F(INFO, "Detected FITS file. Processing as FITS.");

            img = readFits(filepath, header);

            if (header.find("BayerPattern") == header.end()) {
                LOG_F(ERROR, "BayerPattern not found in FITS header.");
                THROW_INVALID_ARGUMENT(
                    "BayerPattern not found in FITS header.");
            }

            std::string bayerPat = header["BayerPattern"];
            LOG_F(INFO, "Bayer Pattern from header: {}", bayerPat);

            if (bayerPat == "RGGB") {
                cv::cvtColor(img, fits_img, cv::COLOR_BayerRG2BGR);
                LOG_F(INFO,
                      "Applied COLOR_BayerRG2BGR for Bayer pattern RGGB.");
            } else if (bayerPat == "GBRG") {
                cv::cvtColor(img, fits_img, cv::COLOR_BayerGB2BGR);
                LOG_F(INFO,
                      "Applied COLOR_BayerGB2BGR for Bayer pattern GBRG.");
            } else if (bayerPat == "BGGR") {
                cv::cvtColor(img, fits_img, cv::COLOR_BayerBG2BGR);
                LOG_F(INFO,
                      "Applied COLOR_BayerBG2BGR for Bayer pattern BGGR.");
            } else if (bayerPat == "GRBG") {
                cv::cvtColor(img, fits_img, cv::COLOR_BayerGR2BGR);
                LOG_F(INFO,
                      "Applied COLOR_BayerGR2BGR for Bayer pattern GRBG.");
            } else {
                LOG_F(WARNING,
                      "Unknown Bayer pattern: {}. Using default "
                      "COLOR_BayerBG2BGR.",
                      bayerPat);
                cv::cvtColor(img, fits_img, cv::COLOR_BayerBG2BGR);
                continue_process = false;
            }
        } else {
            LOG_F(INFO, "Detected non-FITS file. Processing as regular image.");

            img = cv::imread(filepath.string(), cv::IMREAD_UNCHANGED);
            if (img.empty()) {
                LOG_F(ERROR, "Failed to read image from path: {}",
                      filepath.string());
                THROW_INVALID_ARGUMENT("Failed to read image file.");
            }

            LOG_F(INFO, "Image loaded successfully. Channels: {}",
                  img.channels());

            if (img.channels() == 1) {
                LOG_F(INFO,
                      "Single channel image detected. No Debayering needed.");
                fits_img = img;
            } else if (img.channels() == 3 || img.channels() == 4) {
                LOG_F(INFO,
                      "Multi-channel image detected. Attempting Debayering.");

                // Attempt automatic Debayering by detecting Bayer pattern if
                // possible For demonstration, assume RGGB
                std::string assumedBayerPat = "RGGB";
                LOG_F(INFO, "Assuming Bayer pattern: {}", assumedBayerPat);

                if (assumedBayerPat == "RGGB") {
                    cv::cvtColor(img, fits_img, cv::COLOR_BayerRG2BGR);
                    LOG_F(INFO,
                          "Applied COLOR_BayerRG2BGR for assumed Bayer pattern "
                          "RGGB.");
                } else {
                    LOG_F(WARNING,
                          "Unknown or unsupported Bayer pattern. Skipping "
                          "Debayering.");
                    fits_img = img;
                    continue_process = false;
                }
            } else {
                LOG_F(ERROR, "Unsupported number of channels: {}",
                      img.channels());
                THROW_INVALID_ARGUMENT("Unsupported number of image channels.");
            }
        }

        LOG_F(INFO, "Debayer process completed successfully.");
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception during Debayer process: {}", e.what());
        throw;  // Re-throw the exception after logging
    }

    return {fits_img, continue_process, header};
}