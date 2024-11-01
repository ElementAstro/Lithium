#include "lcg.hpp"

#include <cmath>
#include <fstream>
#include <numeric>

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"

namespace atom::utils {

LCG::LCG(result_type seed) : current_(seed) {
    LOG_F(INFO, "LCG initialized with seed: %u", seed);
}

auto LCG::next() -> result_type {
    constexpr result_type MULTIPLIER = 1664525;
    constexpr result_type INCREMENT = 1013904223;
    constexpr result_type MODULUS = 0xFFFFFFFF;  // 2^32

    std::lock_guard lock(mutex_);
    current_ = (MULTIPLIER * current_ + INCREMENT) % MODULUS;
    LOG_F(INFO, "LCG generated next value: %u", current_);
    return current_;
}

void LCG::seed(result_type new_seed) {
    std::lock_guard lock(mutex_);
    current_ = new_seed;
    LOG_F(INFO, "LCG reseeded with new seed: %u", new_seed);
}

void LCG::saveState(const std::string& filename) {
    std::lock_guard lock(mutex_);
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        LOG_F(ERROR, "Failed to open file for saving state: %s",
              filename.c_str());
        THROW_RUNTIME_ERROR("Failed to open file for saving state");
    }
    file.write(reinterpret_cast<char*>(&current_), sizeof(current_));
    LOG_F(INFO, "LCG state saved to file: %s", filename.c_str());
}

void LCG::loadState(const std::string& filename) {
    std::lock_guard lock(mutex_);
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        LOG_F(ERROR, "Failed to open file for loading state: %s",
              filename.c_str());
        THROW_RUNTIME_ERROR("Failed to open file for loading state");
    }
    file.read(reinterpret_cast<char*>(&current_), sizeof(current_));
    LOG_F(INFO, "LCG state loaded from file: %s", filename.c_str());
}

auto LCG::nextInt(int min, int max) -> int {
    if (min > max) {
        LOG_F(ERROR, "Invalid argument: min (%d) > max (%d)", min, max);
        THROW_INVALID_ARGUMENT("Min should be less than or equal to Max");
    }
    int result = min + static_cast<int>(next() % (max - min + 1));
    LOG_F(INFO, "LCG generated next int: %d (range: [%d, %d])", result, min,
          max);
    return result;
}

auto LCG::nextDouble(double min, double max) -> double {
    if (min >= max) {
        LOG_F(ERROR, "Invalid argument: min (%f) >= max (%f)", min, max);
        THROW_INVALID_ARGUMENT("Min should be less than Max");
    }
    constexpr double MAX_UINT32 = 0xFFFFFFFF;
    double result =
        min + static_cast<double>(next()) / MAX_UINT32 * (max - min);
    LOG_F(INFO, "LCG generated next double: %f (range: [%f, %f])", result, min,
          max);
    return result;
}

auto LCG::nextBernoulli(double probability) -> bool {
    if (probability < 0.0 || probability > 1.0) {
        LOG_F(ERROR, "Invalid argument: probability (%f) out of range [0, 1]",
              probability);
        THROW_INVALID_ARGUMENT("Probability should be in range [0, 1]");
    }
    bool result = nextDouble() < probability;
    LOG_F(INFO, "LCG generated next Bernoulli: %s (probability: %f)",
          result ? "true" : "false", probability);
    return result;
}

auto LCG::nextGaussian(double mean, double stddev) -> double {
    static bool hasCachedValue = false;
    static double cachedValue;
    if (hasCachedValue) {
        hasCachedValue = false;
        double result = cachedValue * stddev + mean;
        LOG_F(INFO,
              "LCG generated next Gaussian (cached): %f (mean: %f, stddev: %f)",
              result, mean, stddev);
        return result;
    }
    double uniform1 = nextDouble(0.0, 1.0);
    double uniform2 = nextDouble(0.0, 1.0);
    double radius = std::sqrt(-2.0 * std::log(uniform1));
    double theta = 2.0 * M_PI * uniform2;
    cachedValue = radius * std::sin(theta);
    hasCachedValue = true;
    double result = radius * std::cos(theta) * stddev + mean;
    LOG_F(INFO, "LCG generated next Gaussian: %f (mean: %f, stddev: %f)",
          result, mean, stddev);
    return result;
}

auto LCG::nextPoisson(double lambda) -> int {
    if (lambda <= 0.0) {
        LOG_F(ERROR, "Invalid argument: lambda (%f) <= 0", lambda);
        THROW_INVALID_ARGUMENT("Lambda should be greater than 0");
    }
    double expLambda = std::exp(-lambda);
    int count = 0;
    double product = 1.0;
    do {
        ++count;
        product *= nextDouble();
    } while (product > expLambda);
    int result = count - 1;
    LOG_F(INFO, "LCG generated next Poisson: %d (lambda: %f)", result, lambda);
    return result;
}

auto LCG::nextExponential(double lambda) -> double {
    if (lambda <= 0.0) {
        LOG_F(ERROR, "Invalid argument: lambda (%f) <= 0", lambda);
        THROW_INVALID_ARGUMENT("Lambda should be greater than 0");
    }
    double result = -std::log(1.0 - nextDouble()) / lambda;
    LOG_F(INFO, "LCG generated next Exponential: %f (lambda: %f)", result,
          lambda);
    return result;
}

