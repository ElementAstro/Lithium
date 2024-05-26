#include "stack.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>
#include <numeric>
#include <opencv2/imgproc.hpp>

std::pair<cv::Mat, cv::Mat> computeMeanAndStdDev(
    const std::vector<cv::Mat>& images) {
    cv::Mat mean, stdDev;
    std::vector<cv::Mat> temp(images.size());

    for (size_t i = 0; i < images.size(); ++i) {
        images[i].convertTo(temp[i], CV_32F);
    }

    cv::merge(temp, mean);
    cv::meanStdDev(mean, mean, stdDev);

    return {mean, stdDev};
}

// Sigma剪裁叠加
cv::Mat sigmaClippingStack(const std::vector<cv::Mat>& images,
                           float sigma = 2.0) {
    if (images.empty()) {
        std::cerr << "Error: No images to stack." << std::endl;
        return cv::Mat();
    }

    cv::Mat mean, stdDev;
    std::tie(mean, stdDev) = computeMeanAndStdDev(images);

    std::vector<cv::Mat> layers;
    for (const auto& img : images) {
        cv::Mat temp;
        img.convertTo(temp, CV_32F);
        cv::Mat mask = cv::abs(temp - mean) < sigma * stdDev;
        temp.setTo(0, ~mask);
        layers.push_back(temp);
    }

    cv::Mat sum = cv::Mat::zeros(images[0].size(), CV_32F);
    cv::Mat count = cv::Mat::zeros(images[0].size(), CV_32F);

    for (const auto& layer : layers) {
        cv::Mat mask = layer != 0;
        sum += layer;
        count += mask;
    }

    cv::Mat result = sum / count;
    result.convertTo(result, CV_8U);
    return result;
}

// 图像叠加函数
cv::Mat stackImages(const std::vector<cv::Mat>& images, StackMode mode,
                    float sigma) {
    if (images.empty()) {
        std::cerr << "Error: No images to stack." << std::endl;
        return cv::Mat();
    }

    cv::Mat stackedImage;

    switch (mode) {
        case MEAN: {
            images[0].convertTo(stackedImage, CV_32F);
            for (size_t i = 1; i < images.size(); ++i) {
                cv::Mat temp;
                images[i].convertTo(temp, CV_32F);
                stackedImage += temp;
            }
            stackedImage /= static_cast<float>(images.size());
            stackedImage.convertTo(stackedImage, CV_8U);
            break;
        }
        case MEDIAN: {
            std::vector<cv::Mat> layers;
            for (const auto& img : images) {
                cv::Mat temp;
                img.convertTo(temp, CV_32F);
                layers.push_back(temp);
            }

            cv::merge(layers, stackedImage);
            cv::sort(stackedImage, stackedImage, cv::SORT_EVERY_COLUMN);

            std::vector<cv::Mat> channels;
            cv::split(stackedImage, channels);

            for (auto& channel : channels) {
                channel = channel.row(channel.rows / 2);
            }

            cv::merge(channels, stackedImage);
            stackedImage.convertTo(stackedImage, CV_8U);
            break;
        }
        case MAXIMUM: {
            stackedImage = images[0].clone();
            for (size_t i = 1; i < images.size(); ++i) {
                stackedImage = cv::max(stackedImage, images[i]);
            }
            break;
        }
        case MINIMUM: {
            stackedImage = images[0].clone();
            for (size_t i = 1; i < images.size(); ++i) {
                stackedImage = cv::min(stackedImage, images[i]);
            }
            break;
        }
        case SIGMA_CLIPPING: {
            stackedImage = sigmaClippingStack(images, sigma);
            break;
        }
        case WEIGHTED_MEAN: {
            std::vector<float> weights = {0.1, 0.2, 0.3, 0.4};  // 自定义权重
            float totalWeight =
                std::accumulate(weights.begin(), weights.end(), 0.0f);

            images[0].convertTo(stackedImage, CV_32F);
            stackedImage *= weights[0];

            for (size_t i = 1; i < images.size(); ++i) {
                cv::Mat temp;
                images[i].convertTo(temp, CV_32F);
                temp *= weights[i];
                stackedImage += temp;
            }

            stackedImage /= totalWeight;
            stackedImage.convertTo(stackedImage, CV_8U);
            break;
        }
        case LIGHTEN: {
            stackedImage = images[0].clone();
            for (size_t i = 1; i < images.size(); ++i) {
                cv::Mat mask = stackedImage < images[i];
                stackedImage.setTo(images[i], mask);
            }
            break;
        }
        default: {
            std::cerr << "Error: Unknown stacking mode." << std::endl;
            return cv::Mat();
        }
    }

    return stackedImage;
}
