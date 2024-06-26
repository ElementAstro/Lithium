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

class FunctionParams {
public:
    explicit FunctionParams(const std::any& bv) : m_params{bv} {}

    // Generalizing to accept any range of std::any
    template <std::ranges::input_range Range>
        requires std::same_as<std::ranges::range_value_t<Range>, std::any>
    explicit constexpr FunctionParams(const Range& range)
        : m_params(std::ranges::begin(range), std::ranges::end(range)) {}

    explicit constexpr FunctionParams(std::initializer_list<std::any> ilist)
        : m_params(ilist) {}

    [[nodiscard]] const std::any& operator[](std::size_t t_i) const {
        return m_params.at(t_i);
    }

    [[nodiscard]] auto begin() const noexcept { return m_params.begin(); }
    [[nodiscard]] auto end() const noexcept { return m_params.end(); }
    [[nodiscard]] const std::any& front() const noexcept {
        return m_params.front();
    }
    [[nodiscard]] std::size_t size() const noexcept { return m_params.size(); }
    [[nodiscard]] bool empty() const noexcept { return m_params.empty(); }

    [[nodiscard]] std::vector<std::any> to_vector() const { return m_params; }

    template <typename T>
    [[nodiscard]] std::optional<T> get(std::size_t index) const {
        if (index >= m_params.size()) {
            return std::nullopt;
        }
        try {
            return std::any_cast<T>(m_params[index]);
        } catch (const std::bad_any_cast&) {
            return std::nullopt;
        }
    }

    [[nodiscard]] FunctionParams slice(std::size_t start,
                                       std::size_t end) const {
        if (start > end || end > m_params.size()) {
            throw std::out_of_range("Invalid slice range");
        }
        return FunctionParams(std::vector<std::any>(m_params.begin() + start,
                                                    m_params.begin() + end));
    }

    template <typename Predicate>
    [[nodiscard]] FunctionParams filter(Predicate pred) const {
        std::vector<std::any> filtered;
        std::ranges::copy_if(m_params, std::back_inserter(filtered), pred);
        return FunctionParams(filtered);
    }

    void set(std::size_t index, const std::any& value) {
        if (index >= m_params.size()) {
            throw std::out_of_range("Index out of range");
        }
        m_params[index] = value;
    }

private:
    std::vector<std::any> m_params;
};

#endif
