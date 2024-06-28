/*!
 * \file proxy_params.hpp
 * \brief Proxy Function Params
 * \author Max Qian <lightapt.com>
 * \date 2024-03-01
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_PROXY_PARAMS_HPP
#define ATOM_META_PROXY_PARAMS_HPP

#include <algorithm>
#include <any>
#include <iterator>
#include <optional>
#include <ranges>
#include <vector>

#include "atom/error/exception.hpp"

class FunctionParams {
public:
    explicit FunctionParams(const std::any& bv) : params_{bv} {}

    // Generalizing to accept any range of std::any
    template <std::ranges::input_range Range>
        requires std::same_as<std::ranges::range_value_t<Range>, std::any>
    explicit constexpr FunctionParams(const Range& range)
        : params_(std::ranges::begin(range), std::ranges::end(range)) {}

    constexpr FunctionParams(std::initializer_list<std::any> ilist)
        : params_(ilist) {}

    [[nodiscard]] auto operator[](std::size_t t_i) const -> const std::any& {
        return params_.at(t_i);
    }

    [[nodiscard]] auto begin() const noexcept { return params_.begin(); }
    [[nodiscard]] auto end() const noexcept { return params_.end(); }
    [[nodiscard]] auto front() const noexcept -> const std::any& {
        return params_.front();
    }
    [[nodiscard]] auto size() const noexcept -> std::size_t {
        return params_.size();
    }
    [[nodiscard]] auto empty() const noexcept -> bool {
        return params_.empty();
    }

    [[nodiscard]] auto toVector() const -> std::vector<std::any> {
        return params_;
    }

    template <typename T>
    [[nodiscard]] auto get(std::size_t index) const -> std::optional<T> {
        if (index >= params_.size()) {
            return std::nullopt;
        }
        try {
            return std::any_cast<T>(params_[index]);
        } catch (const std::bad_any_cast&) {
            return std::nullopt;
        }
    }

    [[nodiscard]] auto slice(std::size_t start,
                             std::size_t end) const -> FunctionParams {
        if (start > end || end > params_.size()) {
            THROW_OUT_OF_RANGE("Invalid slice range");
        }
        return FunctionParams(std::vector<std::any>(params_.begin() + start,
                                                    params_.begin() + end));
    }

    template <typename Predicate>
    [[nodiscard]] FunctionParams filter(Predicate pred) const {
        std::vector<std::any> filtered;
        std::ranges::copy_if(params_, std::back_inserter(filtered), pred);
        return FunctionParams(filtered);
    }

    void set(std::size_t index, const std::any& value) {
        if (index >= params_.size()) {
            THROW_OUT_OF_RANGE("Index out of range");
        }
        params_[index] = value;
    }

private:
    std::vector<std::any> params_;
};

#endif
