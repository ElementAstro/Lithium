#ifndef ATOM_ALGORITHM_ERROR_CALIBRATION_HPP
#define ATOM_ALGORITHM_ERROR_CALIBRATION_HPP

#include <algorithm>
#include <cmath>
#include <concepts>
#include <fstream>
#include <functional>
#include <numeric>
#include <optional>
#include <random>
#include <string>
#include <vector>

#ifdef USE_SIMD
#ifdef __AVX__
#include <immintrin.h>
#elif defined(__ARM_NEON)
#include <arm_neon.h>
#endif
#endif

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"

namespace atom::algorithm {
template <std::floating_point T>
class AdvancedErrorCalibration {
private:
    T slope_ = 1.0;
    T intercept_ = 0.0;
    std::optional<T> r_squared_;
    std::vector<T> residuals_;
    T mse_ = 0.0;  // Mean Squared Error
    T mae_ = 0.0;  // Mean Absolute Error

    /**
     * Calculate calibration metrics
     * @param measured Vector of measured values
     * @param actual Vector of actual values
     */
    void calculateMetrics(const std::vector<T>& measured,
                          const std::vector<T>& actual) {
        T sumSquaredError = 0.0;
        T sumAbsoluteError = 0.0;
        T meanActual =
            std::accumulate(actual.begin(), actual.end(), T(0)) / actual.size();
        T ssTotal = 0;
        T ssResidual = 0;

        residuals_.clear();

#ifdef USE_SIMD
#ifdef __AVX__
        // SIMD optimized loop for x86 using AVX
        __m256d sumSquaredErrorVec = _mm256_setzero_pd();
        __m256d sumAbsoluteErrorVec = _mm256_setzero_pd();
        size_t i = 0;

        for (; i + 4 <= actual.size(); i += 4) {
            __m256d measuredVec = _mm256_loadu_pd(&measured[i]);
            __m256d actualVec = _mm256_loadu_pd(&actual[i]);

            __m256d predictedVec = _mm256_add_pd(
                _mm256_mul_pd(_mm256_set1_pd(slope_), measuredVec),
                _mm256_set1_pd(intercept_));

            __m256d errorVec = _mm256_sub_pd(actualVec, predictedVec);

            sumSquaredErrorVec = _mm256_add_pd(
                sumSquaredErrorVec, _mm256_mul_pd(errorVec, errorVec));
            sumAbsoluteErrorVec =
                _mm256_add_pd(sumAbsoluteErrorVec,
                              _mm256_andnot_pd(_mm256_set1_pd(-0.0), errorVec));

            ssTotal += std::pow(actual[i] - meanActual, 2);
            ssResidual += std::pow(predictedVec[i] - actual[i], 2);
        }

        double tempSquaredError[4];
        _mm256_storeu_pd(tempSquaredError, sumSquaredErrorVec);
        sumSquaredError = std::accumulate(
            tempSquaredError, tempSquaredError + 4, sumSquaredError);

        double tempAbsoluteError[4];
        _mm256_storeu_pd(tempAbsoluteError, sumAbsoluteErrorVec);
        sumAbsoluteError = std::accumulate(
            tempAbsoluteError, tempAbsoluteError + 4, sumAbsoluteError);

#elif defined(__ARM_NEON)
        // SIMD optimized loop for ARM using NEON
        float64x2_t sumSquaredErrorVec = vdupq_n_f64(0.0);
        float64x2_t sumAbsoluteErrorVec = vdupq_n_f64(0.0);
        size_t i = 0;

        for (; i + 2 <= actual.size(); i += 2) {
            float64x2_t measuredVec = vld1q_f64(&measured[i]);
            float64x2_t actualVec = vld1q_f64(&actual[i]);

            float64x2_t predictedVec =
                vmlaq_n_f64(vdupq_n_f64(intercept_), measuredVec, slope_);

            float64x2_t errorVec = vsubq_f64(actualVec, predictedVec);

            sumSquaredErrorVec =
                vmlaq_f64(sumSquaredErrorVec, errorVec, errorVec);
            sumAbsoluteErrorVec =
                vaddq_f64(sumAbsoluteErrorVec, vabsq_f64(errorVec));

            ssTotal += std::pow(actual[i] - meanActual, 2);
            ssResidual += std::pow(predictedVec[i] - actual[i], 2);
        }

        double tempSquaredError[2];
        vst1q_f64(tempSquaredError, sumSquaredErrorVec);
        sumSquaredError = std::accumulate(
            tempSquaredError, tempSquaredError + 2, sumSquaredError);

        double tempAbsoluteError[2];
        vst1q_f64(tempAbsoluteError, sumAbsoluteErrorVec);
        sumAbsoluteError = std::accumulate(
            tempAbsoluteError, tempAbsoluteError + 2, sumAbsoluteError);

#endif
#endif

        for (auto i = 0; i < actual.size(); ++i) {
            T predicted = apply(measured[i]);
            T error = actual[i] - predicted;
            residuals_.push_back(error);

            sumSquaredError += error * error;
            sumAbsoluteError += std::abs(error);
            ssTotal += std::pow(actual[i] - meanActual, 2);
            ssResidual += std::pow(error, 2);
        }

        mse_ = sumSquaredError / actual.size();
        mae_ = sumAbsoluteError / actual.size();
        r_squared_ = 1 - (ssResidual / ssTotal);
    }

