/*
 * random.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-25

Description: Simple random number generator

**************************************************/

#ifndef ATOM_UTILS_RANDOM_HPP
#define ATOM_UTILS_RANDOM_HPP

#include <algorithm>
#include <concepts>
#include <iterator>
#include <random>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace atom::utils {
/**
 * @brief A template class that combines a random number engine and a
 * distribution.
 *
 * @tparam Engine A type that meets the requirements of
 * UniformRandomBitGenerator (e.g., std::mt19937).
 * @tparam Distribution A type of distribution (e.g.,
 * std::uniform_int_distribution).
 */
template <std::uniform_random_bit_generator Engine, typename Distribution>
class Random : public Distribution {
    using BaseT = Distribution;  ///< Base type alias for the distribution.

public:
    using EngineType = Engine;  ///< Public type alias for the engine.
    using DistributionType =
        Distribution;  ///< Public type alias for the distribution.
    using ResultType =
        typename DistributionType::result_type;  ///< Result type produced by
                                                 ///< the distribution.
    using ParamType =
        typename DistributionType::param_type;  ///< Parameter type for the
                                                ///< distribution.

private:
    EngineType engine_;  ///< Instance of the engine.

public:
    /**
     * @brief Constructs a Random object with specified minimum and maximum
     * values for the distribution.
     *
     * @param min The minimum value of the distribution.
     * @param max The maximum value of the distribution.
     */
    Random(ResultType min, ResultType max)
        : BaseT(min, max), engine_(std::random_device{}()) {}

    /**
     * @brief Constructs a Random object with a seed and distribution
     * parameters.
     *
     * @param seed A seed value to initialize the engine.
     * @param args Arguments to initialize the distribution.
     */
    template <typename Seed, typename... Args>
    Random(Seed&& seed, Args&&... args)
        : BaseT(std::forward<Args>(args)...),
          engine_(std::forward<Seed>(seed)) {}

    /**
     * @brief Re-seeds the engine.
     *
     * @param value The seed value (default is obtained from
     * std::random_device).
     */
    void seed(ResultType value = std::random_device{}()) {
        engine_.seed(value);
    }

    /**
     * @brief Generates a random value using the underlying distribution and
     * engine.
     *
     * @return A randomly generated value.
     */
    ResultType operator()() { return BaseT::operator()(engine_); }

    /**
     * @brief Generates a random value using the underlying distribution and
     * engine, with specific parameters.
     *
     * @param parm Parameters for the distribution.
     * @return A randomly generated value.
     */
    ResultType operator()(const ParamType& parm) {
        return BaseT::operator()(engine_, parm);
    }

    /**
     * @brief Fills a range with randomly generated values.
     *
     * @param first An iterator pointing to the beginning of the range.
     * @param last An iterator pointing to the end of the range.
     */
    template <typename OutputIt>
    void generate(OutputIt first, OutputIt last) {
        std::generate(first, last, [this]() { return (*this)(); });
    }

    /**
     * @brief Creates a vector of randomly generated values.
     *
     * @param count The number of values to generate.
     * @return A vector containing randomly generated values.
     */
    std::vector<ResultType> vector(size_t count) {
        std::vector<ResultType> vec;
        vec.reserve(count);
        std::generate_n(std::back_inserter(vec), count,
                        [this]() { return (*this)(); });
        return vec;
    }

    /**
     * @brief Sets parameters for the distribution.
     *
     * @param parm The new parameters for the distribution.
     */
    void param(const ParamType& parm) { BaseT::param(parm); }

    /**
     * @brief Accessor for the underlying engine.
     *
     * @return A reference to the engine.
     */
    EngineType& engine() { return engine_; }
};

[[nodiscard]] std::string generateRandomString(int length);
}  // namespace atom::utils

#endif
