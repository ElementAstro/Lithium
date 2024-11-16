#include "stack.hpp"

#include <algorithm>
#include <cmath>
#include <opencv2/imgproc.hpp>
#include <unordered_map>
#include <vector>

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"

// Compute the mean and standard deviation of images
auto computeMeanAndStdDev(const std::vector<cv::Mat>& images)
    -> std::pair<cv::Mat, cv::Mat> {
    if (images.empty()) {
        LOG_F(ERROR,
              "Input images are empty when computing mean and standard "
              "deviation.");
        THROW_RUNTIME_ERROR("Input images are empty");
    }

    LOG_F(
        INFO,
        "Starting to compute mean and standard deviation. Number of images: {}",
        images.size());

    // Initialize mean and standard deviation matrices
    cv::Mat mean = cv::Mat::zeros(images[0].size(), CV_32F);
    cv::Mat accumSquare = cv::Mat::zeros(images[0].size(), CV_32F);

    // Accumulate pixel values
    for (const auto& img : images) {
        if (img.size() != mean.size() || img.type() != mean.type()) {
            LOG_F(ERROR, "All images must have the same size and type.");
            THROW_RUNTIME_ERROR("Image size or type mismatch");
        }

        cv::Mat floatImg;
        img.convertTo(floatImg, CV_32F);
        mean += floatImg;
        accumSquare += floatImg.mul(floatImg);
    }

    // Compute mean
    mean /= static_cast<float>(images.size());

    // Compute standard deviation
    cv::Mat stdDev;
    cv::sqrt(accumSquare / static_cast<float>(images.size()) - mean.mul(mean),
             stdDev);

    LOG_F(INFO, "Mean and standard deviation computation completed.");

    return {mean, stdDev};
}

// Sigma clipping stack
auto sigmaClippingStack(const std::vector<cv::Mat>& images,
                        float sigma) -> cv::Mat {
    if (images.empty()) {
        LOG_F(ERROR, "No input images for sigma clipping stack.");
        THROW_RUNTIME_ERROR("No images to stack");
    }

    LOG_F(INFO, "Starting sigma clipping stack. Sigma value: %.2f", sigma);

    cv::Mat mean;
    cv::Mat stdDev;
    try {
        std::tie(mean, stdDev) = computeMeanAndStdDev(images);
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Failed to compute mean and standard deviation: %s",
              e.what());
        throw;
    }

    std::vector<cv::Mat> layers;
    for (size_t i = 0; i < images.size(); ++i) {
        cv::Mat temp;
        images[i].convertTo(temp, CV_32F);
        cv::Mat mask = cv::abs(temp - mean) < (sigma * stdDev);
        temp.setTo(0, ~mask);
        layers.push_back(temp);
        LOG_F(INFO, "Processed image %zu, applied sigma clipping mask.", i + 1);
    }

    cv::Mat sum = cv::Mat::zeros(images[0].size(), CV_32F);
    cv::Mat count = cv::Mat::zeros(images[0].size(), CV_32F);

    for (size_t i = 0; i < layers.size(); ++i) {
        cv::Mat mask = layers[i] != 0;
        sum += layers[i];
        count += mask;
        LOG_F(INFO, "Accumulated layer %zu.", i + 1);
    }

    // Prevent division by zero
    cv::Mat nonZeroMask = count > 0;
    cv::Mat result = cv::Mat::zeros(images[0].size(), CV_32F);
    sum.copyTo(result, nonZeroMask);
    result /= count;

    // Convert result back to 8-bit image
    result.convertTo(result, CV_8U);

    LOG_F(INFO, "Sigma clipping stack completed.");

    return result;
}

// Compute the mode (most frequent value) of each pixel
auto computeMode(const std::vector<cv::Mat>& images) -> cv::Mat {
    if (images.empty()) {
        LOG_F(ERROR, "Input images are empty when computing mode.");
        THROW_RUNTIME_ERROR("Input images are empty");
    }

    LOG_F(INFO, "Starting to compute image mode. Number of images: {}",
          images.size());

    cv::Mat modeImage = cv::Mat::zeros(images[0].size(), images[0].type());

    for (int row = 0; row < images[0].rows; ++row) {
        for (int col = 0; col < images[0].cols; ++col) {
            std::unordered_map<uchar, int> frequency;
            for (const auto& img : images) {
                uchar pixel = img.at<uchar>(row, col);
                frequency[pixel]++;
            }

            // Find the most frequent pixel value
            int maxFreq = 0;
            uchar modePixel = 0;
            for (const auto& [pixel, freq] : frequency) {
                if (freq > maxFreq) {
                    maxFreq = freq;
                    modePixel = pixel;
                }
            }

            modeImage.at<uchar>(row, col) = modePixel;
        }
    }

    LOG_F(INFO, "Image mode computation completed.");

    return modeImage;
}

auto stackImages(const std::vector<cv::Mat>& images, StackMode mode,
                 float sigma, const std::vector<float>& weights) -> cv::Mat;

// Stack images by layers
auto stackImagesByLayers(const std::vector<cv::Mat>& images, StackMode mode,
                         float sigma,
                         const std::vector<float>& weights) -> cv::Mat {
    if (images.empty()) {
        LOG_F(ERROR, "No input images for stacking.");
        THROW_RUNTIME_ERROR("No images to stack");
    }

    LOG_F(INFO, "Starting image stacking by layers. Mode: {}", mode);

    std::vector<cv::Mat> channels;
    cv::split(images[0], channels);

    for (size_t i = 1; i < images.size(); ++i) {
        std::vector<cv::Mat> tempChannels;
        cv::split(images[i], tempChannels);
        for (size_t j = 0; j < channels.size(); ++j) {
            channels[j].push_back(tempChannels[j]);
        }
    }

    std::vector<cv::Mat> stackedChannels;
    for (auto& channel : channels) {
        stackedChannels.push_back(stackImages(channel, mode, sigma, weights));
    }

    cv::Mat stackedImage;
    cv::merge(stackedChannels, stackedImage);

    LOG_F(INFO, "Image stacking by layers completed.");

    return stackedImage;
}