    using NonlinearFunction = std::function<T(T, const std::vector<T>&)>;

    /**
     * Solve a system of linear equations using the Levenberg-Marquardt method
     * @param x Vector of x values
     * @param y Vector of y values
     * @param func Nonlinear function to fit
     * @param initial_params Initial guess for the parameters
     * @param max_iterations Maximum number of iterations
     * @param lambda Regularization parameter
     * @param epsilon Convergence criterion
     * @return Vector of optimized parameters
     */
    auto levenbergMarquardt(const std::vector<T>& x, const std::vector<T>& y,
                            NonlinearFunction func,
                            std::vector<T> initial_params,
                            int max_iterations = 100, T lambda = 0.01,
                            T epsilon = 1e-8) -> std::vector<T> {
        int n = x.size();
        int m = initial_params.size();
        std::vector<T> params = initial_params;
        std::vector<T> prevParams(m);
        std::vector<std::vector<T>> jacobian(n, std::vector<T>(m));

        for (int iteration = 0; iteration < max_iterations; ++iteration) {
            std::vector<T> residuals(n);
            for (int i = 0; i < n; ++i) {
                residuals[i] = y[i] - func(x[i], params);
                for (int j = 0; j < m; ++j) {
                    T h = std::max(1e-6, std::abs(params[j]) * 1e-6);
                    std::vector<T> paramsPlusH = params;
                    paramsPlusH[j] += h;
                    jacobian[i][j] =
                        (func(x[i], paramsPlusH) - func(x[i], params)) / h;
                }
            }

            std::vector<std::vector<T>> JTJ(m, std::vector<T>(m));
            std::vector<T> jTr(m);
            for (int i = 0; i < m; ++i) {
                for (int j = 0; j < m; ++j) {
                    JTJ[i][j] = 0;
                    for (int k = 0; k < n; ++k) {
                        JTJ[i][j] += jacobian[k][i] * jacobian[k][j];
                    }
                    if (i == j)
                        JTJ[i][j] += lambda;
                }
                jTr[i] = 0;
                for (int k = 0; k < n; ++k) {
                    jTr[i] += jacobian[k][i] * residuals[k];
                }
            }

            std::vector<T> delta = solveLinearSystem(JTJ, jTr);

            prevParams = params;
            for (int i = 0; i < m; ++i) {
                params[i] += delta[i];
            }

            T diff = 0;
            for (int i = 0; i < m; ++i) {
                diff += std::abs(params[i] - prevParams[i]);
            }
            if (diff < epsilon) {
                break;
            }
        }

        return params;
    }

    /**
     * Solve a system of linear equations using Gaussian elimination
     * @param A Coefficient matrix
     * @param b Right-hand side vector
     * @return Solution vector
     */
    auto solveLinearSystem(const std::vector<std::vector<T>>& A,
                           const std::vector<T>& b) -> std::vector<T> {
        int n = A.size();
        std::vector<std::vector<T>> augmented(n, std::vector<T>(n + 1));
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                augmented[i][j] = A[i][j];
            }
            augmented[i][n] = b[i];
        }

        for (int i = 0; i < n; ++i) {
            int maxRow = i;
            for (int k = i + 1; k < n; ++k) {
                if (std::abs(augmented[k][i]) >
                    std::abs(augmented[maxRow][i])) {
                    maxRow = k;
                }
            }
            std::swap(augmented[i], augmented[maxRow]);

            for (int k = i + 1; k < n; ++k) {
                T factor = augmented[k][i] / augmented[i][i];
                for (int j = i; j <= n; ++j) {
                    augmented[k][j] -= factor * augmented[i][j];
                }
            }
        }

        std::vector<T> x(n);
        for (int i = n - 1; i >= 0; --i) {
            x[i] = augmented[i][n];
            for (int j = i + 1; j < n; ++j) {
                x[i] -= augmented[i][j] * x[j];
            }
            x[i] /= augmented[i][i];
        }

