/*
 * convolve.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-10

Description: Implementation of one-dimensional and two-dimensional convolution
and deconvolution.

**************************************************/

#ifndef ATOM_ALGORITHM_CONVOLVE_HPP
#define ATOM_ALGORITHM_CONVOLVE_HPP

#include <complex>
#include <vector>

namespace atom::algorithm {
/**
 * @brief Performs 1D convolution operation.
 *
 * This function convolves the input signal with the given kernel.
 *
 * @param input The input signal.
 * @param kernel The convolution kernel.
 * @return The convolved signal.
 */
[[nodiscard("The result of convolve is not used.")]] std::vector<double>
convolve(const std::vector<double> &input, const std::vector<double> &kernel);

/**
 * @brief Performs 1D deconvolution operation.
 *
 * This function deconvolves the input signal with the given kernel.
 *
 * @param input The input signal.
 * @param kernel The deconvolution kernel.
 * @return The deconvolved signal.
 */
[[nodiscard("The result of deconvolve is not used.")]] std::vector<double>
deconvolve(const std::vector<double> &input, const std::vector<double> &kernel);

/**
 * @brief Performs 2D convolution operation.
 *
 * This function convolves the input image with the given kernel.
 *
 * @param input The input image.
 * @param kernel The convolution kernel.
 * @param numThreads Number of threads for parallel execution (default: 1).
 * @return The convolved image.
 */
[[nodiscard(
    "The result of convolve2D is not used.")]] std::vector<std::vector<double>>
convolve2D(const std::vector<std::vector<double>> &input,
           const std::vector<std::vector<double>> &kernel, int numThreads = 1);

/**
 * @brief Performs 2D deconvolution operation.
 *
 * This function deconvolves the input image with the given kernel.
 *
 * @param signal The input image.
 * @param kernel The deconvolution kernel.
 * @param numThreads Number of threads for parallel execution (default: 1).
 * @return The deconvolved image.
 */
[[nodiscard("The result of deconvolve2D is not used.")]] std::vector<
    std::vector<double>>
deconvolve2D(const std::vector<std::vector<double>> &signal,
             const std::vector<std::vector<double>> &kernel,
             int numThreads = 1);

/**
 * @brief Performs 2D Discrete Fourier Transform (DFT).
 *
 * This function computes the 2D DFT of the input image.
 *
 * @param signal The input image.
 * @param numThreads Number of threads for parallel execution (default: 1).
 * @return The 2D DFT spectrum.
 */
[[nodiscard("The result of DFT2D is not used.")]] std::vector<
    std::vector<std::complex<double>>>
DFT2D(const std::vector<std::vector<double>> &signal, int numThreads = 1);

/**
 * @brief Performs 2D Inverse Discrete Fourier Transform (IDFT).
 *
 * This function computes the 2D IDFT of the input spectrum.
 *
 * @param spectrum The input spectrum.
 * @param numThreads Number of threads for parallel execution (default: 1).
 * @return The 2D IDFT image.
 */
[[nodiscard(
    "The result of IDFT2D is not used.")]] std::vector<std::vector<double>>
IDFT2D(const std::vector<std::vector<std::complex<double>>> &spectrum,
       int numThreads = 1);

/**
 * @brief Generates a Gaussian kernel for 2D convolution.
 *
 * This function generates a Gaussian kernel for 2D convolution.
 *
 * @param size The size of the kernel.
 * @param sigma The standard deviation of the Gaussian distribution.
 * @return The generated Gaussian kernel.
 */
[[nodiscard("The result of generateGaussianKernel is not used.")]] std::vector<
    std::vector<double>>
generateGaussianKernel(int size, double sigma);

/**
 * @brief Applies a Gaussian filter to an image.
 *
 * This function applies a Gaussian filter to an image.
 *
 * @param image The input image.
 * @param kernel The Gaussian kernel.
 * @return The filtered image.
 */
[[nodiscard("The result of applyGaussianFilter is not used.")]] std::vector<
    std::vector<double>>
applyGaussianFilter(const std::vector<std::vector<double>> &image,
                    const std::vector<std::vector<double>> &kernel);
}  // namespace atom::algorithm

#endif
