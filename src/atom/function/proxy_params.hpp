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
#include "atom/type/json.hpp"
using json = nlohmann::json;

namespace atom::meta {
class Arg {
public:
    Arg();
    explicit Arg(std::string name);
    Arg(std::string name, std::any default_value);

    [[nodiscard]] auto getName() const -> const std::string&;
    [[nodiscard]] auto getDefaultValue() const
        -> const std::optional<std::any>&;

private:
    std::string name_;
    std::optional<std::any> default_value_;
};

inline Arg::Arg() = default;
inline Arg::Arg(std::string name) : name_(std::move(name)) {}

inline Arg::Arg(std::string name, std::any default_value)
    : name_(std::move(name)), default_value_(std::move(default_value)) {}

inline auto Arg::getName() const -> const std::string& { return name_; }

inline auto Arg::getDefaultValue() const -> const std::optional<std::any>& {
    return default_value_;
}

// Serialize std::any
inline void to_json(nlohmann::json& j, const std::any& a) {
    if (a.type() == typeid(int)) {
        j = std::any_cast<int>(a);
    } else if (a.type() == typeid(float)) {
        j = std::any_cast<float>(a);
    } else if (a.type() == typeid(double)) {
        j = std::any_cast<double>(a);
    } else if (a.type() == typeid(std::string)) {
        j = std::any_cast<std::string>(a);
    } else if (a.type() == typeid(std::string_view)) {
        j = std::any_cast<std::string_view>(a);
    } else if (a.type() == typeid(const char *)) {
        j = std::any_cast<const char *>(a);
    } else {
        throw std::runtime_error("Unsupported type");
    }
}

// Deserialize std::any
inline void from_json(const nlohmann::json& j, std::any& a) {
    if (j.is_number_integer()) {
        a = j.get<int>();
    } else if (j.is_number_float()) {
        a = j.get<double>();
    } else if (j.is_string()) {
        a = j.get<std::string>();
    } else {
        throw std::runtime_error("Unsupported type");
    }
}

// Serialize Arg
inline void to_json(nlohmann::json& j, const Arg& arg) {
    j = nlohmann::json{{"name", arg.getName()}};
    if (arg.getDefaultValue()) {
        to_json(j["default_value"], *arg.getDefaultValue());
    } else {
        j["default_value"] = nullptr;
    }
}

// Deserialize Arg
inline void from_json(const nlohmann::json& j, Arg& arg) {
    // Get the name field
    std::string name = j.at("name").get<std::string>();

    // Handle the default_value field
    std::optional<std::any> defaultValue;
    if (!j.at("default_value").is_null()) {
        std::any value;
        from_json(j.at("default_value"), value);  // Use std::any's from_json
        defaultValue = value;
    }

    // Create a new Arg instance using the deserialized values
    arg = Arg(std::move(name), defaultValue ? *defaultValue : std::any());
}

// Serialize vector of Arg
inline void to_json(nlohmann::json& j, const std::vector<Arg>& arg) {
    for (const auto& a : arg) {
        j.push_back(a);
    }
}

// Deserialize vector of Arg
inline void from_json(const nlohmann::json& j, std::vector<Arg>& arg) {
    for (const auto& a : j) {
        Arg temp;
        from_json(a, temp);
        arg.push_back(temp);
    }
}

/**
 * @brief A class to encapsulate function parameters using Arg objects.
 */
class FunctionParams {
public:
    /**
     * @brief Default constructor for FunctionParams.
     */
    FunctionParams() = default;
    /**
     * @brief Constructs FunctionParams with a single Arg value.
     *
     * @param arg The initial Arg to store in the parameters.
     */
    explicit FunctionParams(const Arg& arg) : params_{arg} {}

    /**
     * @brief Constructs FunctionParams from any range of Arg.
     *
     * @tparam Range The type of the range.
     * @param range The range of Arg values to initialize the parameters.
     */
    template <std::ranges::input_range Range>
        requires std::same_as<std::ranges::range_value_t<Range>, Arg>
    explicit constexpr FunctionParams(const Range& range)
        : params_(std::ranges::begin(range), std::ranges::end(range)) {}

    /**
     * @brief Constructs FunctionParams from an initializer list of Arg.
     *
     * @param ilist The initializer list of Arg values.
     */
    constexpr FunctionParams(std::initializer_list<Arg> ilist)
        : params_(ilist) {}

    /**
     * @brief Accesses the parameter at the given index.
     *
     * @param t_i The index of the parameter to access.
     * @return const Arg& The parameter at the given index.
     * @throws std::out_of_range if the index is out of range.
     */
    [[nodiscard]] auto operator[](std::size_t t_i) const -> const Arg& {
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
     * @return const Arg& The first parameter.
     */
    [[nodiscard]] auto front() const noexcept -> const Arg& {
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
     * @brief Converts the parameters to a vector of Arg.
     *
     * @return std::vector<Arg> The vector of parameters.
     */
    [[nodiscard]] auto toVector() const -> std::vector<Arg> { return params_; }

    [[nodiscard]] auto toAnyVector() const -> std::vector<std::any> {
        std::vector<std::any> anyVec;
        anyVec.reserve(params_.size());
        std::ranges::transform(
            params_, std::back_inserter(anyVec),
            [](const Arg& arg) { return arg.getDefaultValue(); });
        return anyVec;
    }

    /**
     * @brief Gets the parameter at the given index by name.
     *
     * @param name The name of the parameter to get.
     * @return std::optional<Arg> The parameter if found, std::nullopt
     * otherwise.
     */
    [[nodiscard]] auto getByName(const std::string& name) const
        -> std::optional<Arg> {
        if (auto findTt = std::ranges::find_if(
                params_, [&](const Arg& arg) { return arg.getName() == name; });
            findTt != params_.end()) {
            return *findTt;
        }
        return std::nullopt;
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
        return FunctionParams(std::vector<Arg>(
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
        std::vector<Arg> filtered;
        std::ranges::copy_if(params_, std::back_inserter(filtered), pred);
        return FunctionParams(filtered);
    }

    /**
     * @brief Sets the parameter at the given index to a new Arg.
     *
     * @param index The index of the parameter to set.
     * @param arg The new Arg to set.
     * @throws std::out_of_range if the index is out of range.
     */
    void set(std::size_t index, const Arg& arg) {
        if (index >= params_.size()) {
            THROW_OUT_OF_RANGE("Index out of range");
        }
        params_[index] = arg;
    }

private:
    std::vector<Arg> params_;  ///< The vector of Arg objects.
};
}  // namespace atom::meta

#endif
