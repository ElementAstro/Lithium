#include "matrix_compress.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <random>
#include "error/exception.hpp"

#if USE_SIMD
#include <immintrin.h>
#endif

namespace atom::algorithm {
auto MatrixCompressor::compress(const Matrix& matrix) -> CompressedData {
    CompressedData compressed;
    if (matrix.empty() || matrix[0].empty()) {
        return compressed;
    }

    char currentChar = matrix[0][0];
    int count = 0;

#ifdef USE_SIMD
    // 使用 SIMD 优化压缩
    for (const auto& row : matrix) {
        for (size_t i = 0; i < row.size(); i += 16) {
            __m128i chars =
                _mm_loadu_si128(reinterpret_cast<const __m128i*>(&row[i]));
            for (int j = 0; j < 16; ++j) {
                char ch = reinterpret_cast<const char*>(&chars)[j];
                if (ch == currentChar) {
                    count++;
                } else {
                    compressed.emplace_back(currentChar, count);
                    currentChar = ch;
                    count = 1;
                }
            }
        }
    }
#else
    // 常规压缩
    for (const auto& row : matrix) {
        for (char ch : row) {
            if (ch == currentChar) {
                count++;
            } else {
                compressed.emplace_back(currentChar, count);
                currentChar = ch;
                count = 1;
            }
        }
    }
#endif
    compressed.emplace_back(currentChar, count);

    return compressed;
}

auto MatrixCompressor::decompress(const CompressedData& compressed, int rows,
                                  int cols) -> Matrix {
    Matrix matrix(rows, std::vector<char>(cols));
    int index = 0;

#ifdef USE_SIMD
    // 使用 SIMD 优化解压缩
    for (const auto& [ch, count] : compressed) {
        __m128i chars = _mm_set1_epi8(ch);
        for (int i = 0; i < count; i += 16) {
            int remaining = std::min(16, count - i);
            for (int j = 0; j < remaining; ++j) {
                int row = index / cols;
                int col = index % cols;
                if (row >= rows || col >= cols) {
                    THROW_MATRIX_DECOMPRESS_EXCEPTION(
                        "Decompression error: Invalid matrix size");
                }
                matrix[row][col] = reinterpret_cast<const char*>(&chars)[j];
                index++;
            }
        }
    }
#else
    // 常规解压缩
    for (const auto& [ch, count] : compressed) {
        for (int i = 0; i < count; ++i) {
            int row = index / cols;
            int col = index % cols;
            if (row >= rows || col >= cols) {
                THROW_MATRIX_DECOMPRESS_EXCEPTION(
                    "Decompression error: Invalid matrix size");
            }
            matrix[row][col] = ch;
            index++;
        }
    }
#endif

    if (index != rows * cols) {
        THROW_MATRIX_DECOMPRESS_EXCEPTION(
            "Decompression error: Incorrect number of elements");
    }

    return matrix;
}

void MatrixCompressor::printMatrix(const Matrix& matrix) {
    for (const auto& row : matrix) {
        for (char ch : row) {
            std::cout << ch << ' ';
        }
        std::cout << '\n';
    }
}

auto MatrixCompressor::generateRandomMatrix(
    int rows, int cols, const std::string& charset) -> Matrix {
    std::random_device randomDevice;
    std::mt19937 generator(randomDevice());
    std::uniform_int_distribution<int> distribution(
        0, static_cast<int>(charset.length()) - 1);

    Matrix matrix(rows, std::vector<char>(cols));
    for (auto& row : matrix) {
        std::ranges::generate(row.begin(), row.end(), [&]() {
            return charset[distribution(generator)];
        });
    }
    return matrix;
}

void MatrixCompressor::saveCompressedToFile(const CompressedData& compressed,
                                            const std::string& filename) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        THROW_FAIL_TO_OPEN_FILE("Unable to open file for writing: " + filename);
    }

    for (const auto& [ch, count] : compressed) {
        file.write(reinterpret_cast<const char*>(&ch), sizeof(ch));
        file.write(reinterpret_cast<const char*>(&count), sizeof(count));
    }
}