auto LCG::nextGeometric(double probability) -> int {
    if (probability <= 0.0 || probability >= 1.0) {
        LOG_F(ERROR, "Invalid argument: probability (%f) out of range (0, 1)",
              probability);
        THROW_INVALID_ARGUMENT("Probability should be in range (0, 1)");
    }
    int result = static_cast<int>(
        std::ceil(std::log(1.0 - nextDouble()) / std::log(1.0 - probability)));
    LOG_F(INFO, "LCG generated next Geometric: %d (probability: %f)", result,
          probability);
    return result;
}

auto LCG::nextGamma(double shape, double scale) -> double {
    if (shape <= 0.0 || scale <= 0.0) {
        LOG_F(ERROR, "Invalid argument: shape (%f) <= 0 or scale (%f) <= 0",
              shape, scale);
        THROW_INVALID_ARGUMENT("Shape and scale must be greater than 0");
    }
    if (shape < 1.0) {
        double result =
            nextGamma(1.0 + shape, scale) * std::pow(nextDouble(), 1.0 / shape);
        LOG_F(INFO,
              "LCG generated next Gamma (shape < 1): %f (shape: %f, scale: %f)",
              result, shape, scale);
        return result;
    }
    constexpr double MAGIC_NUMBER_3 = 3.0;
    constexpr double MAGIC_NUMBER_9 = 9.0;
    constexpr double MAGIC_NUMBER_0_0331 = 0.0331;
    double d = shape - 1.0 / MAGIC_NUMBER_3;
    double c = 1.0 / std::sqrt(MAGIC_NUMBER_9 * d);
    double x;
    double v;
    do {
        do {
            x = nextGaussian(0.0, 1.0);
            v = 1.0 + c * x;
        } while (v <= 0);
        v = v * v * v;
    } while (nextDouble() > (1.0 - MAGIC_NUMBER_0_0331 * (x * x) * (x * x)) &&
             std::log(nextDouble()) >
                 0.5 * x * x + d * (1.0 - v + std::log(v)));
    double result = d * v * scale;
    LOG_F(INFO, "LCG generated next Gamma: %f (shape: %f, scale: %f)", result,
          shape, scale);
    return result;
}

auto LCG::nextBeta(double alpha, double beta) -> double {
    if (alpha <= 0.0 || beta <= 0.0) {
        LOG_F(ERROR, "Invalid argument: alpha (%f) <= 0 or beta (%f) <= 0",
              alpha, beta);
        THROW_INVALID_ARGUMENT("Alpha and Beta must be greater than 0");
    }
    double gammaAlpha = nextGamma(alpha, 1.0);
    double gammaBeta = nextGamma(beta, 1.0);
    double result = gammaAlpha / (gammaAlpha + gammaBeta);
    LOG_F(INFO, "LCG generated next Beta: %f (alpha: %f, beta: %f)", result,
          alpha, beta);
    return result;
}

auto LCG::nextChiSquared(double degreesOfFreedom) -> double {
    double result = nextGamma(degreesOfFreedom / 2.0, 2.0);
    LOG_F(INFO, "LCG generated next Chi-Squared: %f (degrees of freedom: %f)",
          result, degreesOfFreedom);
    return result;
}

auto LCG::nextHypergeometric(int total, int success, int draws) -> int {
    if (success > total || draws > total || success < 0 || draws < 0) {
        LOG_F(ERROR,
              "Invalid parameters for hypergeometric distribution: total (%d), "
              "success (%d), draws (%d)",
              total, success, draws);
        THROW_INVALID_ARGUMENT(
            "Invalid parameters for hypergeometric distribution");
    }

    int successCount = 0;
    int remainingSuccess = success;
    int remainingTotal = total;

    for (int i = 0; i < draws; ++i) {
        double probability =
            static_cast<double>(remainingSuccess) / remainingTotal;
        if (nextDouble(0.0, 1.0) < probability) {
            ++successCount;
            --remainingSuccess;
        }
        --remainingTotal;
    }
    LOG_F(INFO,
          "LCG generated next Hypergeometric: %d (total: %d, success: %d, "
          "draws: %d)",
          successCount, total, success, draws);
    return successCount;
}

auto LCG::nextDiscrete(const std::vector<double>& weights) -> int {
    double sum = std::accumulate(weights.begin(), weights.end(), 0.0);
    double randValue = nextDouble(0.0, sum);
    double cumulative = 0.0;
    for (size_t i = 0; i < weights.size(); ++i) {
        cumulative += weights[i];
        if (randValue < cumulative) {
            LOG_F(INFO, "LCG generated next Discrete: %d (weights index: %zu)",
                  static_cast<int>(i), i);
            return static_cast<int>(i);
        }
    }
    LOG_F(INFO, "LCG generated next Discrete: %d (weights index: %zu)",
          static_cast<int>(weights.size() - 1), weights.size() - 1);
    return static_cast<int>(weights.size() - 1);
}

auto LCG::nextMultinomial(int trials, const std::vector<double>& probabilities)
    -> std::vector<int> {
    std::vector<int> counts(probabilities.size(), 0);
    for (int i = 0; i < trials; ++i) {
        int idx = nextDiscrete(probabilities);
        counts[idx]++;
    }
    LOG_F(
        INFO,
        "LCG generated next Multinomial: trials (%d), probabilities size (%zu)",
        trials, probabilities.size());
    return counts;
}

constexpr auto LCG::min() -> result_type { return 0; }

constexpr auto LCG::max() -> result_type {
    return std::numeric_limits<result_type>::max();
}

}  // namespace atom::utils
