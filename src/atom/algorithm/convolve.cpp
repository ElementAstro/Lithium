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

#include <complex>
#include <thread>

namespace Atom::Algorithm {
std::vector<double> convolve(const std::vector<double> &input,
                             const std::vector<double> &kernel) {
    int input_size = input.size();
    int kernel_size = kernel.size();
    int output_size = input_size + kernel_size - 1;
    std::vector<double> output(output_size, 0.0);

    for (int i = 0; i < output_size; i++) {
        for (int j = 0; j < kernel_size; j++) {
            if (i - j >= 0 && i - j < input_size) {
                output[i] += input[i - j] * kernel[j];
            }
        }
    }

    return output;
}

std::vector<double> deconvolve(const std::vector<double> &input,
                               const std::vector<double> &kernel) {
    int input_size = input.size();
    int kernel_size = kernel.size();
    int output_size = input_size - kernel_size + 1;
    std::vector<double> output(output_size, 0.0);

    for (int i = 0; i < output_size; i++) {
        for (int j = 0; j < kernel_size; j++) {
            output[i] += input[i + j] * kernel[j];
        }
    }

    return output;
}

std::vector<std::vector<double>> convolve2D(
    const std::vector<std::vector<double>> &input,
    const std::vector<std::vector<double>> &kernel, int numThreads = 1) {
    int inputRows = input.size();
    int inputCols = input[0].size();
    int kernelRows = kernel.size();
    int kernelCols = kernel[0].size();

    // 将输入矩阵和卷积核矩阵扩展到相同的大小，使用0填充
    std::vector<std::vector<double>> extendedInput(
        inputRows + kernelRows - 1,
        std::vector<double>(inputCols + kernelCols - 1, 0));
    std::vector<std::vector<double>> extendedKernel(
        inputRows + kernelRows - 1,
        std::vector<double>(inputCols + kernelCols - 1, 0));

    for (int i = 0; i < inputRows; ++i) {
        for (int j = 0; j < inputCols; ++j) {
            extendedInput[i + kernelRows / 2][j + kernelCols / 2] = input[i][j];
        }
    }

    for (int i = 0; i < kernelRows; ++i) {
        for (int j = 0; j < kernelCols; ++j) {
            extendedKernel[i][j] = kernel[i][j];
        }
    }

    // 计算卷积结果
    std::vector<std::vector<double>> output(inputRows,
                                            std::vector<double>(inputCols, 0));

    auto computeBlock = [&](int blockStartRow, int blockEndRow) {
        for (int i = blockStartRow; i < blockEndRow; ++i) {
            for (int j = kernelCols / 2; j < inputCols + kernelCols / 2; ++j) {
                double sum = 0;
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

    if (numThreads == 1) {
        computeBlock(kernelRows / 2, inputRows + kernelRows / 2);
    } else {
        std::vector<std::thread> threads(numThreads);
        int blockSize = (inputRows + numThreads - 1) / numThreads;
        int blockStartRow = kernelRows / 2;
        for (int i = 0; i < numThreads; ++i) {
            int blockEndRow =
                std::min(blockStartRow + blockSize, inputRows + kernelRows / 2);
            threads[i] = std::thread(computeBlock, blockStartRow, blockEndRow);
            blockStartRow = blockEndRow;
        }
        for (auto &thread : threads) {
            thread.join();
        }
    }

    return output;
}

// 二维离散傅里叶变换（2D DFT）
std::vector<std::vector<std::complex<double>>> DFT2D(
    const std::vector<std::vector<double>> &signal) {
    int M = signal.size();
    int N = signal[0].size();
    std::vector<std::vector<std::complex<double>>> X(
        M, std::vector<std::complex<double>>(N, {0, 0}));

    for (int u = 0; u < M; ++u) {
        for (int v = 0; v < N; ++v) {
            std::complex<double> sum(0, 0);
            for (int m = 0; m < M; ++m) {
                for (int n = 0; n < N; ++n) {
                    double theta = 2 * M_PI *
                                   (u * m / static_cast<double>(M) +
                                    v * n / static_cast<double>(N));
                    std::complex<double> w(cos(theta), -sin(theta));
                    sum += signal[m][n] * w;
                }
            }
            X[u][v] = sum;
        }
    }

    return X;
}

// 二维离散傅里叶逆变换（2D IDFT）
std::vector<std::vector<double>> IDFT2D(
    const std::vector<std::vector<std::complex<double>>> &spectrum) {
    int M = spectrum.size();
    int N = spectrum[0].size();
    std::vector<std::vector<double>> x(M, std::vector<double>(N, 0));

    for (int m = 0; m < M; ++m) {
        for (int n = 0; n < N; ++n) {
            std::complex<double> sum(0, 0);
            for (int u = 0; u < M; ++u) {
                for (int v = 0; v < N; ++v) {
                    double theta = 2 * M_PI *
                                   (u * m / static_cast<double>(M) +
                                    v * n / static_cast<double>(N));
                    std::complex<double> w(cos(theta), sin(theta));
                    sum += spectrum[u][v] * w;
                }
            }
            x[m][n] = real(sum) / (M * N);
        }
    }

    return x;
}

// 二维反卷积函数
std::vector<std::vector<double>> deconvolve2D(
    const std::vector<std::vector<double>> &signal,
    const std::vector<std::vector<double>> &kernel) {
    // 获取信号和卷积核的维度
    int M = signal.size();
    int N = signal[0].size();
    int K = kernel.size();
    int L = kernel[0].size();

    // 将信号和卷积核扩展到相同的大小，使用0填充
    std::vector<std::vector<double>> extendedSignal(
        M + K - 1, std::vector<double>(N + L - 1, 0));
    std::vector<std::vector<double>> extendedKernel(
        M + K - 1, std::vector<double>(N + L - 1, 0));

    // 将信号复制到扩展后的信号数组中
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            extendedSignal[i][j] = signal[i][j];
        }
    }

    // 将卷积核复制到扩展后的卷积核数组中
    for (int i = 0; i < K; ++i) {
        for (int j = 0; j < L; ++j) {
            extendedKernel[i][j] = kernel[i][j];
        }
    }

    // 计算信号和卷积核的二维DFT
    auto X = DFT2D(extendedSignal);
    auto H = DFT2D(extendedKernel);

    // 对卷积核的频谱进行修正
    std::vector<std::vector<std::complex<double>>> G(
        M, std::vector<std::complex<double>>(N, {0, 0}));
    double alpha = 0.1;  // 防止分母为0
    for (int u = 0; u < M; ++u) {
        for (int v = 0; v < N; ++v) {
            if (std::abs(H[u][v]) > alpha) {
                G[u][v] = std::conj(H[u][v]) / (std::norm(H[u][v]) + alpha);
            } else {
                G[u][v] = std::conj(H[u][v]);
            }
        }
    }

    // 计算反卷积结果
    std::vector<std::vector<std::complex<double>>> Y(
        M, std::vector<std::complex<double>>(N, {0, 0}));
    for (int u = 0; u < M; ++u) {
        for (int v = 0; v < N; ++v) {
            Y[u][v] = G[u][v] * X[u][v];
        }
    }
    auto y = IDFT2D(Y);

    // 取出结果的前M行、前N列
    std::vector<std::vector<double>> result(M, std::vector<double>(N, 0));
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            result[i][j] = y[i][j];
        }
    }

    return result;
}
}  // namespace Atom::Algorithm
