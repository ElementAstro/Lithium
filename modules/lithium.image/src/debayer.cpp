#include "debayer.hpp"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <map>
#include <stdexcept>
#include <string>
#include <tuple>

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"

auto debayer(const std::filesystem::path& filepath) -> DebayerResult {
    DebayerResult result;
    result.continueProcessing = true;
    result.header = {};

    cv::Mat img;
    cv::Mat debayeredImg;

    LOG_F(INFO, "Starting Debayer process for file: {}", filepath.string());

    try {
        std::string fileExtension = filepath.extension().string();
        // Convert file extension to lower case
        std::transform(fileExtension.begin(), fileExtension.end(),
                       fileExtension.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        LOG_F(INFO, "File extension detected: {}", fileExtension);

        if (fileExtension == ".fits" || fileExtension == ".fit") {
            LOG_F(INFO, "Detected FITS file. Processing as FITS.");

            img = readFits(filepath, result.header);

            if (result.header.find("BayerPattern") == result.header.end()) {
                LOG_F(ERROR, "BayerPattern not found in FITS header.");
                THROW_INVALID_ARGUMENT(
                    "BayerPattern not found in FITS header.");
            }

            std::string bayerPat = result.header["BayerPattern"];
            LOG_F(INFO, "Bayer Pattern from header: {}", bayerPat);

            // Convert Bayer pattern to uppercase for consistency
            std::transform(bayerPat.begin(), bayerPat.end(), bayerPat.begin(),
                           [](unsigned char c) { return std::toupper(c); });

            if (bayerPat == "RGGB") {
                cv::cvtColor(img, debayeredImg, cv::COLOR_BayerRG2BGR);
                LOG_F(INFO,
                      "Applied COLOR_BayerRG2BGR for Bayer pattern RGGB.");
            } else if (bayerPat == "GBRG") {
                cv::cvtColor(img, debayeredImg, cv::COLOR_BayerGB2BGR);
                LOG_F(INFO,
                      "Applied COLOR_BayerGB2BGR for Bayer pattern GBRG.");
            } else if (bayerPat == "BGGR") {
                cv::cvtColor(img, debayeredImg, cv::COLOR_BayerBG2BGR);
                LOG_F(INFO,
                      "Applied COLOR_BayerBG2BGR for Bayer pattern BGGR.");
            } else if (bayerPat == "GRBG") {
                cv::cvtColor(img, debayeredImg, cv::COLOR_BayerGR2BGR);
                LOG_F(INFO,
                      "Applied COLOR_BayerGR2BGR for Bayer pattern GRBG.");
            } else {
                LOG_F(WARNING,
                      "Unknown Bayer pattern: {}. Using default "
                      "COLOR_BayerBG2BGR.",
                      bayerPat);
                cv::cvtColor(img, debayeredImg, cv::COLOR_BayerBG2BGR);
                result.continueProcessing = false;
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
                debayeredImg = img;
            } else if (img.channels() == 3 || img.channels() == 4) {
                LOG_F(INFO,
                      "Multi-channel image detected. Attempting Debayering.");

                // Attempt automatic Debayering by detecting Bayer pattern if
                // possible For demonstration, assume RGGB
                std::string assumedBayerPat = "RGGB";
                LOG_F(INFO, "Assuming Bayer pattern: {}", assumedBayerPat);

                // Convert Bayer pattern to uppercase for consistency
                std::transform(assumedBayerPat.begin(), assumedBayerPat.end(),
                               assumedBayerPat.begin(),
                               [](unsigned char c) { return std::toupper(c); });

                if (assumedBayerPat == "RGGB") {
                    cv::cvtColor(img, debayeredImg, cv::COLOR_BayerRG2BGR);
                    LOG_F(INFO,
                          "Applied COLOR_BayerRG2BGR for assumed Bayer pattern "
                          "RGGB.");
                } else if (assumedBayerPat == "GBRG") {
                    cv::cvtColor(img, debayeredImg, cv::COLOR_BayerGB2BGR);
                    LOG_F(INFO,
                          "Applied COLOR_BayerGB2BGR for assumed Bayer pattern "
                          "GBRG.");
                } else if (assumedBayerPat == "BGGR") {
                    cv::cvtColor(img, debayeredImg, cv::COLOR_BayerBG2BGR);
                    LOG_F(INFO,
                          "Applied COLOR_BayerBG2BGR for assumed Bayer pattern "
                          "BGGR.");
                } else if (assumedBayerPat == "GRBG") {
                    cv::cvtColor(img, debayeredImg, cv::COLOR_BayerGR2BGR);
                    LOG_F(INFO,
                          "Applied COLOR_BayerGR2BGR for assumed Bayer pattern "
                          "GRBG.");
                } else {
                    LOG_F(WARNING,
                          "Unknown or unsupported Bayer pattern. Skipping "
                          "Debayering.");
                    debayeredImg = img;
                    result.continueProcessing = false;
                }
            } else {
                LOG_F(ERROR, "Unsupported number of channels: {}",
                      img.channels());
                THROW_INVALID_ARGUMENT("Unsupported number of image channels.");
            }
        }

        LOG_F(INFO, "Debayer process completed successfully.");
        result.debayeredImage = debayeredImg;

    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception during Debayer process for file {}: {}",
              filepath.string(), e.what());
        THROW_RUNTIME_ERROR("");  // Re-throw the exception after logging
    }

    return result;
}