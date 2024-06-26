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
#include <cstddef>
#include <thread>

#include "atom/error/exception.hpp"

namespace atom::algorithm {
std::vector<double> convolve(const std::vector<double> &input,
                             const std::vector<double> &kernel) {
    auto input_size = input.size();
    auto kernel_size = kernel.size();
    auto output_size = input_size + kernel_size - 1;
    std::vector<double> output(output_size, 0.0);

    for (std::size_t i = 0; i < output_size; ++i) {
        for (std::size_t j = 0; j < kernel_size; ++j) {
            if (i >= j && (i - j) < input_size) {
                output[i] += input[i - j] * kernel[j];
            }
        }
    }

    return output;
}

std::vector<double> deconvolve(const std::vector<double> &input,
                               const std::vector<double> &kernel) {
    auto input_size = input.size();
    auto kernel_size = kernel.size();
    if (kernel_size > input_size) {
        THROW_EXCEPTION("Kernel size cannot be larger than input size.");
    }

    auto output_size = input_size - kernel_size + 1;
    std::vector<double> output(output_size, 0.0);

    for (std::size_t i = 0; i < output_size; ++i) {
        for (std::size_t j = 0; j < kernel_size; ++j) {
            output[i] += input[i + j] * kernel[j];
        }
    }

    return output;
}

std::vector<std::vector<double>> convolve2D(
    const std::vector<std::vector<double>> &input,
    const std::vector<std::vector<double>> &kernel, int numThreads) {
    auto inputRows = input.size();
    auto inputCols = input[0].size();
    auto kernelRows = kernel.size();
    auto kernelCols = kernel[0].size();

    // Extend input and kernel matrices with zeros
    std::vector<std::vector<double>> extendedInput(
        inputRows + kernelRows - 1,
        std::vector<double>(inputCols + kernelCols - 1, 0.0));
    std::vector<std::vector<double>> extendedKernel(
        inputRows + kernelRows - 1,
        std::vector<double>(inputCols + kernelCols - 1, 0.0));

    // Center the input in the extended input matrix
    for (std::size_t i = 0; i < inputRows; ++i) {
        for (std::size_t j = 0; j < inputCols; ++j) {
            extendedInput[i + kernelRows / 2][j + kernelCols / 2] = input[i][j];
        }
    }

    // Center the kernel in the extended kernel matrix
    for (std::size_t i = 0; i < kernelRows; ++i) {
        for (std::size_t j = 0; j < kernelCols; ++j) {
            extendedKernel[i][j] = kernel[i][j];
        }
    }

    // Prepare output matrix
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

std::vector<std::vector<double>> deconvolve2D(
    const std::vector<std::vector<double>> &signal,
    const std::vector<std::vector<double>> &kernel, int numThreads) {
    int M = signal.size();
    int N = signal[0].size();
    int K = kernel.size();
    int L = kernel[0].size();

    // 扩展信号和卷积核到相同的大小
    std::vector<std::vector<double>> extendedSignal(
        M + K - 1, std::vector<double>(N + L - 1, 0));
    std::vector<std::vector<double>> extendedKernel(
        M + K - 1, std::vector<double>(N + L - 1, 0));

    // 复制原始信号和卷积核到扩展矩阵中
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            extendedSignal[i][j] = signal[i][j];
        }
    }
    for (int i = 0; i < K; ++i) {
        for (int j = 0; j < L; ++j) {
            extendedKernel[i][j] = kernel[i][j];
        }
    }

    // 计算信号和卷积核的二维DFT
    auto DFT2DWrapper = [&](const std::vector<std::vector<double>> &input) {
        return DFT2D(input,
                     numThreads);  // Assume DFT2D supports multithreading
    };

    auto X = DFT2DWrapper(extendedSignal);
    auto H = DFT2DWrapper(extendedKernel);

    // 对卷积核的频谱进行修正
    std::vector<std::vector<std::complex<double>>> G(
        M + K - 1, std::vector<std::complex<double>>(N + L - 1));
    double alpha = 0.1;  // 防止分母为0
    for (int u = 0; u < M + K - 1; ++u) {
        for (int v = 0; v < N + L - 1; ++v) {
            if (std::abs(H[u][v]) > alpha) {
                G[u][v] = std::conj(H[u][v]) / (std::norm(H[u][v]) + alpha);
            } else {
                G[u][v] = std::conj(H[u][v]);
            }
        }
    }

    // 计算反卷积结果
    std::vector<std::vector<std::complex<double>>> Y(
        M + K - 1, std::vector<std::complex<double>>(N + L - 1));
    for (int u = 0; u < M + K - 1; ++u) {
        for (int v = 0; v < N + L - 1; ++v) {
            Y[u][v] = G[u][v] * X[u][v];
        }
    }

    auto y = IDFT2D(Y, numThreads);

    // 提取有效结果
    std::vector<std::vector<double>> result(M, std::vector<double>(N, 0));
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            result[i][j] = y[i][j];
        }
    }

    return result;
}

// 二维离散傅里叶变换（2D DFT）
std::vector<std::vector<std::complex<double>>> DFT2D(
    const std::vector<std::vector<double>> &signal, int numThreads) {
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

// 二维离散傅里叶逆变换（2D IDFT）
std::vector<std::vector<double>> IDFT2D(
    const std::vector<std::vector<std::complex<double>>> &spectrum,
    int numThreads) {
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
                x[m][n] = real(sum) / (M * N);  // Normalize by dividing by M*N
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

std::vector<std::vector<double>> generateGaussianKernel(int size,
                                                        double sigma) {
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

std::vector<std::vector<double>> applyGaussianFilter(
    const std::vector<std::vector<double>> &image,
    const std::vector<std::vector<double>> &kernel) {
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
