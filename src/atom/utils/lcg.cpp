#include "lcg.hpp"

#include <cmath>
#include <fstream>
#include <numeric>
#include "exception.hpp"

namespace atom::utils {
LCG::LCG(result_type seed) : current_(seed) {}

auto LCG::next() -> result_type {
    constexpr result_type MULTIPLIER = 1664525;
    constexpr result_type INCREMENT = 1013904223;
    constexpr result_type MODULUS = 0xFFFFFFFF;  // 2^32

    std::lock_guard lock(mutex_);
    current_ = (MULTIPLIER * current_ + INCREMENT) % MODULUS;
    return current_;
}

void LCG::seed(result_type new_seed) {
    std::lock_guard lock(mutex_);
    current_ = new_seed;
}

void LCG::saveState(const std::string& filename) {
    std::lock_guard lock(mutex_);
    std::ofstream file(filename, std::ios::binary);
    file.write(reinterpret_cast<char*>(&current_), sizeof(current_));
}

void LCG::loadState(const std::string& filename) {
    std::lock_guard lock(mutex_);
    std::ifstream file(filename, std::ios::binary);
    file.read(reinterpret_cast<char*>(&current_), sizeof(current_));
}

auto LCG::nextInt(int min, int max) -> int {
    if (min > max) {
        THROW_INVALID_ARGUMENT("Min should be less than or equal to Max");
    }
    return min + static_cast<int>(next() % (max - min + 1));
}

auto LCG::nextDouble(double min, double max) -> double {
    if (min >= max) {
        THROW_INVALID_ARGUMENT("Min should be less than Max");
    }
    constexpr double MAX_UINT32 = 0xFFFFFFFF;
    return min + static_cast<double>(next()) / MAX_UINT32 * (max - min);
}

auto LCG::nextBernoulli(double probability) -> bool {
    if (probability < 0.0 || probability > 1.0) {
        THROW_INVALID_ARGUMENT("Probability should be in range [0, 1]");
    }
    return nextDouble() < probability;
}

auto LCG::nextGaussian(double mean, double stddev) -> double {
    static bool hasCachedValue = false;
    static double cachedValue;
    if (hasCachedValue) {
        hasCachedValue = false;
        return cachedValue * stddev + mean;
    }
    double uniform1 = nextDouble(0.0, 1.0);
    double uniform2 = nextDouble(0.0, 1.0);
    double radius = std::sqrt(-2.0 * std::log(uniform1));
    double theta = 2.0 * M_PI * uniform2;
    cachedValue = radius * std::sin(theta);
    hasCachedValue = true;
    return radius * std::cos(theta) * stddev + mean;
}

auto LCG::nextPoisson(double lambda) -> int {
    if (lambda <= 0.0) {
        THROW_INVALID_ARGUMENT("Lambda should be greater than 0");
    }
    double expLambda = std::exp(-lambda);
    int count = 0;
    double product = 1.0;
    do {
        ++count;
        product *= nextDouble();
    } while (product > expLambda);
    return count - 1;
}

auto LCG::nextExponential(double lambda) -> double {
    if (lambda <= 0.0) {
        THROW_INVALID_ARGUMENT("Lambda should be greater than 0");
    }
    return -std::log(1.0 - nextDouble()) / lambda;
}

auto LCG::nextGeometric(double probability) -> int {
    if (probability <= 0.0 || probability >= 1.0) {
        THROW_INVALID_ARGUMENT("Probability should be in range (0, 1)");
    }
    return static_cast<int>(std::ceil(std::log(1.0 - nextDouble()) / std::log(1.0 - probability)));
}

auto LCG::nextGamma(double shape, double scale) -> double {
    if (shape <= 0.0 || scale <= 0.0) {
        THROW_INVALID_ARGUMENT("Shape and scale must be greater than 0");
    }
    if (shape < 1.0) {
        return nextGamma(1.0 + shape, scale) * std::pow(nextDouble(), 1.0 / shape);
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
             std::log(nextDouble()) > 0.5 * x * x + d * (1.0 - v + std::log(v)));
    return d * v * scale;
}

auto LCG::nextBeta(double alpha, double beta) -> double {
    if (alpha <= 0.0 || beta <= 0.0) {
        THROW_INVALID_ARGUMENT("Alpha and Beta must be greater than 0");
    }
    double gammaAlpha = nextGamma(alpha, 1.0);
    double gammaBeta = nextGamma(beta, 1.0);
    return gammaAlpha / (gammaAlpha + gammaBeta);
}

auto LCG::nextChiSquared(double degreesOfFreedom) -> double {
    return nextGamma(degreesOfFreedom / 2.0, 2.0);
}

auto LCG::nextHypergeometric(int total, int success, int draws) -> int {
    if (success > total || draws > total || success < 0 || draws < 0) {
        THROW_INVALID_ARGUMENT("Invalid parameters for hypergeometric distribution");
    }

    int successCount = 0;
    int remainingSuccess = success;
    int remainingTotal = total;

    for (int i = 0; i < draws; ++i) {
        double probability = static_cast<double>(remainingSuccess) / remainingTotal;
        if (nextDouble(0.0, 1.0) < probability) {
            ++successCount;
            --remainingSuccess;
        }
        --remainingTotal;
    }
    return successCount;
}

auto LCG::nextDiscrete(const std::vector<double>& weights) -> int {
    double sum = std::accumulate(weights.begin(), weights.end(), 0.0);
    double randValue = nextDouble(0.0, sum);
    double cumulative = 0.0;
    for (size_t i = 0; i < weights.size(); ++i) {
        cumulative += weights[i];
        if (randValue < cumulative) {
            return static_cast<int>(i);
        }
    }
    return static_cast<int>(weights.size() - 1);
}

auto LCG::nextMultinomial(int trials, const std::vector<double>& probabilities) -> std::vector<int> {
    std::vector<int> counts(probabilities.size(), 0);
    for (int i = 0; i < trials; ++i) {
        int idx = nextDiscrete(probabilities);
        counts[idx]++;
    }
    return counts;
}

constexpr auto LCG::min() -> result_type {
    return 0;
}

constexpr auto LCG::max() -> result_type {
    return std::numeric_limits<result_type>::max();
}
}
