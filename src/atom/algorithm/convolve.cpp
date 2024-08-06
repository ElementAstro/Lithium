/*
 * convolve.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-10

Description: Implementation of one-dimensional and two-dimensional convolution
and deconvolution.

**************************************************/

#include "convolve.hpp"

#include <algorithm>
#include <complex>
#include <cstddef>
#include <numbers>
#include <ranges>
#include <stdexcept>
#include <thread>
#include <vector>

#include "atom/error/exception.hpp"

namespace atom::algorithm {
// Function to convolve a 1D input with a kernel
auto convolve(const std::vector<double> &input,
              const std::vector<double> &kernel) -> std::vector<double> {
    auto inputSize = input.size();
    auto kernelSize = kernel.size();
    auto outputSize = inputSize + kernelSize - 1;
    std::vector<double> output(outputSize, 0.0);

    for (std::size_t i = 0; i < outputSize; ++i) {
        for (std::size_t j = 0; j < kernelSize; ++j) {
            if (i >= j && (i - j) < inputSize) {
                output[i] += input[i - j] * kernel[j];
            }
        }
    }

    return output;
}

// Function to deconvolve a 1D input with a kernel
auto deconvolve(const std::vector<double> &input,
                const std::vector<double> &kernel) -> std::vector<double> {
    auto inputSize = input.size();
    auto kernelSize = kernel.size();
    if (kernelSize > inputSize) {
        THROW_INVALID_ARGUMENT("Kernel size cannot be larger than input size.");
    }

    auto outputSize = inputSize - kernelSize + 1;
    std::vector<double> output(outputSize, 0.0);

    for (std::size_t i = 0; i < outputSize; ++i) {
        for (std::size_t j = 0; j < kernelSize; ++j) {
            output[i] += input[i + j] * kernel[j];
        }
    }

    return output;
}

// Helper function to extend 2D vectors
template <typename T>
auto extend2D(const std::vector<std::vector<T>> &input, std::size_t newRows,
              std::size_t newCols) -> std::vector<std::vector<T>> {
    std::vector<std::vector<T>> extended(newRows, std::vector<T>(newCols, 0.0));
    auto inputRows = input.size();
    auto inputCols = input[0].size();
    for (std::size_t i = 0; i < inputRows; ++i) {
        for (std::size_t j = 0; j < inputCols; ++j) {
            extended[i + newRows / 2][j + newCols / 2] = input[i][j];
        }
    }
    return extended;
}

// Function to convolve a 2D input with a 2D kernel using multithreading
auto convolve2D(const std::vector<std::vector<double>> &input,
                const std::vector<std::vector<double>> &kernel,
                int numThreads) -> std::vector<std::vector<double>> {
    auto inputRows = input.size();
    auto inputCols = input[0].size();
    auto kernelRows = kernel.size();
    auto kernelCols = kernel[0].size();

    auto extendedInput =
        extend2D(input, inputRows + kernelRows - 1, inputCols + kernelCols - 1);
    auto extendedKernel = extend2D(kernel, inputRows + kernelRows - 1,
                                   inputCols + kernelCols - 1);

    std::vector<std::vector<double>> output(
        inputRows, std::vector<double>(inputCols, 0.0));

    // Function to compute a block of the convolution
    auto computeBlock = [&](int blockStartRow, int blockEndRow) {
        for (int i = blockStartRow; i < blockEndRow; ++i) {
            for (std::size_t j = kernelCols / 2; j < inputCols + kernelCols / 2;
                 ++j) {
                double sum = 0.0;
                for (int k = -kernelRows / 2; k <= kernelRows / 2; ++k) {
                    for (int l = -kernelCols / 2; l <= kernelCols / 2; ++l) {
                        sum += extendedInput[i + k][j + l] *
                               extendedKernel[kernelRows / 2 + k]
                                             [kernelCols / 2 + l];
                    }
                }
                output[i - kernelRows / 2][j - kernelCols / 2] = sum;
            }
        }
    };

    // Use multiple threads if requested
    if (numThreads > 1) {
        std::vector<std::jthread> threads;
        int blockSize = (inputRows + numThreads - 1) / numThreads;
        int blockStartRow = kernelRows / 2;

        for (int i = 0; i < numThreads; ++i) {
            int blockEndRow = std::min<unsigned long long>(
                blockStartRow + blockSize, inputRows + kernelRows / 2);
            threads.emplace_back(computeBlock, blockStartRow, blockEndRow);
            blockStartRow = blockEndRow;
        }

        for (auto &thread : threads) {
            thread.join();
        }
    } else {
        // Single-threaded execution
        computeBlock(kernelRows / 2, inputRows + kernelRows / 2);
    }

    return output;
}

// Function to deconvolve a 2D input with a 2D kernel using multithreading
auto deconvolve2D(const std::vector<std::vector<double>> &signal,
                  const std::vector<std::vector<double>> &kernel,
                  int numThreads) -> std::vector<std::vector<double>> {
    int M = signal.size();
    int N = signal[0].size();
    int K = kernel.size();
    int L = kernel[0].size();

    auto extendedSignal = extend2D(signal, M + K - 1, N + L - 1);
    auto extendedKernel = extend2D(kernel, M + K - 1, N + L - 1);

    auto dfT2DWrapper = [&](const std::vector<std::vector<double>> &input) {
        return dfT2D(input,
                     numThreads);  // Assume DFT2D supports multithreading
    };

    auto x = dfT2DWrapper(extendedSignal);
    auto h = dfT2DWrapper(extendedKernel);

    std::vector<std::vector<std::complex<double>>> g(
        M + K - 1, std::vector<std::complex<double>>(N + L - 1));
    double alpha = 0.1;  // Prevent division by zero
    for (int u = 0; u < M + K - 1; ++u) {
        for (int v = 0; v < N + L - 1; ++v) {
            if (std::abs(h[u][v]) > alpha) {
                g[u][v] = std::conj(h[u][v]) / (std::norm(h[u][v]) + alpha);
            } else {
                g[u][v] = std::conj(h[u][v]);
            }
        }
    }

    std::vector<std::vector<std::complex<double>>> Y(
        M + K - 1, std::vector<std::complex<double>>(N + L - 1));
    for (int u = 0; u < M + K - 1; ++u) {
        for (int v = 0; v < N + L - 1; ++v) {
            Y[u][v] = g[u][v] * x[u][v];
        }
    }

    auto y = idfT2D(Y, numThreads);

    std::vector<std::vector<double>> result(M, std::vector<double>(N, 0.0));
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            result[i][j] = y[i][j];
        }
    }

    return result;
}

