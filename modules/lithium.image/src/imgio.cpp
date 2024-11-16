#include "imgio.hpp"
#include <fitsio.h>

#include <chrono>
#include <filesystem>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include "atom/log/loguru.hpp"

namespace fs = std::filesystem;

auto loadImage(const std::string& filename, int flags) -> cv::Mat {
    LOG_F(INFO, "Starting to load image '{}' with flags={}", filename, flags);

    if (!fs::exists(filename)) {
        LOG_F(ERROR, "Image file does not exist: {}", filename);
        return {};
    }

    auto start = std::chrono::high_resolution_clock::now();
    cv::Mat image = cv::imread(filename, flags);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    if (image.empty()) {
        LOG_F(ERROR, "Failed to load image: {} (Load time: {}ms)", filename,
              duration.count());
        return {};
    }

    LOG_F(INFO, "Successfully loaded image: {}", filename);
    LOG_F(INFO, "Image properties: {}x{}, {} channels, type={}, depth={}",
          image.cols, image.rows, image.channels(), image.type(),
          image.depth());
    LOG_F(INFO, "Load time: {}ms", duration.count());

    return image;
}

auto loadImages(const std::string& folder,
                const std::vector<std::string>& filenames,
                int flags) -> std::vector<std::pair<std::string, cv::Mat>> {
    LOG_F(INFO, "Starting batch image loading from folder: {}", folder);
    LOG_F(INFO, "Target files count: {}",
          filenames.empty() ? "all" : std::to_string(filenames.size()));

    if (!fs::exists(folder)) {
        LOG_F(ERROR, "Folder does not exist: {}", folder);
        return {};
    }

    std::vector<std::pair<std::string, cv::Mat>> images;
    auto startTotal = std::chrono::high_resolution_clock::now();
    int successCount = 0;
    int failCount = 0;

    if (filenames.empty()) {
        LOG_F(INFO, "Scanning directory for all image files...");
        for (const auto& entry : fs::directory_iterator(folder)) {
            if (entry.is_regular_file()) {
                std::string filepath = entry.path().string();
                auto start = std::chrono::high_resolution_clock::now();
                cv::Mat img = cv::imread(filepath, flags);
                auto end = std::chrono::high_resolution_clock::now();
                auto duration =
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                        end - start);

                if (!img.empty()) {
                    images.emplace_back(filepath, img);
                    successCount++;
                    LOG_F(INFO, "Loaded image {}: {}x{}, {} channels ({}ms)",
                          filepath, img.cols, img.rows, img.channels(),
                          duration.count());
                } else {
                    failCount++;
                    LOG_F(ERROR, "Failed to load image: {} ({}ms)", filepath,
                          duration.count());
                }
            }
        }
    } else {
        LOG_F(INFO, "Loading {} specified image files...", filenames.size());
        for (const auto& filename : filenames) {
            std::string filepath = folder + "/" + filename;
            auto start = std::chrono::high_resolution_clock::now();
            cv::Mat img = cv::imread(filepath, flags);
            auto end = std::chrono::high_resolution_clock::now();
            auto duration =
                std::chrono::duration_cast<std::chrono::milliseconds>(end -
                                                                      start);

            if (!img.empty()) {
                images.emplace_back(filepath, img);
                successCount++;
                LOG_F(INFO, "Loaded image {}: {}x{}, {} channels ({}ms)",
                      filepath, img.cols, img.rows, img.channels(),
                      duration.count());
            } else {
                failCount++;
                LOG_F(ERROR, "Failed to load image: {} ({}ms)", filepath,
                      duration.count());
            }
        }
    }

    auto endTotal = std::chrono::high_resolution_clock::now();
    auto totalDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTotal - startTotal);

    LOG_F(INFO, "Batch loading completed:");
    LOG_F(INFO, "  Success: {} images", successCount);
    LOG_F(INFO, "  Failed: {} images", failCount);
    LOG_F(INFO, "  Total time: {}ms", totalDuration.count());
    LOG_F(INFO, "  Average time per image: {}ms",
          (successCount > 0) ? totalDuration.count() / successCount : 0);

    return images;
}

auto saveImage(const std::string& filename, const cv::Mat& image) -> bool {
    LOG_F(INFO, "Starting to save image: {}", filename);
    LOG_F(INFO, "Image properties: {}x{}, {} channels, type={}", image.cols,
          image.rows, image.channels(), image.type());

    if (image.empty()) {
        LOG_F(ERROR, "Cannot save empty image: {}", filename);
        return false;
    }

    auto start = std::chrono::high_resolution_clock::now();
    bool success = cv::imwrite(filename, image);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    if (success) {
        LOG_F(INFO, "Image saved successfully: {} ({}ms)", filename,
              duration.count());
        LOG_F(INFO, "File size: {} bytes", fs::file_size(filename));
        return true;
    }

    LOG_F(ERROR, "Failed to save image: {} ({}ms)", filename, duration.count());
    return false;
}

