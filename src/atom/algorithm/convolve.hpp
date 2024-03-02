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

#include <vector>

namespace Atom::Algorithm {
/**
 * @brief Perform one-dimensional convolution.
 *
 * @param input The input signal.
 * @param kernel The convolution kernel.
 * @return The result of the convolution.
 */
[[nodiscard]] [[maybe_unused]] std::vector<double> convolve(
    const std::vector<double> &input, const std::vector<double> &kernel);

/**
 * @brief Perform one-dimensional deconvolution.
 *
 * @param input The input signal.
 * @param kernel The deconvolution kernel.
 * @return The result of the deconvolution.
 */
[[nodiscard]] [[maybe_unused]] std::vector<double> deconvolve(
    const std::vector<double> &input, const std::vector<double> &kernel);

/**
 * @brief Perform two-dimensional convolution.
 *
 * @param input The 2D input signal.
 * @param kernel The 2D convolution kernel.
 * @param numThreads The number of threads to use for parallel computation.
 * @return The result of the 2D convolution.
 */
[[nodiscard]] [[maybe_unused]] std::vector<std::vector<double>> convolve2D(
    const std::vector<std::vector<double>> &input,
    const std::vector<std::vector<double>> &kernel, int numThreads);

/**
 * @brief Perform two-dimensional deconvolution.
 *
 * @param signal The 2D input signal.
 * @param kernel The 2D deconvolution kernel.
 * @return The result of the 2D deconvolution.
 */
[[nodiscard]] [[maybe_unused]] std::vector<std::vector<double>> deconvolve2D(
    const std::vector<std::vector<double>> &signal,
    const std::vector<std::vector<double>> &kernel);
}  // namespace Atom::Algorithm

#endif
