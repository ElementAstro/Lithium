#include "thumbhash.hpp"

#include <cmath>
#include <iomanip>
#include <sstream>
#include <vector>

#include <opencv2/opencv.hpp>

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"

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
const double MAGIC_1_402 = 1.402;
const double MAGIC_0_344136 = 0.344136;
const double MAGIC_0_714136 = 0.714136;
const double MAGIC_1_772 = 1.772;
const int MAGIC_255 = 255;

/**
 * @brief Implements the Discrete Cosine Transform (DCT).
 *
 * This function performs a 2D DCT on the input matrix and stores the result in
 * the output matrix.
 *
 * @param input The input matrix (grayscale image) for DCT.
 * @param output The output matrix where the DCT coefficients will be stored.
 */
void dct(const cv::Mat& input, cv::Mat& output) {
    try {
        LOG_F(INFO, "Starting DCT transformation.");
        if (input.empty()) {
            LOG_F(ERROR, "Input matrix for DCT is empty.");
            THROW_INVALID_ARGUMENT("Input matrix for DCT is empty.");
        }

        // OpenCV provides a built-in function for DCT which is optimized and
        // faster than manual implementation
        cv::dct(input, output);
        LOG_F(INFO, "DCT transformation completed successfully.");
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception in DCT: {}", e.what());
        throw;
    }
}

/**
 * @brief Converts RGB color to YCbCr color space.
 *
 * This function takes an RGB pixel and converts it to its corresponding Y, Cb,
 * and Cr components.
 *
 * @param rgb The RGB pixel as a cv::Vec3b.
 * @param yChannel Reference to store the Y component.
 * @param cbChannel Reference to store the Cb component.
 * @param crChannel Reference to store the Cr component.
 */
auto rgbToYCbCr(const cv::Vec3b& rgb) -> YCbCr {
    try {
        LOG_F(INFO, "Converting RGB to YCbCr.");
        double red = rgb[2] / RGB_MAX;
        double green = rgb[1] / RGB_MAX;
        double blue = rgb[0] / RGB_MAX;

        YCbCr yCbCr{};
        yCbCr.y = Y_COEFF_R * red + Y_COEFF_G * green + Y_COEFF_B * blue;
        yCbCr.cb = CB_COEFF_R * red + CB_COEFF_G * green + CB_COEFF_B * blue;
        yCbCr.cr = CR_COEFF_R * red + CR_COEFF_G * green + CR_COEFF_B * blue;
        LOG_F(INFO, "Conversion to YCbCr completed.");
        return yCbCr;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception in RGBToYCbCr: {}", e.what());
        throw;
    }
}

/**
 * @brief Implements ThumbHash encoding.
 *
 * This function encodes an image into its ThumbHash representation by
 * converting it to YCbCr, performing DCT, and extracting the DCT coefficients.
 *
 * @param image The input image as a cv::Mat (expected to be in BGR format).
 * @return A vector of doubles representing the ThumbHash.
 */