auto saveMatTo8BitJpg(const cv::Mat& image,
                      const std::string& output_path) -> bool {
    LOG_F(INFO, "Starting 8-bit JPG conversion for image {}x{}", image.cols,
          image.rows);

    if (image.empty()) {
        LOG_F(ERROR, "Input image is empty");
        return false;
    }

    LOG_F(INFO, "Input image: type={}, depth={}, channels={}", image.type(),
          image.depth(), image.channels());

    try {
        cv::Mat image16;
        cv::Mat outputImage;

        // Convert to 16-bit based on input depth
        switch (image.depth()) {
            case CV_8U:
                LOG_F(INFO, "Converting 8-bit to 16-bit with MSB alignment");
                image.convertTo(image16, CV_16UC1, 256.0);
                break;
            case CV_16U:
                LOG_F(INFO, "Maintaining 16-bit depth");
                image.convertTo(image16, CV_16UC1);
                break;
            default:
                LOG_F(ERROR, "Unsupported image depth: {}", image.depth());
                return false;
        }

        // Normalize to 8-bit range using modern OpenCV method
        cv::normalize(image16, outputImage, 0, 255, cv::NORM_MINMAX, CV_8U);

        // Configure JPEG compression parameters
        std::vector<int> compressionParams;
        compressionParams.push_back(cv::IMWRITE_JPEG_QUALITY);
        compressionParams.push_back(95);  // High quality JPEG

        return saveImage(output_path,
                         outputImage);  // Using existing saveImage function
    } catch (const cv::Exception& e) {
        LOG_F(ERROR, "OpenCV error during 8-bit conversion: {}", e.what());
        return false;
    }
}

auto saveMatTo16BitPng(const cv::Mat& image,
                       const std::string& output_path) -> bool {
    LOG_F(INFO, "Starting 16-bit PNG conversion for image {}x{}", image.cols,
          image.rows);

    if (image.empty()) {
        LOG_F(ERROR, "Input image is empty");
        return false;
    }

    try {
        cv::Mat outputImage;

        // Optimal 16-bit conversion
        if (image.depth() == CV_8U) {
            LOG_F(INFO, "Converting 8-bit to 16-bit");
            image.convertTo(outputImage, CV_16U, 256.0);
        } else if (image.depth() == CV_16U) {
            outputImage = image.clone();
        } else {
            LOG_F(ERROR, "Unsupported image depth: {}", image.depth());
            return false;
        }

        // Configure PNG compression parameters
        std::vector<int> compressionParams;
        compressionParams.push_back(cv::IMWRITE_PNG_COMPRESSION);
        compressionParams.push_back(9);  // Maximum compression

        return saveImage(output_path,
                         outputImage);  // Using existing saveImage function
    } catch (const cv::Exception& e) {
        LOG_F(ERROR, "OpenCV error during 16-bit conversion: {}", e.what());
        return false;
    }
}

auto saveMatToFits(const cv::Mat& image,
                   const std::string& output_path) -> bool {
    LOG_F(INFO, "Starting FITS conversion for image {}x{}", image.cols,
          image.rows);

    if (image.empty()) {
        LOG_F(ERROR, "Input image is empty");
        return false;
    }

    try {
        // Ensure grayscale
        cv::Mat grayImage;
        if (image.channels() == 3) {
            cv::cvtColor(image, grayImage, cv::COLOR_BGR2GRAY);
        } else {
            grayImage = image.clone();
        }

        // FITS specific parameters
        const long NAXES[2] = {grayImage.cols, grayImage.rows};
        int status = 0;
        fitsfile* fptr = nullptr;

        // Create FITS file with error handling
        const std::string FITS_PATH = "!" + output_path;  // Force overwrite
        if (auto ret = fits_create_file(&fptr, FITS_PATH.c_str(), &status);
            ret != 0) {
            LOG_F(ERROR, "Failed to create FITS file: {}", output_path);
            return false;
        }

        // Create image
        if (auto ret = fits_create_img(fptr, SHORT_IMG, 2,
                                       const_cast<long*>(NAXES), &status);
            ret != 0) {
            LOG_F(ERROR, "Failed to create FITS image structure");
            fits_close_file(fptr, &status);
            return false;
        }

        // Write data
        if (auto ret = fits_write_img(fptr, TSHORT, 1, grayImage.total(),
                                      grayImage.ptr<short>(), &status);
            ret != 0) {
            LOG_F(ERROR, "Failed to write FITS image data");
            fits_close_file(fptr, &status);
            return false;
        }

        // Close file
        fits_close_file(fptr, &status);

        if (status != 0) {
            char error_msg[80];
            fits_get_errstatus(status, error_msg);
            LOG_F(ERROR, "FITS error: {}", error_msg);
            return false;
        }

        LOG_F(INFO, "Successfully saved FITS file: {}", output_path);
        return true;

    } catch (const std::exception& e) {
        LOG_F(ERROR, "Error during FITS conversion: {}", e.what());
        return false;
    }
}
