#ifndef ATOM_ALGORITHM_MATRIX_HPP
#define ATOM_ALGORITHM_MATRIX_HPP

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>

#include "atom/error/exception.hpp"

namespace atom::algorithm {
template <typename T, std::size_t Rows, std::size_t Cols>
class Matrix;

template <typename T, std::size_t Size>
constexpr Matrix<T, Size, Size> identity();

// 矩阵模板类，支持编译期矩阵计算
template <typename T, std::size_t Rows, std::size_t Cols>
class Matrix {
private:
    std::array<T, Rows * Cols> data_{};

public:
    // 构造函数
    constexpr Matrix() = default;
    constexpr explicit Matrix(const std::array<T, Rows * Cols>& arr)
        : data_(arr) {}

    // 访问矩阵元素
    constexpr auto operator()(std::size_t row, std::size_t col) -> T& {
        return data_[row * Cols + col];
    }

    constexpr auto operator()(std::size_t row,
                              std::size_t col) const -> const T& {
        return data_[row * Cols + col];
    }

    // 数据访问器
    auto getData() const -> const std::array<T, Rows * Cols>& { return data_; }

    auto getData() -> std::array<T, Rows * Cols>& { return data_; }

    // 打印矩阵
    void print(int width = 8, int precision = 2) const {
        for (std::size_t i = 0; i < Rows; ++i) {
            for (std::size_t j = 0; j < Cols; ++j) {
                std::cout << std::setw(width) << std::fixed
                          << std::setprecision(precision) << (*this)(i, j)
                          << ' ';
            }
            std::cout << '\n';
        }
    }

    // 矩阵的迹（对角线元素之和）
    constexpr auto trace() const -> T {
        static_assert(Rows == Cols,
                      "Trace is only defined for square matrices");
        T result = T{};
        for (std::size_t i = 0; i < Rows; ++i) {
            result += (*this)(i, i);
        }
        return result;
    }

    // Frobenius范数
    auto freseniusNorm() const -> T {
        T sum = T{};
        for (const auto& elem : data_) {
            sum += std::norm(elem);
        }
        return std::sqrt(sum);
    }

    // 矩阵的最大元素
    auto maxElement() const -> T {
        return *std::max_element(
            data_.begin(), data_.end(),
            [](const T& a, const T& b) { return std::abs(a) < std::abs(b); });
    }

    // 矩阵的最小元素
    auto minElement() const -> T {
        return *std::min_element(
            data_.begin(), data_.end(),
            [](const T& a, const T& b) { return std::abs(a) < std::abs(b); });
    }

    // 判断矩阵是否为对称矩阵
    [[nodiscard]] auto isSymmetric() const -> bool {
        static_assert(Rows == Cols,
                      "Symmetry is only defined for square matrices");
        for (std::size_t i = 0; i < Rows; ++i) {
            for (std::size_t j = i + 1; j < Cols; ++j) {
                if ((*this)(i, j) != (*this)(j, i)) {
                    return false;
                }
            }
        }
        return true;
    }

    // 矩阵的幂运算
    auto pow(unsigned int n) const -> Matrix {
        static_assert(Rows == Cols,
                      "Matrix power is only defined for square matrices");
        if (n == 0) {
            return identity<T, Rows>();
        }
        if (n == 1) {
            return *this;
        }
        Matrix result = *this;
        for (unsigned int i = 1; i < n; ++i) {
            result = result * (*this);
        }
        return result;
    }

    // 矩阵的行列式（使用LU分解）
    auto determinant() const -> T {
        static_assert(Rows == Cols,
                      "Determinant is only defined for square matrices");
        auto [L, U] = lu_decomposition(*this);
        T det = T{1};
        for (std::size_t i = 0; i < Rows; ++i) {
            det *= U(i, i);
        }
        return det;
    }

