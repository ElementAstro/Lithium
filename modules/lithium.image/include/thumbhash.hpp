#ifndef THUMBHASH_H
#define THUMBHASH_H

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

/**
 * @brief Performs Discrete Cosine Transform (DCT) on the input image.
 *
 * This function applies the Discrete Cosine Transform to the input image and
 * stores the result in the output matrix.
 *
 * @param input The input image matrix.
 * @param output The output matrix to store the DCT result.
 */
void DCT(const cv::Mat& input, cv::Mat& output);

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
void RGBToYCbCr(const cv::Vec3b& rgb, double& Y, double& Cb, double& Cr);

/**
 * @brief Encodes an image into a ThumbHash.
 *
 * This function extracts frequency information from the input image and encodes
 * it into a ThumbHash.
 *
 * @param image The input image to be encoded.
 * @return A vector of doubles representing the encoded ThumbHash.
 */
std::vector<double> encodeThumbHash(const cv::Mat& image);

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
cv::Mat decodeThumbHash(const std::vector<double>& thumbHash, int width,
                        int height);

/**
 * @brief Encodes ThumbHash data into a Base64 string.
 *
 * This function converts the ThumbHash data into a Base64 encoded string for
 * easy storage and transmission.
 *
 * @param thumbHash The ThumbHash data to be encoded.
 * @return A Base64 encoded string representing the ThumbHash data.
 */
std::string base64Encode(const std::vector<double>& thumbHash);

#endif  // THUMBHASH_H
