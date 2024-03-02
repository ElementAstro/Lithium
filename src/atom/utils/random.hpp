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

#include <random>
#include <string>

namespace Atom::Utils {
template <class Engine = std::default_random_engine,
          class Distribution = std::uniform_int_distribution<>>
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
    template <typename... T>
    random(T &&...args)
        : base_t(std::forward<T>(args)...), engine_(std::random_device{}()) {}

    result_type operator()() { return static_cast<base_t &>(*this)(engine_); }

    result_type operator()(const param_type &parm) {
        return static_cast<base_t &>(*this)(engine_, parm);
    }
};

[[nodiscard]] std::string generateRandomString(int length);
}  // namespace Atom::Utils

#endif