    // 矩阵的秩（使用高斯消元）
    [[nodiscard]] auto rank() const -> std::size_t {
        Matrix<T, Rows, Cols> temp = *this;
        std::size_t rank = 0;
        for (std::size_t i = 0; i < Rows && i < Cols; ++i) {
            // 找主元
            std::size_t pivot = i;
            for (std::size_t j = i + 1; j < Rows; ++j) {
                if (std::abs(temp(j, i)) > std::abs(temp(pivot, i))) {
                    pivot = j;
                }
            }
            if (std::abs(temp(pivot, i)) < 1e-10) {
                continue;
            }
            // 交换行
            if (pivot != i) {
                for (std::size_t j = i; j < Cols; ++j) {
                    std::swap(temp(i, j), temp(pivot, j));
                }
            }
            // 消元
            for (std::size_t j = i + 1; j < Rows; ++j) {
                T factor = temp(j, i) / temp(i, i);
                for (std::size_t k = i; k < Cols; ++k) {
                    temp(j, k) -= factor * temp(i, k);
                }
            }
            ++rank;
        }
        return rank;
    }

    // 矩阵的条件数（使用2范数）
    auto conditionNumber() const -> T {
        static_assert(Rows == Cols,
                      "Condition number is only defined for square matrices");
        auto svd = singular_value_decomposition(*this);
        return svd[0] / svd[svd.size() - 1];
    }
};

// 矩阵加法
template <typename T, std::size_t Rows, std::size_t Cols>
constexpr auto operator+(const Matrix<T, Rows, Cols>& a,
                         const Matrix<T, Rows, Cols>& b)
    -> Matrix<T, Rows, Cols> {
    Matrix<T, Rows, Cols> result{};
    for (std::size_t i = 0; i < Rows * Cols; ++i) {
        result.get_data()[i] = a.get_data()[i] + b.get_data()[i];
    }
    return result;
}

// 矩阵减法
template <typename T, std::size_t Rows, std::size_t Cols>
constexpr auto operator-(const Matrix<T, Rows, Cols>& a,
                         const Matrix<T, Rows, Cols>& b)
    -> Matrix<T, Rows, Cols> {
    Matrix<T, Rows, Cols> result{};
    for (std::size_t i = 0; i < Rows * Cols; ++i) {
        result.get_data()[i] = a.get_data()[i] - b.get_data()[i];
    }
    return result;
}

// 矩阵乘法
template <typename T, std::size_t RowsA, std::size_t ColsA_RowsB,
          std::size_t ColsB>
auto operator*(const Matrix<T, RowsA, ColsA_RowsB>& a,
               const Matrix<T, ColsA_RowsB, ColsB>& b)
    -> Matrix<T, RowsA, ColsB> {
    Matrix<T, RowsA, ColsB> result{};
    for (std::size_t i = 0; i < RowsA; ++i) {
        for (std::size_t j = 0; j < ColsB; ++j) {
            for (std::size_t k = 0; k < ColsA_RowsB; ++k) {
                result(i, j) += a(i, k) * b(k, j);
            }
        }
    }
    return result;
}

// 标量乘法（左乘和右乘）
template <typename T, typename U, std::size_t Rows, std::size_t Cols>
constexpr auto operator*(const Matrix<T, Rows, Cols>& m, U scalar) {
    Matrix<decltype(T{} * U{}), Rows, Cols> result;
    for (std::size_t i = 0; i < Rows * Cols; ++i) {
        result.get_data()[i] = m.get_data()[i] * scalar;
    }
    return result;
}

template <typename T, typename U, std::size_t Rows, std::size_t Cols>
constexpr auto operator*(U scalar, const Matrix<T, Rows, Cols>& m) {
    return m * scalar;
}

// 矩阵逐元素乘法（Hadamard积）
template <typename T, std::size_t Rows, std::size_t Cols>
constexpr auto hadamardProduct(const Matrix<T, Rows, Cols>& a,
                               const Matrix<T, Rows, Cols>& b)
    -> Matrix<T, Rows, Cols> {
    Matrix<T, Rows, Cols> result{};
    for (std::size_t i = 0; i < Rows * Cols; ++i) {
        result.get_data()[i] = a.get_data()[i] * b.get_data()[i];
    }
    return result;
}

// 矩阵转置
template <typename T, std::size_t Rows, std::size_t Cols>
constexpr auto transpose(const Matrix<T, Rows, Cols>& m)
    -> Matrix<T, Cols, Rows> {
    Matrix<T, Cols, Rows> result{};
    for (std::size_t i = 0; i < Rows; ++i) {
        for (std::size_t j = 0; j < Cols; ++j) {
            result(j, i) = m(i, j);
        }
    }
    return result;
}

// 创建单位矩阵
template <typename T, std::size_t Size>
constexpr auto identity() -> Matrix<T, Size, Size> {
    Matrix<T, Size, Size> result{};
    for (std::size_t i = 0; i < Size; ++i) {
        result(i, i) = T{1};
    }
    return result;
}

// 矩阵的LU分解
template <typename T, std::size_t Size>
auto luDecomposition(const Matrix<T, Size, Size>& m)
    -> std::pair<Matrix<T, Size, Size>, Matrix<T, Size, Size>> {
    Matrix<T, Size, Size> L = identity<T, Size>();
    Matrix<T, Size, Size> U = m;

    for (std::size_t k = 0; k < Size - 1; ++k) {
        for (std::size_t i = k + 1; i < Size; ++i) {
            if (std::abs(U(k, k)) < 1e-10) {
                THROW_RUNTIME_ERROR(
                    "LU decomposition failed: division by zero");
            }
            T factor = U(i, k) / U(k, k);
            L(i, k) = factor;
            for (std::size_t j = k; j < Size; ++j) {
                U(i, j) -= factor * U(k, j);
            }
        }
    }

    return {L, U};
}

// 矩阵的奇异值分解（仅返回奇异值）
template <typename T, std::size_t Rows, std::size_t Cols>
auto singularValueDecomposition(const Matrix<T, Rows, Cols>& m)
    -> std::vector<T> {
    const std::size_t n = std::min(Rows, Cols);
    Matrix<T, Cols, Rows> mt = transpose(m);
    Matrix<T, Cols, Cols> mtm = mt * m;

    // 使用幂法计算最大特征值和对应的特征向量
    auto powerIteration = [&mtm](std::size_t max_iter = 100, T tol = 1e-10) {
        std::vector<T> v(Cols);
        std::generate(v.begin(), v.end(),
                      []() { return static_cast<T>(rand()) / RAND_MAX; });
        T lambdaOld = 0;
        for (std::size_t iter = 0; iter < max_iter; ++iter) {
            std::vector<T> vNew(Cols);
            for (std::size_t i = 0; i < Cols; ++i) {
                for (std::size_t j = 0; j < Cols; ++j) {
                    vNew[i] += mtm(i, j) * v[j];
                }
            }
            T lambda = 0;
            for (std::size_t i = 0; i < Cols; ++i) {
                lambda += vNew[i] * v[i];
            }
            T norm = std::sqrt(std::inner_product(vNew.begin(), vNew.end(),
                                                  vNew.begin(), T(0)));
            for (auto& x : vNew) {
                x /= norm;
            }
            if (std::abs(lambda - lambdaOld) < tol) {
                return std::sqrt(lambda);
            }
            lambdaOld = lambda;
            v = vNew;
        }
        THROW_RUNTIME_ERROR("Power iteration did not converge");
    };

    std::vector<T> singularValues;
    for (std::size_t i = 0; i < n; ++i) {
        T sigma = powerIteration();
        singularValues.push_back(sigma);
        // Deflate the matrix
        Matrix<T, Cols, Cols> vvt;
        for (std::size_t j = 0; j < Cols; ++j) {
            for (std::size_t k = 0; k < Cols; ++k) {
                vvt(j, k) = mtm(j, k) / (sigma * sigma);
            }
        }
        mtm = mtm - vvt;
    }

    std::sort(singularValues.begin(), singularValues.end(), std::greater<T>());
    return singularValues;
}

// 生成随机矩阵
template <typename T, std::size_t Rows, std::size_t Cols>
auto randomMatrix(T min = 0, T max = 1) -> Matrix<T, Rows, Cols> {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(min, max);

    Matrix<T, Rows, Cols> result;
    for (auto& elem : result.get_data()) {
        elem = dis(gen);
    }
    return result;
}

}  // namespace atom::algorithm

#endif