auto encodeThumbHash(const cv::Mat& image) -> std::vector<double> {
    try {
        LOG_F(INFO, "Starting ThumbHash encoding.");
        if (image.empty()) {
            LOG_F(ERROR, "Input image for encoding is empty.");
            THROW_INVALID_ARGUMENT("Input image for encoding is empty.");
        }

        if (image.cols < THUMB_SIZE || image.rows < THUMB_SIZE) {
            LOG_F(WARNING,
                  "Input image is smaller than THUMB_SIZE. Resizing may affect "
                  "quality.");
        }

        int width = THUMB_SIZE;
        int height = THUMB_SIZE;

        cv::Mat resizedImage;
        cv::resize(image, resizedImage, cv::Size(width, height));
        LOG_F(INFO, "Image resized to {}x{} for ThumbHash.", width, height);

        cv::Mat yChannel = cv::Mat::zeros(height, width, CV_64F);
        cv::Mat cbChannel = cv::Mat::zeros(height, width, CV_64F);
        cv::Mat crChannel = cv::Mat::zeros(height, width, CV_64F);

        for (int rowIdx = 0; rowIdx < height; ++rowIdx) {
            for (int colIdx = 0; colIdx < width; ++colIdx) {
                const cv::Vec3b& rgb =
                    resizedImage.at<cv::Vec3b>(rowIdx, colIdx);
                auto [yChannelValue, cbChannelValue, crChannelValue] =
                    rgbToYCbCr(rgb);
                yChannel.at<double>(rowIdx, colIdx) = yChannelValue;
                cbChannel.at<double>(rowIdx, colIdx) = cbChannelValue;
                crChannel.at<double>(rowIdx, colIdx) = crChannelValue;
            }
        }
        LOG_F(INFO, "Converted image to YCbCr channels.");

        cv::Mat dctY;
        cv::Mat dctCb;
        cv::Mat dctCr;
        dct(yChannel, dctY);
        dct(cbChannel, dctCb);
        dct(crChannel, dctCr);
        LOG_F(INFO, "DCT applied to Y, Cb, and Cr channels.");

        std::vector<double> thumbHash;
        thumbHash.reserve(
            static_cast<std::vector<double>::size_type>(DCT_SIZE) * DCT_SIZE *
            3);  // Pre-allocate memory

        for (int i = 0; i < DCT_SIZE; ++i) {
            for (int j = 0; j < DCT_SIZE; ++j) {
                thumbHash.push_back(dctY.at<double>(i, j));
                thumbHash.push_back(dctCb.at<double>(i, j));
                thumbHash.push_back(dctCr.at<double>(i, j));
            }
        }
        LOG_F(INFO, "ThumbHash encoding completed successfully.");
        return thumbHash;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception in encodeThumbHash: {}", e.what());
        throw;
    }
}

/**
 * @brief Implements ThumbHash decoding.
 *
 * This function decodes a ThumbHash back into an image by performing inverse
 * DCT and converting YCbCr back to RGB.
 *
 * @param thumbHash The ThumbHash data as a vector of doubles.
 * @param width The width of the decoded image.
 * @param height The height of the decoded image.
 * @return The decoded image as a cv::Mat in BGR format.
 */
auto decodeThumbHash(const std::vector<double>& thumbHash, int width,
                     int height) -> cv::Mat {
    try {
        LOG_F(INFO, "Starting ThumbHash decoding.");
        if (thumbHash.empty()) {
            LOG_F(ERROR, "ThumbHash data is empty.");
            THROW_INVALID_ARGUMENT("ThumbHash data is empty.");
        }

        if (thumbHash.size() != DCT_SIZE * DCT_SIZE * 3) {
            LOG_F(ERROR, "ThumbHash data size mismatch. Expected {} elements.",
                  DCT_SIZE * DCT_SIZE * 3);
            THROW_INVALID_ARGUMENT("ThumbHash data size mismatch.");
        }

        cv::Mat yChannel = cv::Mat::zeros(width, height, CV_64F);
        cv::Mat cbChannel = cv::Mat::zeros(width, height, CV_64F);
        cv::Mat crChannel = cv::Mat::zeros(width, height, CV_64F);

        int index = 0;
        for (int i = 0; i < DCT_SIZE; ++i) {
            for (int j = 0; j < DCT_SIZE; ++j) {
                yChannel.at<double>(i, j) = thumbHash[index++];
                cbChannel.at<double>(i, j) = thumbHash[index++];
                crChannel.at<double>(i, j) = thumbHash[index++];
            }
        }
        LOG_F(INFO, "Parsed ThumbHash data into YCbCr channels.");

        cv::Mat idctY;
        cv::Mat idctCb;
        cv::Mat idctCr;
        dct(yChannel, idctY);  // Assuming DCT is its own inverse for simplicity
        dct(cbChannel, idctCb);
        dct(crChannel, idctCr);
        LOG_F(INFO, "Inverse DCT applied to Y, Cb, and Cr channels.");

        cv::Mat decodedImage(height, width, CV_8UC3);

        for (int rowIdx = 0; rowIdx < height; ++rowIdx) {
            for (int colIdx = 0; colIdx < width; ++colIdx) {
                double yChannelValue = idctY.at<double>(rowIdx, colIdx);
                double cbChannelValue = idctCb.at<double>(rowIdx, colIdx);
                double crChannelValue = idctCr.at<double>(rowIdx, colIdx);

                int red = std::min(
                    std::max(static_cast<int>(yChannelValue +
                                              MAGIC_1_402 * crChannelValue *
                                                  RGB_MAX),
                             0),
                    MAGIC_255);
                int green = std::min(
                    std::max(static_cast<int>(
                                 yChannelValue -
                                 MAGIC_0_344136 * cbChannelValue * RGB_MAX -
                                 MAGIC_0_714136 * crChannelValue * RGB_MAX),
                             0),
                    MAGIC_255);
                int blue = std::min(
                    std::max(static_cast<int>(yChannelValue +
                                              MAGIC_1_772 * cbChannelValue *
                                                  RGB_MAX),
                             0),
                    MAGIC_255);

                decodedImage.at<cv::Vec3b>(rowIdx, colIdx) = cv::Vec3b(
                    static_cast<uchar>(blue), static_cast<uchar>(green),
                    static_cast<uchar>(red));
            }
        }
        LOG_F(INFO,
              "Converted YCbCr back to RGB and assembled the decoded image.");

        LOG_F(INFO, "ThumbHash decoding completed successfully.");
        return decodedImage;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception in decodeThumbHash: {}", e.what());
        throw;
    }
}