// 2D Discrete Fourier Transform (2D DFT)
auto dfT2D(const std::vector<std::vector<double>> &signal,
           int numThreads) -> std::vector<std::vector<std::complex<double>>> {
    const auto M = signal.size();
    const auto N = signal[0].size();
    std::vector<std::vector<std::complex<double>>> X(
        M, std::vector<std::complex<double>>(N, {0, 0}));

    // Lambda function to compute the DFT for a block of rows
    auto computeDFT = [&M, &N, &signal, &X](int startRow, int endRow) {
        for (int u = startRow; u < endRow; ++u) {
            for (int v = 0; v < N; ++v) {
                std::complex<double> sum(0, 0);
                for (int m = 0; m < M; ++m) {
                    for (int n = 0; n < N; ++n) {
                        double theta = -2 * std::numbers::pi *
                                       ((u * m / static_cast<double>(M)) +
                                        (v * n / static_cast<double>(N)));
                        std::complex<double> w(cos(theta), sin(theta));
                        sum += signal[m][n] * w;
                    }
                }
                X[u][v] = sum;
            }
        }
    };

    // Multithreading support
    if (numThreads > 1) {
        std::vector<std::jthread> threads;
        auto rowsPerThread = M / numThreads;
        for (int i = 0; i < numThreads; ++i) {
            auto startRow = i * rowsPerThread;
            auto endRow = (i == numThreads - 1) ? M : startRow + rowsPerThread;
            threads.emplace_back(computeDFT, startRow, endRow);
        }
        for (auto &thread : threads) {
            thread.join();
        }
    } else {
        // Single-threaded execution
        computeDFT(0, M);
    }

    return X;
}

