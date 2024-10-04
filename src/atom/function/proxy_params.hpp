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

/**
 * @brief A class to encapsulate function parameters using std::any.
 */
class FunctionParams {
public:
    /**
     * @brief Constructs FunctionParams with a single std::any value.
     *
     * @param value The initial value to store in the parameters.
     */
    explicit FunctionParams(const std::any& value) : params_{value} {}

    /**
     * @brief Constructs FunctionParams from any range of std::any.
     *
     * @tparam Range The type of the range.
     * @param range The range of std::any values to initialize the parameters.
     */
    template <std::ranges::input_range Range>
        requires std::same_as<std::ranges::range_value_t<Range>, std::any>
    explicit constexpr FunctionParams(const Range& range)
        : params_(std::ranges::begin(range), std::ranges::end(range)) {}

    /**
     * @brief Constructs FunctionParams from an initializer list of std::any.
     *
     * @param ilist The initializer list of std::any values.
     */
    constexpr FunctionParams(std::initializer_list<std::any> ilist)
        : params_(ilist) {}

    /**
     * @brief Accesses the parameter at the given index.
     *
     * @param t_i The index of the parameter to access.
     * @return const std::any& The parameter at the given index.
     * @throws std::out_of_range if the index is out of range.
     */
    [[nodiscard]] auto operator[](std::size_t t_i) const -> const std::any& {
        if (t_i >= params_.size()) {
            THROW_OUT_OF_RANGE("Index out of range");
        }
        return params_.at(t_i);
    }

    /**
     * @brief Returns an iterator to the beginning of the parameters.
     *
     * @return auto An iterator to the beginning of the parameters.
     */
    [[nodiscard]] auto begin() const noexcept { return params_.begin(); }

    /**
     * @brief Returns an iterator to the end of the parameters.
     *
     * @return auto An iterator to the end of the parameters.
     */
    [[nodiscard]] auto end() const noexcept { return params_.end(); }

    /**
     * @brief Returns the first parameter.
     *
     * @return const std::any& The first parameter.
     */
    [[nodiscard]] auto front() const noexcept -> const std::any& {
        return params_.front();
    }

    /**
     * @brief Returns the number of parameters.
     *
     * @return std::size_t The number of parameters.
     */
    [[nodiscard]] auto size() const noexcept -> std::size_t {
        return params_.size();
    }

    /**
     * @brief Checks if there are no parameters.
     *
     * @return bool True if there are no parameters, false otherwise.
     */
    [[nodiscard]] auto empty() const noexcept -> bool {
        return params_.empty();
    }

    /**
     * @brief Converts the parameters to a vector of std::any.
     *
     * @return std::vector<std::any> The vector of parameters.
     */
    [[nodiscard]] auto toVector() const -> std::vector<std::any> {
        return params_;
    }

    /**
     * @brief Gets the parameter at the given index as a specific type.
     *
     * @tparam T The type to cast the parameter to.
     * @param index The index of the parameter to get.
     * @return std::optional<T> The parameter cast to the specified type, or
     * std::nullopt if the cast fails.
     */
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

    /**
     * @brief Slices the parameters from the given start index to the end index.
     *
     * @param start The start index of the slice.
     * @param end The end index of the slice.
     * @return FunctionParams The sliced parameters.
     * @throws std::out_of_range if the slice range is invalid.
     */
    [[nodiscard]] auto slice(std::size_t start,
                             std::size_t end) const -> FunctionParams {
        if (start > end || end > params_.size()) {
            THROW_OUT_OF_RANGE("Invalid slice range");
        }
        using DifferenceType = std::make_signed_t<std::size_t>;
        return FunctionParams(std::vector<std::any>(
            params_.begin() + static_cast<DifferenceType>(start),
            params_.begin() + static_cast<DifferenceType>(end)));
    }

    /**
     * @brief Filters the parameters using a predicate.
     *
     * @tparam Predicate The type of the predicate.
     * @param pred The predicate to filter the parameters.
     * @return FunctionParams The filtered parameters.
     */
    template <typename Predicate>
    [[nodiscard]] auto filter(Predicate pred) const -> FunctionParams {
        std::vector<std::any> filtered;
        std::ranges::copy_if(params_, std::back_inserter(filtered), pred);
        return FunctionParams(filtered);
    }

    /**
     * @brief Sets the parameter at the given index to a new value.
     *
     * @param index The index of the parameter to set.
     * @param value The new value to set.
     * @throws std::out_of_range if the index is out of range.
     */
    void set(std::size_t index, const std::any& value) {
        if (index >= params_.size()) {
            THROW_OUT_OF_RANGE("Index out of range");
        }
        params_[index] = value;
    }

private:
    std::vector<std::any> params_;  ///< The vector of parameters.
};

#endif