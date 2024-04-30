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
#include <random>
#include <string>
#include <vector>

namespace atom::utils {
template <std::uniform_random_bit_generator Engine, typename Distribution>
class random : public Distribution {
    using base_t = Distribution;

public:
    using engine_type = Engine;
    using distribution_type = Distribution;
    using result_type = typename distribution_type::result_type;
    using param_type = typename distribution_type::param_type;

private:
    engine_type engine_;

public:
    random(result_type min, result_type max)
        : base_t(min, max), engine_(std::random_device{}()) {}

    template <typename Seed, typename... Args>
    random(Seed&& seed, Args&&... args)
        : base_t(std::forward<Args>(args)...),
          engine_(std::forward<Seed>(seed)) {}

    void seed(result_type value = std::random_device{}()) {
        engine_.seed(value);
    }

    result_type operator()() {
        return base_t::operator()(engine_, base_t::param());
    }

    result_type operator()(const param_type& parm) {
        return base_t::operator()(engine_, parm);
    }

    template <typename OutputIt>
    void generate(OutputIt first, OutputIt last) {
        std::generate(first, last, *this);
    }

    std::vector<result_type> vector(size_t count) {
        std::vector<result_type> vec(count);
        generate(vec.begin(), vec.end());
        return vec;
    }
};

[[nodiscard]] std::string generateRandomString(int length);
}  // namespace atom::utils

#endif