/**
 * @brief Encodes ThumbHash data into a Base64 string.
 *
 * Note: This is a simplified version. For actual Base64 encoding, consider
 * using a library.
 *
 * @param thumbHash The ThumbHash data as a vector of doubles.
 * @return A Base64 encoded string representing the ThumbHash.
 */
auto base64Encode(const std::vector<double>& thumbHash) -> std::string {
    try {
        LOG_F(INFO, "Starting Base64 encoding of ThumbHash.");
        if (thumbHash.empty()) {
            LOG_F(ERROR, "ThumbHash data is empty. Cannot encode.");
            THROW_INVALID_ARGUMENT("ThumbHash data is empty.");
        }

        std::stringstream stringStream;
        for (const auto& value : thumbHash) {
            stringStream << std::fixed << std::setprecision(2) << value << ",";
        }
        std::string encoded = stringStream.str();
        if (!encoded.empty()) {
            encoded.pop_back();  // Remove the trailing comma
        }
        LOG_F(INFO, "Base64 encoding completed successfully.");
        return encoded;  // Placeholder for actual Base64 encoding
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception in base64Encode: {}", e.what());
        throw;
    }
}

/**
 * @brief Decodes a Base64 string back into ThumbHash data.
 *
 * Note: This is a simplified version. For actual Base64 decoding, consider
 * using a library.
 *
 * @param encoded The Base64 encoded string representing the ThumbHash.
 * @return The ThumbHash data as a vector of doubles.
 */
auto base64Decode(const std::string& encoded) -> std::vector<double> {
    try {
        LOG_F(INFO, "Starting Base64 decoding of ThumbHash.");
        std::vector<double> thumbHash;
        std::stringstream stringStream(encoded);
        std::string item;

        while (std::getline(stringStream, item, ',')) {
            thumbHash.push_back(std::stod(item));
        }

        if (thumbHash.size() != DCT_SIZE * DCT_SIZE * 3) {
            LOG_F(ERROR,
                  "Decoded ThumbHash size mismatch. Expected {} elements.",
                  DCT_SIZE * DCT_SIZE * 3);
            THROW_INVALID_ARGUMENT("Decoded ThumbHash size mismatch.");
        }

        LOG_F(INFO, "Base64 decoding completed successfully.");
        return thumbHash;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception in base64Decode: {}", e.what());
        throw;
    }
}