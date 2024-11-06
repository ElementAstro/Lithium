#ifndef ATOM_UTILS_LCG_HPP
#define ATOM_UTILS_LCG_HPP

#include <chrono>
#include <limits>
#include <mutex>
#include <vector>

#include "atom/error/exception.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace atom::utils {
/**
 * @class LCG
 * @brief Linear Congruential Generator for pseudo-random number generation.
 *
 * This class implements a Linear Congruential Generator (LCG) which is a type
 * of pseudo-random number generator. It provides various methods to generate
 * random numbers following different distributions.
 */
class LCG {
public:
    using result_type = uint32_t;

    /**
     * @brief Constructs an LCG with an optional seed.
     * @param seed The initial seed value. Defaults to the current time since
     * epoch.
     */
    explicit LCG(
        result_type seed = static_cast<result_type>(
            std::chrono::steady_clock::now().time_since_epoch().count()));

    /**
     * @brief Generates the next random number in the sequence.
     * @return The next random number.
     */
    auto next() -> result_type;

    /**
     * @brief Seeds the generator with a new seed value.
     * @param new_seed The new seed value.
     */
    void seed(result_type new_seed);

    /**
     * @brief Saves the current state of the generator to a file.
     * @param filename The name of the file to save the state to.
     */
    void saveState(const std::string& filename);

    /**
     * @brief Loads the state of the generator from a file.
     * @param filename The name of the file to load the state from.
     */
    void loadState(const std::string& filename);

    /**
     * @brief Generates a random integer within a specified range.
     * @param min The minimum value (inclusive). Defaults to 0.
     * @param max The maximum value (inclusive). Defaults to the maximum value
     * of int.
     * @return A random integer within the specified range.
     */
    auto nextInt(int min = 0, int max = std::numeric_limits<int>::max()) -> int;

    /**
     * @brief Generates a random double within a specified range.
     * @param min The minimum value (inclusive). Defaults to 0.0.
     * @param max The maximum value (exclusive). Defaults to 1.0.
     * @return A random double within the specified range.
     */
    auto nextDouble(double min = 0.0, double max = 1.0) -> double;

    /**
     * @brief Generates a random boolean value based on a specified probability.
     * @param probability The probability of returning true. Defaults to 0.5.
     * @return A random boolean value.
     */
    auto nextBernoulli(double probability = 0.5) -> bool;

    /**
     * @brief Generates a random number following a Gaussian (normal)
     * distribution.
     * @param mean The mean of the distribution. Defaults to 0.0.
     * @param stddev The standard deviation of the distribution. Defaults
     * to 1.0.
     * @return A random number following a Gaussian distribution.
     */
    auto nextGaussian(double mean = 0.0, double stddev = 1.0) -> double;

    /**
     * @brief Generates a random number following a Poisson distribution.
     * @param lambda The rate parameter (lambda) of the distribution. Defaults
     * to 1.0.
     * @return A random number following a Poisson distribution.
     */
    auto nextPoisson(double lambda = 1.0) -> int;

    /**
     * @brief Generates a random number following an Exponential distribution.
     * @param lambda The rate parameter (lambda) of the distribution. Defaults
     * to 1.0.
     * @return A random number following an Exponential distribution.
     */
    auto nextExponential(double lambda = 1.0) -> double;

    /**
     * @brief Generates a random number following a Geometric distribution.
     * @param probability The probability of success in each trial. Defaults to
     * 0.5.
     * @return A random number following a Geometric distribution.
     */
    auto nextGeometric(double probability = 0.5) -> int;

    /**
     * @brief Generates a random number following a Gamma distribution.
     * @param shape The shape parameter of the distribution.
     * @param scale The scale parameter of the distribution. Defaults to 1.0.
     * @return A random number following a Gamma distribution.
     */
    auto nextGamma(double shape, double scale = 1.0) -> double;

    /**
     * @brief Generates a random number following a Beta distribution.
     * @param alpha The alpha parameter of the distribution.
     * @param beta The beta parameter of the distribution.
     * @return A random number following a Beta distribution.
     */
    auto nextBeta(double alpha, double beta) -> double;

    /**
     * @brief Generates a random number following a Chi-Squared distribution.
     * @param degreesOfFreedom The degrees of freedom of the distribution.
     * @return A random number following a Chi-Squared distribution.
     */
    auto nextChiSquared(double degreesOfFreedom) -> double;

    /**
     * @brief Generates a random number following a Hypergeometric distribution.
     * @param total The total number of items.
     * @param success The number of successful items.
     * @param draws The number of draws.
     * @return A random number following a Hypergeometric distribution.
     */
    auto nextHypergeometric(int total, int success, int draws) -> int;

    /**
     * @brief Generates a random index based on a discrete distribution.
     * @param weights The weights of the discrete distribution.
     * @return A random index based on the weights.
     */
    auto nextDiscrete(const std::vector<double>& weights) -> int;

    /**
     * @brief Generates a multinomial distribution.
     * @param trials The number of trials.
     * @param probabilities The probabilities of each outcome.
     * @return A vector of counts for each outcome.
     */
    auto nextMultinomial(int trials, const std::vector<double>& probabilities)
        -> std::vector<int>;

    /**
     * @brief Shuffles a vector of data.
     * @tparam T The type of the elements in the vector.
     * @param data The vector of data to shuffle.
     */
    template <typename T>
    void shuffle(std::vector<T>& data);

    /**
     * @brief Samples a subset of data from a vector.
     * @tparam T The type of the elements in the vector.
     * @param data The vector of data to sample from.
     * @param sampleSize The number of elements to sample.
     * @return A vector containing the sampled elements.
     */
    template <typename T>
    auto sample(const std::vector<T>& data, int sampleSize) -> std::vector<T>;

    /**
     * @brief Returns the minimum value that can be generated.
     * @return The minimum value.
     */
    static constexpr auto min() -> result_type { return 0; }

    /**
     * @brief Returns the maximum value that can be generated.
     * @return The maximum value.
     */
    static constexpr auto max() -> result_type {
        return std::numeric_limits<result_type>::max();
    }

private:
    result_type current_;  ///< The current state of the generator.
    std::mutex mutex_;     ///< Mutex for thread-safe operations.
};

template <typename T>
void LCG::shuffle(std::vector<T>& data) {
    std::lock_guard lock(mutex_);
    for (size_t i = data.size() - 1; i > 0; --i) {
        std::swap(data[i], data[nextInt(0, static_cast<int>(i))]);
    }
}

template <typename T>
auto LCG::sample(const std::vector<T>& data, int sampleSize) -> std::vector<T> {
    if (sampleSize > static_cast<int>(data.size())) {
        THROW_INVALID_ARGUMENT(
            "Sample size cannot be greater than the size of the input data");
    }
    std::vector<T> result = data;
    shuffle(result);
    return std::vector<T>(result.begin(), result.begin() + sampleSize);
}
}  // namespace atom::utils

#endif  // ATOM_UTILS_LCG_HPP
