#include "curve.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <future>
#include <mutex>
#include <thread>

#include "atom/log/loguru.hpp"
#include "atom/system/command.hpp"

class FocusCurveFitter::Impl {
public:
    Impl() = default;
    ~Impl() = default;

    void addDataPoint(double position, double sharpness) {
        std::lock_guard lock(data_mutex_);
        data_.emplace_back(DataPoint{position, sharpness});
    }

    auto fitCurve() -> std::vector<double> {
        std::lock_guard lock(fit_mutex_);
        switch (currentModel) {
            case ModelType::POLYNOMIAL:
                return fitPolynomialCurve();
            case ModelType::GAUSSIAN:
                return fitGaussianCurve();
            case ModelType::LORENTZIAN:
                return fitLorentzianCurve();
        }
        return {};
    }

    void autoSelectModel() {
        std::lock_guard lock(fit_mutex_);
        std::vector<ModelType> models = {
            ModelType::POLYNOMIAL, ModelType::GAUSSIAN, ModelType::LORENTZIAN};
        double bestAic = std::numeric_limits<double>::infinity();
        ModelType bestModel = ModelType::POLYNOMIAL;

        std::vector<std::future<std::pair<ModelType, double>>> futures;
        futures.reserve(models.size());
        for (const auto& model : models) {
            futures.emplace_back(
                std::async(std::launch::async, [this, model]() {
                    currentModel = model;
                    auto coeffs = fitCurve();
                    double aic = calculateAIC(coeffs);
                    return std::make_pair(model, aic);
                }));
        }

        for (auto& future : futures) {
            auto [model, aic] = future.get();
            if (aic < bestAic) {
                bestAic = aic;
                bestModel = model;
            }
        }

        currentModel = bestModel;
        LOG_F(INFO, "Selected model: {}", getModelName(currentModel));
    }

    auto calculateConfidenceIntervals(double confidence_level = 0.95)
        -> std::vector<std::pair<double, double>> {
        std::lock_guard lock(fit_mutex_);
        auto coeffs = fitCurve();
        int dataSize = static_cast<int>(data_.size());
        int coeffsSize = static_cast<int>(coeffs.size());
        double tValue =
            calculateTValue(dataSize - coeffsSize, confidence_level);

        std::vector<std::pair<double, double>> intervals;
        intervals.reserve(coeffsSize);
        for (int i = 0; i < coeffsSize; ++i) {
            double stdError = calculateStandardError(coeffs, i);
            intervals.emplace_back(coeffs[i] - tValue * stdError,
                                   coeffs[i] + tValue * stdError);
        }
        return intervals;
    }

    void visualize(const std::string& filename = "focus_curve.png") {
        std::lock_guard lock(data_mutex_);
        std::ofstream gnuplotScript("plot_script.gp");
        if (!gnuplotScript.is_open()) {
            LOG_F(ERROR, "Failed to open gnuplot script file.");
            return;
        }
        gnuplotScript << "set terminal png enhanced\n";
        gnuplotScript << "set output '" << filename << "'\n";
        gnuplotScript << "set title 'Focus Position Curve'\n";
        gnuplotScript << "set xlabel 'Position'\n";
        gnuplotScript << "set ylabel 'Sharpness'\n";
        gnuplotScript << "plot '-' with points title 'Data', '-' with lines "
                         "title 'Fitted Curve'\n";

        for (const auto& point : data_) {
            gnuplotScript << point.position << " " << point.sharpness << "\n";
        }
        gnuplotScript << "e\n";

        auto coeffs = fitCurve();
        double minPos = data_.front().position;
        double maxPos = data_.back().position;
        int steps = 1000;  // Increased steps for higher resolution
        double stepSize = (maxPos - minPos) / steps;
        for (int i = 0; i <= steps; ++i) {
            double pos = minPos + i * stepSize;
            double val = evaluateCurve(coeffs, pos);
            gnuplotScript << pos << " " << val << "\n";
        }
        gnuplotScript << "e\n";
        gnuplotScript.close();

        auto res =
            atom::system::executeCommandWithStatus("gnuplot plot_script.gp");
        if (res.second != 0) {
            LOG_F(ERROR, "Failed to execute gnuplot script: {}", res.first);
            return;
        }
        LOG_F(INFO, "Curve visualization saved as {}", filename);
    }

