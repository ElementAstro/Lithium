#include "thumbhash.hpp"

#include <cmath>
#include <iomanip>
#include <sstream>

const double RGB_MAX = 255.0;
const double Y_COEFF_R = 0.299;
const double Y_COEFF_G = 0.587;
const double Y_COEFF_B = 0.114;
const double CB_COEFF_R = -0.168736;
const double CB_COEFF_G = -0.331264;
const double CB_COEFF_B = 0.5;
const double CR_COEFF_R = 0.5;
const double CR_COEFF_G = -0.418688;
const double CR_COEFF_B = -0.081312;
const int THUMB_SIZE = 32;
const int DCT_SIZE = 6;

// 实现DCT（离散余弦变换）
void DCT(const cv::Mat& input, cv::Mat& output) {
    int numRows = input.rows;
    output = cv::Mat::zeros(input.size(), CV_64F);

    for (int rowIdx = 0; rowIdx < numRows; ++rowIdx) {
        for (int colIdx = 0; colIdx < numRows; ++colIdx) {
            double sum = 0.0;
            for (int xIdx = 0; xIdx < numRows; ++xIdx) {
                for (int yIdx = 0; yIdx < numRows; ++yIdx) {
                    sum += input.at<double>(xIdx, yIdx) *
                           cos((2 * xIdx + 1) * rowIdx * M_PI / (2 * numRows)) *
                           cos((2 * yIdx + 1) * colIdx * M_PI / (2 * numRows));
                }
            }
            double alphaRow =
                (rowIdx == 0) ? sqrt(1.0 / numRows) : sqrt(2.0 / numRows);
            double alphaCol =
                (colIdx == 0) ? sqrt(1.0 / numRows) : sqrt(2.0 / numRows);
            output.at<double>(rowIdx, colIdx) = alphaRow * alphaCol * sum;
        }
    }
}

// RGB到YCbCr的颜色转换
void RGBToYCbCr(const cv::Vec3b& rgb, double& yChannel, double& cbChannel,
                double& crChannel) {
    double red = rgb[2] / RGB_MAX;
    double green = rgb[1] / RGB_MAX;
    double blue = rgb[0] / RGB_MAX;

    yChannel = Y_COEFF_R * red + Y_COEFF_G * green + Y_COEFF_B * blue;
    cbChannel = CB_COEFF_R * red + CB_COEFF_G * green + CB_COEFF_B * blue;
    crChannel = CR_COEFF_R * red + CR_COEFF_G * green + CR_COEFF_B * blue;
}

// 实现 ThumbHash 编码
auto encodeThumbHash(const cv::Mat& image) -> std::vector<double> {
    int width = THUMB_SIZE;
    int height = THUMB_SIZE;

    cv::Mat yChannel = cv::Mat::zeros(height, width, CV_64F);
    cv::Mat cbChannel = cv::Mat::zeros(height, width, CV_64F);
    cv::Mat crChannel = cv::Mat::zeros(height, width, CV_64F);

    for (int rowIdx = 0; rowIdx < height; ++rowIdx) {
        for (int colIdx = 0; colIdx < width; ++colIdx) {
            const cv::Vec3b& rgb = image.at<cv::Vec3b>(rowIdx, colIdx);
            double y, cb, cr;
            RGBToYCbCr(rgb, y, cb, cr);
            yChannel.at<double>(rowIdx, colIdx) = y;
            cbChannel.at<double>(rowIdx, colIdx) = cb;
            crChannel.at<double>(rowIdx, colIdx) = cr;
        }
    }

    cv::Mat dctY, dctCb, dctCr;
    DCT(yChannel, dctY);
    DCT(cbChannel, dctCb);
    DCT(crChannel, dctCr);

    std::vector<double> thumbHash;
    for (int i = 0; i < DCT_SIZE; ++i) {
        for (int j = 0; j < DCT_SIZE; ++j) {
            thumbHash.push_back(dctY.at<double>(i, j));
            thumbHash.push_back(dctCb.at<double>(i, j));
            thumbHash.push_back(dctCr.at<double>(i, j));
        }
    }

    return thumbHash;
}

// 实现 ThumbHash 解码
auto decodeThumbHash(const std::vector<double>& thumbHash, int width,
                     int height) -> cv::Mat {
    cv::Mat yChannel = cv::Mat::zeros(height, width, CV_64F);
    cv::Mat cbChannel = cv::Mat::zeros(height, width, CV_64F);
    cv::Mat crChannel = cv::Mat::zeros(height, width, CV_64F);

    int index = 0;
    for (int i = 0; i < DCT_SIZE; ++i) {
        for (int j = 0; j < DCT_SIZE; ++j) {
            yChannel.at<double>(i, j) = thumbHash[index++];
            cbChannel.at<double>(i, j) = thumbHash[index++];
            crChannel.at<double>(i, j) = thumbHash[index++];
        }
    }

    cv::Mat idctY, idctCb, idctCr;
    DCT(yChannel, idctY);
    DCT(cbChannel, idctCb);
    DCT(crChannel, idctCr);

    cv::Mat decodedImage(height, width, CV_8UC3);
    for (int rowIdx = 0; rowIdx < height; ++rowIdx) {
        for (int colIdx = 0; colIdx < width; ++colIdx) {
            double y = idctY.at<double>(rowIdx, colIdx);
            double cb = idctCb.at<double>(rowIdx, colIdx);
            double cr = idctCr.at<double>(rowIdx, colIdx);

            int red = std::min(std::max(int(y + 1.402 * cr), 0), 255);
            int green = std::min(
                std::max(int(y - 0.344136 * cb - 0.714136 * cr), 0), 255);
            int blue = std::min(std::max(int(y + 1.772 * cb), 0), 255);

            decodedImage.at<cv::Vec3b>(rowIdx, colIdx) =
                cv::Vec3b(blue, green, red);
        }
    }

    return decodedImage;
}

// 将 ThumbHash 数据编码为 Base64
auto base64Encode(const std::vector<double>& thumbHash) -> std::string {
    std::stringstream stringStream;
    for (const auto& value : thumbHash) {
        stringStream << std::fixed << std::setprecision(2) << value;
    }
    return stringStream.str();  // 这里为了简化，实际上应该用 base64 编码工具
}
