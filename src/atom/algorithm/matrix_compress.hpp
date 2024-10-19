#ifndef MATRIX_COMPRESS_HPP
#define MATRIX_COMPRESS_HPP

#include <string>
#include <vector>

#include "atom/error/exception.hpp"

class MatrixCompressException : public atom::error::Exception {
public:
    using atom::error::Exception::Exception;
};

#define THROW_MATRIX_COMPRESS_EXCEPTION(...)                      \
    throw MatrixCompressException(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                  ATOM_FUNC_NAME, __VA_ARGS__);

#define THROW_NESTED_MATRIX_COMPRESS_EXCEPTION(...)                        \
    MatrixCompressException::rethrowNested(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                           ATOM_FUNC_NAME, __VA_ARGS__);

class MatrixDecompressException : public atom::error::Exception {
public:
    using atom::error::Exception::Exception;
};

#define THROW_MATRIX_DECOMPRESS_EXCEPTION(...)                      \
    throw MatrixDecompressException(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                    ATOM_FUNC_NAME, __VA_ARGS__);

#define THROW_NESTED_MATRIX_DECOMPRESS_EXCEPTION(...)                        \
    MatrixDecompressException::rethrowNested(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                             ATOM_FUNC_NAME, __VA_ARGS__);

namespace atom::algorithm {
/**
 * @class MatrixCompressor
 * @brief A class for compressing and decompressing matrices.
 */
class MatrixCompressor {
public:
    using Matrix = std::vector<std::vector<char>>;
    using CompressedData = std::vector<std::pair<char, int>>;

    /**
     * @brief Compresses a matrix using run-length encoding.
     * @param matrix The matrix to compress.
     * @return The compressed data.
     */
    static auto compress(const Matrix& matrix) -> CompressedData;

    /**
     * @brief Decompresses data into a matrix.
     * @param compressed The compressed data.
     * @param rows The number of rows in the decompressed matrix.
     * @param cols The number of columns in the decompressed matrix.
     * @return The decompressed matrix.
     */
    static auto decompress(const CompressedData& compressed, int rows,
                           int cols) -> Matrix;

    /**
     * @brief Prints the matrix to the standard output.
     * @param matrix The matrix to print.
     */
    static void printMatrix(const Matrix& matrix);

    /**
     * @brief Generates a random matrix.
     * @param rows The number of rows in the matrix.
     * @param cols The number of columns in the matrix.
     * @param charset The set of characters to use for generating the matrix.
     * @return The generated random matrix.
     */
    static auto generateRandomMatrix(
        int rows, int cols, const std::string& charset = "ABCD") -> Matrix;

    /**
     * @brief Saves the compressed data to a file.
     * @param compressed The compressed data to save.
     * @param filename The name of the file to save the data to.
     */
    static void saveCompressedToFile(const CompressedData& compressed,
                                     const std::string& filename);

    /**
     * @brief Loads compressed data from a file.
     * @param filename The name of the file to load the data from.
     * @return The loaded compressed data.
     */
    static auto loadCompressedFromFile(const std::string& filename)
        -> CompressedData;

    /**
     * @brief Calculates the compression ratio.
     * @param original The original matrix.
     * @param compressed The compressed data.
     * @return The compression ratio.
     */
    static auto calculateCompressionRatio(
        const Matrix& original, const CompressedData& compressed) -> double;

    /**
     * @brief Downsamples a matrix by a given factor.
     * @param matrix The matrix to downsample.
     * @param factor The downsampling factor.
     * @return The downsampled matrix.
     */
    static auto downsample(const Matrix& matrix, int factor) -> Matrix;

    /**
     * @brief Upsamples a matrix by a given factor.
     * @param matrix The matrix to upsample.
     * @param factor The upsampling factor.
     * @return The upsampled matrix.
     */
    static auto upsample(const Matrix& matrix, int factor) -> Matrix;

    /**
     * @brief Calculates the mean squared error (MSE) between two matrices.
     * @param matrix1 The first matrix.
     * @param matrix2 The second matrix.
     * @return The mean squared error.
     */
    static auto calculateMSE(const Matrix& matrix1,
                             const Matrix& matrix2) -> double;
};

#if ATOM_ENABLE_DEBUG
/**
 * @brief Runs a performance test on matrix compression and decompression.
 * @param rows The number of rows in the test matrix.
 * @param cols The number of columns in the test matrix.
 */
void performanceTest(int rows, int cols);
#endif
}  // namespace atom::algorithm

#endif  // MATRIX_COMPRESS_HPP