        return x;
    }

public:
    /**
     * Linear calibration using the least squares method
     * @param measured Vector of measured values
     * @param actual Vector of actual values
     */
    void linearCalibrate(const std::vector<T>& measured,
                         const std::vector<T>& actual) {
        if (measured.size() != actual.size() || measured.empty()) {
            THROW_INVALID_ARGUMENT(
                "Input vectors must be non-empty and of equal size");
        }

        T sumX = std::accumulate(measured.begin(), measured.end(), T(0));
        T sumY = std::accumulate(actual.begin(), actual.end(), T(0));
        T sumXy = std::inner_product(measured.begin(), measured.end(),
                                     actual.begin(), T(0));
        T sumXx = std::inner_product(measured.begin(), measured.end(),
                                     measured.begin(), T(0));

        T n = static_cast<T>(measured.size());
        slope_ = (n * sumXy - sumX * sumY) / (n * sumXx - sumX * sumX);
        intercept_ = (sumY - slope_ * sumX) / n;

        calculateMetrics(measured, actual);
    }

    /**
     * Polynomial calibration using the least squares method
     * @param measured Vector of measured values
     * @param actual Vector of actual values
     * @param degree Degree of the polynomial
     */
    void polynomialCalibrate(const std::vector<T>& measured,
                             const std::vector<T>& actual, int degree) {
        if (measured.size() != actual.size() || measured.empty()) {
            THROW_INVALID_ARGUMENT(
                "Input vectors must be non-empty and of equal size");
        }

        auto polyFunc = [degree](T x, const std::vector<T>& params) {
            T result = 0;
            for (int i = 0; i <= degree; ++i) {
                result += params[i] * std::pow(x, i);
            }
            return result;
        };

        std::vector<T> initialParams(degree + 1, 1.0);
        auto params =
            levenbergMarquardt(measured, actual, polyFunc, initialParams);

        slope_ = params[1];      // First-order coefficient as slope
        intercept_ = params[0];  // Constant term as intercept

        calculateMetrics(measured, actual);
    }

    /**
     * Exponential calibration using the least squares method
     * @param measured Vector of measured values
     * @param actual Vector of actual values
     */
    [[nodiscard]] auto apply(T value) const -> T {
        return slope_ * value + intercept_;
    }

    void printParameters() const {
        LOG_F(INFO, "Calibration parameters: slope = {}, intercept = {}",
              slope_, intercept_);
        if (r_squared_.has_value()) {
            LOG_F(INFO, "R-squared = {}", r_squared_.value());
        }
        LOG_F(INFO, "MSE = {}, MAE = {}", mse_, mae_);
    }

    [[nodiscard]] auto getResiduals() const -> std::vector<T> {
        return residuals_;
    }

    void plotResiduals(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file.is_open()) {
            THROW_FAIL_TO_OPEN_FILE("Failed to open file: " + filename);
        }

        file << "Index,Residual\n";
        for (size_t i = 0; i < residuals_.size(); ++i) {
            file << i << "," << residuals_[i] << "\n";
        }
    }

    /**
     * Bootstrap confidence interval for the slope
     * @param measured Vector of measured values
     * @param actual Vector of actual values
     * @param n_iterations Number of bootstrap iterations
     * @param confidence_level Confidence level for the interval
     * @return Pair of lower and upper bounds of the confidence interval
     */
    auto bootstrapConfidenceInterval(
        const std::vector<T>& measured, const std::vector<T>& actual,
        int n_iterations = 1000,
        double confidence_level = 0.95) -> std::pair<T, T> {
        std::vector<T> bootstrapSlopes;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, measured.size() - 1);

        for (int i = 0; i < n_iterations; ++i) {
            std::vector<T> bootMeasured;
            std::vector<T> bootActual;
            for (size_t j = 0; j < measured.size(); ++j) {
                int idx = dis(gen);
                bootMeasured.push_back(measured[idx]);
                bootActual.push_back(actual[idx]);
            }

            AdvancedErrorCalibration<T> bootCalibrator;
            bootCalibrator.linearCalibrate(bootMeasured, bootActual);
            bootstrapSlopes.push_back(bootCalibrator.getSlope());
        }

        std::sort(bootstrapSlopes.begin(), bootstrapSlopes.end());
        int lowerIdx =
            static_cast<int>((1 - confidence_level) / 2 * n_iterations);
        int upperIdx =
            static_cast<int>((1 + confidence_level) / 2 * n_iterations);

        return {bootstrapSlopes[lowerIdx], bootstrapSlopes[upperIdx]};
    }

    /**
     * Detect outliers using the residuals of the calibration
     * @param measured Vector of measured values
     * @param actual Vector of actual values
     * @param threshold Threshold for outlier detection
     * @return Tuple of mean residual, standard deviation, and threshold
     */
    auto outlierDetection(const std::vector<T>& measured,
                          const std::vector<T>& actual,
                          T threshold = 2.0) -> std::tuple<T, T, T> {
        if (residuals_.empty()) {
            THROW_RUNTIME_ERROR("Please call calculate_metrics() first");
        }

        T meanResidual =
            std::accumulate(residuals_.begin(), residuals_.end(), T(0)) /
            residuals_.size();
        T std_dev = std::sqrt(
            std::accumulate(residuals_.begin(), residuals_.end(), T(0),
                            [meanResidual](T acc, T val) {
                                return acc + std::pow(val - meanResidual, 2);
                            }) /
            residuals_.size());

#if ENABLE_DEBUG
        std::cout << "Detected outliers:" << std::endl;
        for (size_t i = 0; i < residuals_.size(); ++i) {
            if (std::abs(residuals_[i] - meanResidual) > threshold * std_dev) {
                std::cout << "Index: " << i << ", Measured: " << measured[i]
                          << ", Actual: " << actual[i]
                          << ", Residual: " << residuals_[i] << std::endl;
            }
        }
#endif
        return {meanResidual, std_dev, threshold};
    }

    void crossValidation(const std::vector<T>& measured,
                         const std::vector<T>& actual, int k = 5) {
        if (measured.size() != actual.size() || measured.size() < k) {
            THROW_INVALID_ARGUMENT(
                "Input vectors must be non-empty and of size greater than k");
        }

        std::vector<T> mseValues;
        std::vector<T> maeValues;
        std::vector<T> rSquaredValues;

        for (int i = 0; i < k; ++i) {
            std::vector<T> trainMeasured;
            std::vector<T> trainActual;
            std::vector<T> testMeasured;
            std::vector<T> testActual;
            for (size_t j = 0; j < measured.size(); ++j) {
                if (j % k == i) {
                    testMeasured.push_back(measured[j]);
                    testActual.push_back(actual[j]);
                } else {
                    trainMeasured.push_back(measured[j]);
                    trainActual.push_back(actual[j]);
                }
            }

            AdvancedErrorCalibration<T> cvCalibrator;
            cvCalibrator.linearCalibrate(trainMeasured, trainActual);

            T foldMse = 0;
            T foldMae = 0;
            T foldSsTotal = 0;
            T foldSsResidual = 0;
            T meanTestActual =
                std::accumulate(testActual.begin(), testActual.end(), T(0)) /
                testActual.size();
            for (size_t j = 0; j < testMeasured.size(); ++j) {
                T predicted = cvCalibrator.apply(testMeasured[j]);
                T error = testActual[j] - predicted;
                foldMse += error * error;
                foldMae += std::abs(error);
                foldSsTotal += std::pow(testActual[j] - meanTestActual, 2);
                foldSsResidual += std::pow(error, 2);
            }

            mseValues.push_back(foldMse / testMeasured.size());
            maeValues.push_back(foldMae / testMeasured.size());
            rSquaredValues.push_back(1 - (foldSsResidual / foldSsTotal));
        }

        T avgMse =
            std::accumulate(mseValues.begin(), mseValues.end(), T(0)) / k;
        T avgMae =
            std::accumulate(maeValues.begin(), maeValues.end(), T(0)) / k;
        T avgRSquared = std::accumulate(rSquaredValues.begin(),
                                        rSquaredValues.end(), T(0)) /
                        k;

#if ENABLE_DEBUG
        std::cout << "K-fold cross-validation results (k = " << k
                  << "):" << std::endl;
        std::cout << "Average MSE: " << avgMse << std::endl;
        std::cout << "Average MAE: " << avgMae << std::endl;
        std::cout << "Average R-squared: " << avgRSquared << std::endl;
#endif
    }

    [[nodiscard]] auto getSlope() const -> T { return slope_; }
    [[nodiscard]] auto getIntercept() const -> T { return intercept_; }
    [[nodiscard]] auto getRSquared() const -> std::optional<T> {
        return r_squared_;
    }
    [[nodiscard]] auto getMse() const -> T { return mse_; }
    [[nodiscard]] auto getMae() const -> T { return mae_; }
};
}  // namespace atom::algorithm

#endif  // ATOM_ALGORITHM_ERROR_CALIBRATION_HPP