// 2D Inverse Discrete Fourier Transform (2D IDFT)
auto idfT2D(const std::vector<std::vector<std::complex<double>>> &spectrum,
            int numThreads) -> std::vector<std::vector<double>> {
    const auto M = spectrum.size();
    const auto N = spectrum[0].size();
    std::vector<std::vector<double>> x(M, std::vector<double>(N, 0.0));

    // Lambda function to compute the IDFT for a block of rows
    auto computeIDFT = [&M, &N, &spectrum, &x](int startRow, int endRow) {
        for (int m = startRow; m < endRow; ++m) {
            for (int n = 0; n < N; ++n) {
                std::complex<double> sum(0.0, 0.0);
                for (int u = 0; u < M; ++u) {
                    for (int v = 0; v < N; ++v) {
                        double theta = 2 * std::numbers::pi *
                                       ((u * m / static_cast<double>(M)) +
                                        (v * n / static_cast<double>(N)));
                        std::complex<double> w(cos(theta), sin(theta));
                        sum += spectrum[u][v] * w;
                    }
                }
                x[m][n] =
                    std::real(sum) / (M * N);  // Normalize by dividing by M*N
            }
        }
    };

    // Multithreading support
    if (numThreads > 1) {
        std::vector<std::jthread> threads;
        auto rowsPerThread = M / numThreads;
        for (int i = 0; i < numThreads; ++i) {
            auto startRow = i * rowsPerThread;
            auto endRow = (i == numThreads - 1) ? M : startRow + rowsPerThread;
            threads.emplace_back(computeIDFT, startRow, endRow);
        }
        for (auto &thread : threads) {
            thread.join();
        }
    } else {
        // Single-threaded execution
        computeIDFT(0, M);
    }

    return x;
}

auto generateGaussianKernel(int size,
                            double sigma) -> std::vector<std::vector<double>> {
    std::vector<std::vector<double>> kernel(size, std::vector<double>(size));
    double sum = 0.0;
    int center = size / 2;

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            kernel[i][j] = exp(-0.5 * (pow((i - center) / sigma, 2.0) +
                                       pow((j - center) / sigma, 2.0))) /
                           (2 * std::numbers::pi * sigma * sigma);
            sum += kernel[i][j];
        }
    }

    // 归一化，确保权重和为1
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            kernel[i][j] /= sum;
        }
    }

    return kernel;
}

auto applyGaussianFilter(const std::vector<std::vector<double>> &image,
                         const std::vector<std::vector<double>> &kernel)
    -> std::vector<std::vector<double>> {
    auto imageHeight = image.size();
    auto imageWidth = image[0].size();
    auto kernelSize = kernel.size();
    auto kernelRadius = kernelSize / 2;
    std::vector<std::vector<double>> filteredImage(
        imageHeight, std::vector<double>(imageWidth, 0));

    for (auto i = 0; i < imageHeight; ++i) {
        for (auto j = 0; j < imageWidth; ++j) {
            double sum = 0.0;
            for (auto k = -kernelRadius; k <= kernelRadius; ++k) {
                for (auto l = -kernelRadius; l <= kernelRadius; ++l) {
                    auto x = std::clamp(static_cast<int>(i + k), 0,
                                        static_cast<int>(imageHeight) - 1);
                    auto y = std::clamp(static_cast<int>(j + l), 0,
                                        static_cast<int>(imageWidth) - 1);
                    sum += image[x][y] *
                           kernel[kernelRadius + k][kernelRadius + l];
                }
            }
            filteredImage[i][j] = sum;
        }
    }

    return filteredImage;
}

}  // namespace atom::algorithm