    void preprocessData() {
        std::lock_guard lock(data_mutex_);
        std::sort(data_.begin(), data_.end(),
                  [](const DataPoint& a, const DataPoint& b) {
                      return a.position < b.position;
                  });

        auto last = std::unique(data_.begin(), data_.end(),
                                [](const DataPoint& a, const DataPoint& b) {
                                    return a.position == b.position;
                                });
        data_.erase(last, data_.end());

        double minPos = data_.front().position;
        double maxPos = data_.back().position;
        double minSharpness = std::numeric_limits<double>::infinity();
        double maxSharpness = -std::numeric_limits<double>::infinity();

        for (const auto& point : data_) {
            if (point.sharpness < minSharpness)
                minSharpness = point.sharpness;
            if (point.sharpness > maxSharpness)
                maxSharpness = point.sharpness;
        }

        // Normalize data
        for (auto& point : data_) {
            point.position = (point.position - minPos) / (maxPos - minPos);
            point.sharpness = (point.sharpness - minSharpness) /
                              (maxSharpness - minSharpness);
        }
    }

    void realTimeFitAndPredict(double new_position) {
        addDataPoint(new_position, 0);
        preprocessData();
        auto coeffs = fitCurve();
        double predictedSharpness = evaluateCurve(coeffs, new_position);
        LOG_F(INFO, "Predicted sharpness at position {}: {}", new_position,
              predictedSharpness);
    }

    void parallelFitting() {
        std::lock_guard lock(fit_mutex_);
        int numThreads = std::thread::hardware_concurrency();
        if (numThreads == 0) {
            numThreads = 2;  // Fallback
        }
        std::vector<std::future<std::vector<double>>> futures;
        futures.reserve(numThreads);

        for (int i = 0; i < numThreads; ++i) {
            futures.emplace_back(std::async(std::launch::async,
                                            [this]() { return fitCurve(); }));
        }

        std::vector<std::vector<double>> results;
        results.reserve(futures.size());
        for (auto& future : futures) {
            results.emplace_back(future.get());
        }

        // Choose the best fit based on MSE
        auto bestFit =
            *std::min_element(results.begin(), results.end(),
                              [this](const auto& a, const auto& b) {
                                  return calculateMSE(a) < calculateMSE(b);
                              });

        LOG_F(INFO, "Best parallel fit MSE: {}", calculateMSE(bestFit));
    }

    void saveFittedCurve(const std::string& filename) {
        std::lock_guard lock(fit_mutex_);
        auto coeffs = fitCurve();
        std::ofstream outFile(filename, std::ios::binary);
        if (!outFile.is_open()) {
            LOG_F(ERROR, "Failed to open file for saving fitted curve.");
            return;
        }
        size_t size = coeffs.size();
        outFile.write(reinterpret_cast<char*>(&size), sizeof(size));
        outFile.write(reinterpret_cast<char*>(coeffs.data()),
                      size * sizeof(double));
        outFile.close();
        LOG_F(INFO, "Fitted curve saved to {}", filename);
    }

    void loadFittedCurve(const std::string& filename) {
        std::lock_guard lock(fit_mutex_);
        std::ifstream inFile(filename, std::ios::binary);
        if (!inFile.is_open()) {
            LOG_F(ERROR, "Failed to open file for loading fitted curve.");
            return;
        }
        size_t size;
        inFile.read(reinterpret_cast<char*>(&size), sizeof(size));
        std::vector<double> coeffs(size);
        inFile.read(reinterpret_cast<char*>(coeffs.data()),
                    size * sizeof(double));
        inFile.close();
        LOG_F(INFO, "Fitted curve loaded from {}", filename);
        // You can add logic to apply the loaded coefficients
    }

private:
    std::vector<DataPoint> data_;
    int polynomialDegree = 3;  // Increased degree for better fit
    ModelType currentModel = ModelType::POLYNOMIAL;

    std::mutex data_mutex_;
    std::mutex fit_mutex_;

    // Helper functions
    auto fitPolynomialCurve() -> std::vector<double> {
        int dataSize = static_cast<int>(data_.size());
        int degree = polynomialDegree;

        std::vector<std::vector<double>> matrixX(
            dataSize, std::vector<double>(degree + 1, 1.0));
        std::vector<double> vectorY(dataSize);

        for (int i = 0; i < dataSize; ++i) {
            for (int j = 1; j <= degree; ++j) {
                matrixX[i][j] = std::pow(data_[i].position, j);
            }
            vectorY[i] = data_[i].sharpness;
        }

        auto matrixXt = transpose(matrixX);
        auto matrixXtX = matrixMultiply(matrixXt, matrixX);
        auto vectorXty = matrixVectorMultiply(matrixXt, vectorY);
        return solveLinearSystem(matrixXtX, vectorXty);
    }

