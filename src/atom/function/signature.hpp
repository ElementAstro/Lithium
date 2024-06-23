/*!
 * \file signature.hpp
 * \brief Signature parsing
 * \author Max Qian <lightapt.com>
 * \date 2024-6-7
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_SIGNATURE_HPP
#define ATOM_META_SIGNATURE_HPP

#include <array>
#include <cctype>
#include <optional>
#include <string_view>
#include <tuple>
#include <utility>

#include "atom/utils/cstring.hpp"

namespace atom::meta {
struct FunctionSignature {
    constexpr FunctionSignature(
        std::string_view name,
        std::array<std::pair<std::string_view, std::string_view>, 2> parameters,
        std::optional<std::string_view> returnType)
        : name(name), parameters(parameters), returnType(returnType) {}

    std::string_view name;
    std::array<std::pair<std::string_view, std::string_view>, 2> parameters;
    std::optional<std::string_view> returnType;
};

constexpr std::optional<FunctionSignature> parseFunctionDefinition(
    const std::string_view definition) noexcept {
    constexpr std::string_view def_prefix = "def ";
    constexpr std::string_view arrow = " -> ";

    if (definition.substr(0, def_prefix.size()) != def_prefix) {
        return std::nullopt;
    }

    size_t name_start = def_prefix.size();
    size_t name_end = definition.find('(', name_start);
    if (name_end == std::string_view::npos) {
        return std::nullopt;
    }

    std::string_view name =
        definition.substr(name_start, name_end - name_start);
    size_t params_start = name_end + 1;
    size_t params_end = definition.find(')', params_start);
    if (params_end == std::string_view::npos) {
        return std::nullopt;
    }

    std::string_view params =
        definition.substr(params_start, params_end - params_start);
    size_t arrow_pos = definition.find(arrow, params_end + 1);
    std::optional<std::string_view> returnType;
    if (arrow_pos != std::string_view::npos) {
        returnType = definition.substr(arrow_pos + arrow.size());
    }

    std::array<std::pair<std::string_view, std::string_view>, 2> parameters{};
    size_t param_start = 0;
    size_t param_index = 0;

    while (param_start < params.size() && param_index < parameters.size()) {
        size_t param_end = params.size();
        int bracket_count = 0;
        for (size_t i = param_start; i < params.size(); ++i) {
            if (params[i] == ',' && bracket_count == 0) {
                param_end = i;
                break;
            }
            if (params[i] == '[')
                ++bracket_count;
            if (params[i] == ']')
                --bracket_count;
        }

        std::string_view param =
            params.substr(param_start, param_end - param_start);
        size_t colon_pos = param.find(':');
        if (colon_pos != std::string_view::npos) {
            std::string_view paramName =
                atom::utils::trim(param.substr(0, colon_pos));
            std::string_view paramType =
                atom::utils::trim(param.substr(colon_pos + 1));
            parameters[param_index++] = {paramName, paramType};
        }

        param_start = param_end + 1;
    }

    return FunctionSignature{name, parameters, returnType};
}

}  // namespace atom::meta

#endif