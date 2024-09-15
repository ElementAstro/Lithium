#include "curve.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <future>
#include <numeric>
#include <thread>

#include "atom/log/loguru.hpp"
#include "atom/system/command.hpp"

class FocusCurveFitter::Impl {
public:
    std::vector<DataPoint> data_;
    int polynomial_degree_ = 2;
    ModelType current_model_ = ModelType::POLYNOMIAL;

    void addDataPoint(double position, double sharpness) {
        data_.push_back({position, sharpness});
    }

    std::vector<double> fitCurve() {
        switch (current_model_) {
            case ModelType::POLYNOMIAL:
                return fitPolynomialCurve();
            case ModelType::GAUSSIAN:
                return fitGaussianCurve();
            case ModelType::LORENTZIAN:
                return fitLorentzianCurve();
        }
        return {};
    }

    std::vector<double> fitPolynomialCurve() {
        int n = data_.size();
        int degree = polynomial_degree_;

        std::vector<std::vector<double>> X(n, std::vector<double>(degree + 1));
        std::vector<double> y(n);

        for (int i = 0; i < n; ++i) {
            for (int j = 0; j <= degree; ++j) {
                X[i][j] = std::pow(data_[i].position, j);
            }
            y[i] = data_[i].sharpness;
        }

        auto xt = transpose(X);
        auto xtX = matrixMultiply(xt, X);
        auto xty = matrixVectorMultiply(xt, y);
        return solveLinearSystem(xtX, xty);
    }

    std::vector<double> fitGaussianCurve() {
        auto [min_it, max_it] =
            std::minmax_element(data_.begin(), data_.end(),
                                [](const DataPoint& a, const DataPoint& b) {
                                    return a.sharpness < b.sharpness;
                                });

        std::vector<double> initialGuess = {
            max_it->sharpness - min_it->sharpness, max_it->position, 1.0,
            min_it->sharpness};

        return levenbergMarquardt(
            initialGuess, [](double x, const std::vector<double>& params) {
                double A = params[0], mu = params[1], sigma = params[2],
                       C = params[3];
                return A * std::exp(-std::pow(x - mu, 2) /
                                    (2 * std::pow(sigma, 2))) +
                       C;
            });
    }

    std::vector<double> fitLorentzianCurve() {
        auto [min_it, max_it] =
            std::minmax_element(data_.begin(), data_.end(),
                                [](const DataPoint& a, const DataPoint& b) {
                                    return a.sharpness < b.sharpness;
                                });

        std::vector<double> initialGuess = {
            max_it->sharpness - min_it->sharpness, max_it->position, 1.0,
            min_it->sharpness};

        return levenbergMarquardt(
            initialGuess, [](double x, const std::vector<double>& params) {
                double A = params[0], x0 = params[1], gamma = params[2],
                       C = params[3];
                return A / (1 + std::pow((x - x0) / gamma, 2)) + C;
            });
    }

    void autoSelectModel() {
        std::vector<ModelType> models = {
            ModelType::POLYNOMIAL, ModelType::GAUSSIAN, ModelType::LORENTZIAN};
        double bestAic = std::numeric_limits<double>::infinity();
        ModelType bestModel = ModelType::POLYNOMIAL;

        for (const auto& model : models) {
            current_model_ = model;
            auto coeffs = fitCurve();
            double aic = calculateAIC(coeffs);
            if (aic < bestAic) {
                bestAic = aic;
                bestModel = model;
            }
        }

        current_model_ = bestModel;
        LOG_F(INFO, "Selected model: {}", getModelName(current_model_));
    }

    std::vector<std::pair<double, double>> calculateConfidenceIntervals(
        double confidence_level = 0.95) {
        auto coeffs = fitCurve();
        int n = data_.size();
        int p = coeffs.size();
        double tValue = calculateTValue(n - p, confidence_level);

        std::vector<std::pair<double, double>> intervals;
        for (int i = 0; i < p; ++i) {
            double se = calculateStandardError(coeffs, i);
            intervals.emplace_back(coeffs[i] - tValue * se,
                                   coeffs[i] + tValue * se);
        }
        return intervals;
    }