    auto fitGaussianCurve() -> std::vector<double> {
        int dataSize = static_cast<int>(data_.size());
        if (dataSize < 4) {
            LOG_F(ERROR, "Not enough data points for Gaussian fit.");
            return {};
        }

        auto [min_it, max_it] =
            std::minmax_element(data_.begin(), data_.end(),
                                [](const DataPoint& a, const DataPoint& b) {
                                    return a.sharpness < b.sharpness;
                                });

        std::vector<double> initialGuess = {
            max_it->sharpness - min_it->sharpness, max_it->position, 0.1,
            min_it->sharpness};

        return levenbergMarquardt(
            initialGuess,
            [this](double position,
                   const std::vector<double>& params) -> double {
                double amplitude = params[0];
                double mean = params[1];
                double std_dev = params[2];
                double offset = params[3];
                return amplitude * std::exp(-std::pow(position - mean, 2) /
                                            (2 * std::pow(std_dev, 2))) +
                       offset;
            });
    }

    auto fitLorentzianCurve() -> std::vector<double> {
        int dataSize = static_cast<int>(data_.size());
        if (dataSize < 4) {
            LOG_F(ERROR, "Not enough data points for Lorentzian fit.");
            return {};
        }

        auto [min_it, max_it] =
            std::minmax_element(data_.begin(), data_.end(),
                                [](const DataPoint& a, const DataPoint& b) {
                                    return a.sharpness < b.sharpness;
                                });

        std::vector<double> initialGuess = {
            max_it->sharpness - min_it->sharpness, max_it->position, 0.1,
            min_it->sharpness};

        return levenbergMarquardt(
            initialGuess,
            [this](double position,
                   const std::vector<double>& params) -> double {
                double amplitude = params[0];
                double center = params[1];
                double width = params[2];
                double offset = params[3];
                return amplitude /
                           (1 + std::pow((position - center) / width, 2)) +
                       offset;
            });
    }

    auto levenbergMarquardt(
        const std::vector<double>& initial_guess,
        std::function<double(double, const std::vector<double>&)> model)
        -> std::vector<double> {
        // Implementation of Levenberg-Marquardt algorithm
        // For brevity, a placeholder is provided
        // In practice, use a library like Eigen or Ceres Solver
        return initial_guess;  // Placeholder
    }

    auto calculateAIC(const std::vector<double>& coeffs) -> double {
        int dataSize = static_cast<int>(data_.size());
        int coeffsSize = static_cast<int>(coeffs.size());
        double mse = calculateMSE(coeffs);
        return dataSize * std::log(mse) + 2 * coeffsSize;
    }

    double calculateMSE(const std::vector<double>& coeffs) {
        double mse = 0.0;
        for (const auto& point : data_) {
            double fit = evaluateCurve(coeffs, point.position);
            mse += std::pow(point.sharpness - fit, 2);
        }
        return mse / static_cast<double>(data_.size());
    }

    auto calculateTValue(int degrees_of_freedom,
                         double confidence_level) -> double {
        // Placeholder for T-distribution value
        // In practice, use a statistics library
        if (confidence_level == 0.95) {
            return 1.96;
        }
        return 1.0;
    }

    double calculateStandardError(const std::vector<double>& coeffs,
                                  int index) {
        double mse = calculateMSE(coeffs);
        return std::sqrt(mse);  // Simplified
    }