// Image stacking function
auto stackImages(const std::vector<cv::Mat>& images, StackMode mode,
                 float sigma, const std::vector<float>& weights) -> cv::Mat {
    if (images.empty()) {
        LOG_F(ERROR, "No input images for stacking.");
        THROW_RUNTIME_ERROR("No images to stack");
    }

    LOG_F(INFO, "Starting image stacking. Mode: {}", mode);

    cv::Mat stackedImage;

    try {
        switch (mode) {
            case MEAN: {
                LOG_F(INFO, "Selected stacking mode: Mean stack (MEAN)");
                // Compute mean
                cv::Mat stdDev;  // Declare stdDev variable
                std::tie(stackedImage, stdDev) = computeMeanAndStdDev(images);
                stackedImage.convertTo(stackedImage, CV_8U);
                break;
            }
            case MEDIAN: {
                LOG_F(INFO, "Selected stacking mode: Median stack (MEDIAN)");
                std::vector<cv::Mat> sortedImages;
                for (const auto& img : images) {
                    cv::Mat floatImg;
                    img.convertTo(floatImg, CV_32F);
                    sortedImages.push_back(floatImg);
                }

                // Stack all images into a 4D matrix
                cv::Mat stacked4D;
                cv::merge(sortedImages, stacked4D);

                // Compute median
                cv::Mat medianImg = cv::Mat::zeros(images[0].size(), CV_32F);
                for (int row = 0; row < medianImg.rows; ++row) {
                    for (int col = 0; col < medianImg.cols; ++col) {
                        std::vector<float> pixelValues;
                        pixelValues.reserve(sortedImages.size());
                        for (const auto& sortedImg : sortedImages) {
                            pixelValues.push_back(
                                sortedImg.at<float>(row, col));
                        }
                        std::nth_element(
                            pixelValues.begin(),
                            pixelValues.begin() + static_cast<std::ptrdiff_t>(
                                                      pixelValues.size() / 2),
                            pixelValues.end());
                        medianImg.at<float>(row, col) =
                            pixelValues[pixelValues.size() / 2];
                    }
                }
                medianImg.convertTo(stackedImage, CV_8U);
                break;
            }
            case MAXIMUM: {
                LOG_F(INFO, "Selected stacking mode: Maximum stack (MAXIMUM)");
                stackedImage = images[0].clone();
                for (size_t i = 1; i < images.size(); ++i) {
                    cv::max(stackedImage, images[i], stackedImage);
                    LOG_F(INFO, "Applied maximum stack: Image %zu", i + 1);
                }
                break;
            }
            case MINIMUM: {
                LOG_F(INFO, "Selected stacking mode: Minimum stack (MINIMUM)");
                stackedImage = images[0].clone();
                for (size_t i = 1; i < images.size(); ++i) {
                    cv::min(stackedImage, images[i], stackedImage);
                    LOG_F(INFO, "Applied minimum stack: Image %zu", i + 1);
                }
                break;
            }
            case SIGMA_CLIPPING: {
                LOG_F(INFO,
                      "Selected stacking mode: Sigma clipping stack "
                      "(SIGMA_CLIPPING)");
                stackedImage = sigmaClippingStack(images, sigma);
                break;
            }
            case WEIGHTED_MEAN: {
                LOG_F(INFO,
                      "Selected stacking mode: Weighted mean stack "
                      "(WEIGHTED_MEAN)");
                if (weights.empty()) {
                    LOG_F(ERROR,
                          "Weight vector is empty for weighted mean stack.");
                    THROW_RUNTIME_ERROR("Weight vector cannot be empty");
                }
                if (weights.size() != images.size()) {
                    LOG_F(ERROR,
                          "Number of weights does not match number of images.");
                    THROW_RUNTIME_ERROR(
                        "Number of weights does not match number of images");
                }

                // Compute weighted sum
                cv::Mat weightedSum = cv::Mat::zeros(images[0].size(), CV_32F);
                float totalWeight = 0.0F;
                for (size_t i = 0; i < images.size(); ++i) {
                    cv::Mat floatImg;
                    images[i].convertTo(floatImg, CV_32F);
                    weightedSum += floatImg * weights[i];
                    totalWeight += weights[i];
                    LOG_F(INFO, "Applied weight %zu: %.2f", i + 1, weights[i]);
                }

                // Compute weighted mean
                weightedSum /= totalWeight;
                weightedSum.convertTo(stackedImage, CV_8U);
                break;
            }
            case LIGHTEN: {
                LOG_F(INFO, "Selected stacking mode: Lighten stack (LIGHTEN)");
                stackedImage = images[0].clone();
                for (size_t i = 1; i < images.size(); ++i) {
                    cv::Mat mask = images[i] > stackedImage;
                    images[i].copyTo(stackedImage, mask);
                    LOG_F(INFO, "Applied lighten stack: Image %zu", i + 1);
                }
                break;
            }
            default: {
                LOG_F(ERROR, "Unknown stacking mode: %d", mode);
                throw std::invalid_argument("Unknown stacking mode");
            }
        }

        LOG_F(INFO, "Image stacking completed.");
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception occurred during image stacking: %s", e.what());
        throw;
    }

    return stackedImage;
}