    void visualize(const std::string& filename = "focus_curve.png") {
        std::ofstream gnuplotScript("plot_script.gp");
        gnuplotScript << "set terminal png\n";
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
        for (double pos = minPos; pos <= maxPos;
             pos += (maxPos - minPos) / 100) {
            gnuplotScript << pos << " " << evaluateCurve(coeffs, pos) << "\n";
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
        std::sort(data_.begin(), data_.end(),
                  [](const DataPoint& a, const DataPoint& b) {
                      return a.position < b.position;
                  });

        data_.erase(std::unique(data_.begin(), data_.end(),
                                [](const DataPoint& a, const DataPoint& b) {
                                    return a.position == b.position;
                                }),
                    data_.end());

        double minPos = data_.front().position;
        double maxPos = data_.back().position;
        double minSharpness = std::numeric_limits<double>::infinity();
        double maxSharpness = -std::numeric_limits<double>::infinity();

        for (const auto& point : data_) {
            minSharpness = std::min(minSharpness, point.sharpness);
            maxSharpness = std::max(maxSharpness, point.sharpness);
        }

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
        int numThreads = std::thread::hardware_concurrency();
        std::vector<std::future<std::vector<double>>> futures;

        for (int i = 0; i < numThreads; ++i) {
            futures.push_back(std::async(std::launch::async,
                                         [this]() { return fitCurve(); }));
        }

        std::vector<std::vector<double>> results;
        results.reserve(futures.size());
        for (auto& future : futures) {
            results.push_back(future.get());
        }

        // Choose the best fit based on MSE
        auto bestFit =
            *std::min_element(results.begin(), results.end(),
                              [this](const auto& a, const auto& b) {
                                  return calculateMSE(a) < calculateMSE(b);
                              });

        LOG_F(INFO, "Best parallel fit MSE: {}", calculateMSE(bestFit));
    }

private:
    // Helper functions

    static auto matrixVectorMultiply(const std::vector<std::vector<double>>& A,
                                     const std::vector<double>& v)
        -> std::vector<double> {
        int m = A.size();
        int n = A[0].size();
        std::vector<double> result(m, 0.0);

        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < n; ++j) {
                result[i] += A[i][j] * v[j];
            }
        }
        return result;
    }

    static auto matrixMultiply(const std::vector<std::vector<double>>& A,
                               const std::vector<std::vector<double>>& B)
        -> std::vector<std::vector<double>> {
        int m = A.size();
        int n = A[0].size();
        int p = B[0].size();

        std::vector<std::vector<double>> C(m, std::vector<double>(p, 0.0));

        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < p; ++j) {
                for (int k = 0; k < n; ++k) {
                    C[i][j] += A[i][k] * B[k][j];
                }
            }
        }
        return C;
    }

    template <typename Func>
    auto levenbergMarquardt(const std::vector<double>& initial_guess,
                            Func model) -> std::vector<double> {
        const int MAX_ITERATIONS = 100;
        const double TOLERANCE = 1e-6;
        double lambda = 0.001;

        std::vector<double> params = initial_guess;
        int n = data_.size();
        int p = initial_guess.size();

        for (int iter = 0; iter < MAX_ITERATIONS; ++iter) {
            std::vector<std::vector<double>> J(
                n, std::vector<double>(p));  // Jacobian matrix
            std::vector<double> residuals(n);

            for (int i = 0; i < n; ++i) {
                double x = data_[i].position;
                double y = data_[i].sharpness;
                double modelValue = model(x, params);
                residuals[i] = y - modelValue;

                for (int j = 0; j < p; ++j) {
                    std::vector<double> paramsDelta = params;
                    paramsDelta[j] += TOLERANCE;
                    double modelDelta = model(x, paramsDelta);
                    J[i][j] = (modelDelta - modelValue) / TOLERANCE;
                }
            }

            auto jt = transpose(J);
            auto jtJ = matrixMultiply(jt, J);
            for (int i = 0; i < p; ++i) {
                jtJ[i][i] += lambda;
            }
            auto jtr = matrixVectorMultiply(jt, residuals);
            auto deltaParams = solveLinearSystem(jtJ, jtr);

            for (int i = 0; i < p; ++i) {
                params[i] += deltaParams[i];
            }

            if (std::inner_product(deltaParams.begin(), deltaParams.end(),
                                   deltaParams.begin(), 0.0) < TOLERANCE) {
                break;
            }
        }

        return params;
    }

    auto calculateAIC(const std::vector<double>& coeffs) -> double {
        int n = data_.size();
        int p = coeffs.size();
        double mse = calculateMSE(coeffs);
        double aic = n * std::log(mse) + 2 * p;
        return aic;
    }

    static auto calculateTValue(int /*degrees_of_freedom*/,
                                double confidence_level) -> double {
        if (confidence_level == 0.95) {
            return 1.96;
        }
        return 1.0;
    }

    auto calculateStandardError(const std::vector<double>& coeffs,
                                int /*index*/) -> double {
        double mse = calculateMSE(coeffs);
        return std::sqrt(mse);
    }

    static auto transpose(const std::vector<std::vector<double>>& A)
        -> std::vector<std::vector<double>> {
        int m = A.size();
        int n = A[0].size();
        std::vector<std::vector<double>> At(n, std::vector<double>(m));
        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < n; ++j) {
                At[j][i] = A[i][j];
            }
        }
        return At;
    }

    auto calculateMSE(const std::vector<double>& coeffs) -> double {
        double mse = 0.0;
        for (const auto& point : data_) {
            double predicted = evaluateCurve(coeffs, point.position);
            mse += std::pow(predicted - point.sharpness, 2);
        }
        return mse / data_.size();
    }

    static auto solveLinearSystem(std::vector<std::vector<double>> A,
                                  std::vector<double> b)
        -> std::vector<double> {
        int n = A.size();
        for (int i = 0; i < n; ++i) {
            int maxRow = i;
            for (int j = i + 1; j < n; ++j) {
                if (std::abs(A[j][i]) > std::abs(A[maxRow][i])) {
                    maxRow = j;
                }
            }
            std::swap(A[i], A[maxRow]);
            std::swap(b[i], b[maxRow]);

            for (int j = i + 1; j < n; ++j) {
                double factor = A[j][i] / A[i][i];
                for (int k = i; k < n; ++k) {
                    A[j][k] -= factor * A[i][k];
                }
                b[j] -= factor * b[i];
            }
        }

        std::vector<double> x(n);
        for (int i = n - 1; i >= 0; --i) {
            x[i] = b[i];
            for (int j = i + 1; j < n; ++j) {
                x[i] -= A[i][j] * x[j];
            }
            x[i] /= A[i][i];
        }
        return x;
    }

    auto evaluateCurve(const std::vector<double>& coeffs, double x) -> double {
        switch (current_model_) {
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
        return 0;
    }

    static auto evaluatePolynomial(const std::vector<double>& coeffs,
                                   double x) -> double {
        double result = 0.0;
        for (int i = 0; i < coeffs.size(); ++i) {
            result += coeffs[i] * std::pow(x, i);
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

// Constructor and Destructor for Pimpl pattern
FocusCurveFitter::FocusCurveFitter() : impl_(new Impl()) {}
FocusCurveFitter::~FocusCurveFitter() { delete impl_; }

// Public interface forwarding to the implementation
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
