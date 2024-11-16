#ifndef THUMBHASH_H
#define THUMBHASH_H

#include <string>
#include <vector>

#include "atom/macro.hpp"

struct YCbCr {
    double y;
    double cb;
    double cr;
} ATOM_ALIGNAS(32);

namespace cv {
template <typename _Tp, int cn>
class Vec;
class Mat;
}  // namespace cv

/**
 * @brief Performs Discrete Cosine Transform (DCT) on the input image.
 *
 * This function applies the Discrete Cosine Transform to the input image and
 * stores the result in the output matrix.
 *
 * @param input The input image matrix.
 * @param output The output matrix to store the DCT result.
 */
void dct(const cv::Mat& input, cv::Mat& output);

/**
 * @brief Converts an RGB color to YCbCr color space.
 *
 * This function converts a given RGB color to its corresponding YCbCr color
 * space values.
 *
 * @param rgb The input RGB color.
 * @param Y The output luminance component.
 * @param Cb The output blue-difference chroma component.
 * @param Cr The output red-difference chroma component.
 */
auto rgbToYCbCr(const cv::Vec<unsigned char, 3>& rgb) -> YCbCr;

/**
 * @brief Encodes an image into a ThumbHash.
 *
 * This function extracts frequency information from the input image and encodes
 * it into a ThumbHash.
 *
 * @param image The input image to be encoded.
 * @return A vector of doubles representing the encoded ThumbHash.
 */
auto encodeThumbHash(const cv::Mat& image) -> std::vector<double>;

/**
 * @brief Decodes a ThumbHash into an image.
 *
 * This function generates a thumbnail image from the encoded ThumbHash data.
 *
 * @param thumbHash The encoded ThumbHash data.
 * @param width The width of the output thumbnail image.
 * @param height The height of the output thumbnail image.
 * @return The decoded thumbnail image.
 */
auto decodeThumbHash(const std::vector<double>& thumbHash, int width,
                     int height) -> cv::Mat;

/**
 * @brief Encodes ThumbHash data into a Base64 string.
 *
 * This function converts the ThumbHash data into a Base64 encoded string for
 * easy storage and transmission.
 *
 * @param thumbHash The ThumbHash data to be encoded.
 * @return A Base64 encoded string representing the ThumbHash data.
 */
auto base64Encode(const std::vector<double>& thumbHash) -> std::string;

#endif  // THUMBHASH_H