auto MatrixCompressor::loadCompressedFromFile(const std::string& filename)
    -> CompressedData {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        THROW_FAIL_TO_OPEN_FILE("Unable to open file for reading: " + filename);
    }

    CompressedData compressed;
    char ch;
    int count;
    while (file.read(reinterpret_cast<char*>(&ch), sizeof(ch)) &&
           file.read(reinterpret_cast<char*>(&count), sizeof(count))) {
        compressed.emplace_back(ch, count);
    }

    return compressed;
}

auto MatrixCompressor::calculateCompressionRatio(
    const Matrix& original, const CompressedData& compressed) -> double {
    size_t originalSize = original.size() * original[0].size() * sizeof(char);
    size_t compressedSize = compressed.size() * (sizeof(char) + sizeof(int));
    return static_cast<double>(compressedSize) /
           static_cast<double>(originalSize);
}

auto MatrixCompressor::downsample(const Matrix& matrix, int factor) -> Matrix {
    if (factor <= 0) {
        THROW_INVALID_ARGUMENT("Downsampling factor must be positive");
    }

    int rows = static_cast<int>(matrix.size());
    int cols = static_cast<int>(matrix[0].size());
    int newRows = std::max(1, rows / factor);
    int newCols = std::max(1, cols / factor);

    Matrix downsampled(newRows, std::vector<char>(newCols));

    for (int i = 0; i < newRows; ++i) {
        for (int j = 0; j < newCols; ++j) {
            // 使用简单的平均值作为降采样策略
            int sum = 0;
            int count = 0;
            for (int di = 0; di < factor && i * factor + di < rows; ++di) {
                for (int dj = 0; dj < factor && j * factor + dj < cols; ++dj) {
                    sum += matrix[i * factor + di][j * factor + dj];
                    count++;
                }
            }
            downsampled[i][j] = static_cast<char>(sum / count);
        }
    }

    return downsampled;
}

auto MatrixCompressor::upsample(const Matrix& matrix, int factor) -> Matrix {
    if (factor <= 0) {
        THROW_INVALID_ARGUMENT("Upsampling factor must be positive");
    }

    int rows = static_cast<int>(matrix.size());
    int cols = static_cast<int>(matrix[0].size());
    int newRows = rows * factor;
    int newCols = cols * factor;

    Matrix upsampled(newRows, std::vector<char>(newCols));

    for (int i = 0; i < newRows; ++i) {
        for (int j = 0; j < newCols; ++j) {
            // 使用最近邻插值
            upsampled[i][j] = matrix[i / factor][j / factor];
        }
    }

    return upsampled;
}

auto MatrixCompressor::calculateMSE(const Matrix& matrix1,
                                    const Matrix& matrix2) -> double {
    if (matrix1.size() != matrix2.size() ||
        matrix1[0].size() != matrix2[0].size()) {
        THROW_INVALID_ARGUMENT("Matrices must have the same dimensions");
    }

    double mse = 0.0;
    auto rows = static_cast<int>(matrix1.size());
    auto cols = static_cast<int>(matrix1[0].size());

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            double diff = static_cast<double>(matrix1[i][j]) -
                          static_cast<double>(matrix2[i][j]);
            mse += diff * diff;
        }
    }

    return mse / (rows * cols);
}

#if ATOM_ENABLE_DEBUG
void performanceTest(int rows, int cols) {
    auto matrix = MatrixCompressor::generateRandomMatrix(rows, cols);

    auto start = std::chrono::high_resolution_clock::now();
    auto compressed = MatrixCompressor::compress(matrix);
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> compression_time = end - start;

    start = std::chrono::high_resolution_clock::now();
    auto decompressed = MatrixCompressor::decompress(compressed, rows, cols);
    end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> decompression_time = end - start;

    double compression_ratio =
        MatrixCompressor::calculateCompressionRatio(matrix, compressed);

    std::cout << "Matrix size: " << rows << "x" << cols << "\n";
    std::cout << "Compression time: " << compression_time.count() << " ms\n";
    std::cout << "Decompression time: " << decompression_time.count()
              << " ms\n";
    std::cout << "Compression ratio: " << compression_ratio << "\n";
    std::cout << "Compressed size: " << compressed.size() << " elements\n";
}
#endif
}  // namespace atom::algorithm