    auto transpose(const std::vector<std::vector<double>>& matrix_A)
        -> std::vector<std::vector<double>> {
        int rows = static_cast<int>(matrix_A.size());
        int cols = static_cast<int>(matrix_A[0].size());
        std::vector<std::vector<double>> matrixAt(
            cols, std::vector<double>(rows, 0.0));

        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                matrixAt[j][i] = matrix_A[i][j];
            }
        }
        return matrixAt;
    }

    auto matrixMultiply(const std::vector<std::vector<double>>& A,
                        const std::vector<std::vector<double>>& B)
        -> std::vector<std::vector<double>> {
        int rows = static_cast<int>(A.size());
        int cols = static_cast<int>(B[0].size());
        int inner = static_cast<int>(A[0].size());

        std::vector<std::vector<double>> C(rows,
                                           std::vector<double>(cols, 0.0));

        for (int i = 0; i < rows; ++i) {
            for (int k = 0; k < inner; ++k) {
                for (int j = 0; j < cols; ++j) {
                    C[i][j] += A[i][k] * B[k][j];
                }
            }
        }
        return C;
    }

    auto matrixVectorMultiply(const std::vector<std::vector<double>>& A,
                              const std::vector<double>& v)
        -> std::vector<double> {
        int rows = static_cast<int>(A.size());
        int cols = static_cast<int>(A[0].size());
        std::vector<double> result(rows, 0.0);

        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                result[i] += A[i][j] * v[j];
            }
        }
        return result;
    }

    auto solveLinearSystem(std::vector<std::vector<double>> A,
                           std::vector<double> b) -> std::vector<double> {
        int n = static_cast<int>(A.size());
        for (int i = 0; i < n; ++i) {
            // Partial pivoting
            int maxRow = i;
            for (int k = i + 1; k < n; ++k) {
                if (std::abs(A[k][i]) > std::abs(A[maxRow][i])) {
                    maxRow = k;
                }
            }
            std::swap(A[i], A[maxRow]);
            std::swap(b[i], b[maxRow]);

            // Make all rows below this one 0 in current column
            for (int k = i + 1; k < n; ++k) {
                double factor = A[k][i] / A[i][i];
                for (int j = i; j < n; ++j) {
                    A[k][j] -= factor * A[i][j];
                }
                b[k] -= factor * b[i];
            }
        }

        // Solve equation Ax=b for an upper triangular matrix A
        std::vector<double> x(n, 0.0);
        for (int i = n - 1; i >= 0; --i) {
            x[i] = b[i] / A[i][i];
            for (int k = i - 1; k >= 0; --k) {
                b[k] -= A[k][i] * x[i];
            }
        }
        return x;
    }

    auto evaluateCurve(const std::vector<double>& coeffs, double x) -> double {
        switch (currentModel) {
            case ModelType::POLYNOMIAL:
                return evaluatePolynomial(coeffs, x);
            case ModelType::GAUSSIAN:
                return coeffs[0] * std::exp(-std::pow(x - coeffs[1], 2) /
                                            (2 * std::pow(coeffs[2], 2))) +
                       coeffs[3];
            case ModelType::LORENTZIAN:
                return coeffs[0] /
                           (1 + std::pow((x - coeffs[1]) / coeffs[2], 2)) +
                       coeffs[3];
        }
        return 0.0;
    }

    static auto evaluatePolynomial(const std::vector<double>& coeffs,
                                   double x) -> double {
        double result = 0.0;
        double xn = 1.0;
        for (const auto& c : coeffs) {
            result += c * xn;
            xn *= x;
        }
        return result;
    }

    static auto getModelName(ModelType model) -> std::string {
        switch (model) {
            case ModelType::POLYNOMIAL:
                return "Polynomial";
            case ModelType::GAUSSIAN:
                return "Gaussian";
            case ModelType::LORENTZIAN:
                return "Lorentzian";
        }
        return "Unknown";
    }
};

FocusCurveFitter::FocusCurveFitter() : impl_(std::make_unique<Impl>()) {}

FocusCurveFitter::~FocusCurveFitter() = default;

void FocusCurveFitter::addDataPoint(double position, double sharpness) {
    impl_->addDataPoint(position, sharpness);
}

auto FocusCurveFitter::fitCurve() -> std::vector<double> {
    return impl_->fitCurve();
}

void FocusCurveFitter::autoSelectModel() { impl_->autoSelectModel(); }

auto FocusCurveFitter::calculateConfidenceIntervals(double confidence_level)
    -> std::vector<std::pair<double, double>> {
    return impl_->calculateConfidenceIntervals(confidence_level);
}

void FocusCurveFitter::visualize(const std::string& filename) {
    impl_->visualize(filename);
}

void FocusCurveFitter::preprocessData() { impl_->preprocessData(); }

void FocusCurveFitter::realTimeFitAndPredict(double new_position) {
    impl_->realTimeFitAndPredict(new_position);
}

void FocusCurveFitter::parallelFitting() { impl_->parallelFitting(); }

void FocusCurveFitter::saveFittedCurve(const std::string& filename) {
    impl_->saveFittedCurve(filename);
}

void FocusCurveFitter::loadFittedCurve(const std::string& filename) {
    impl_->loadFittedCurve(filename);
}