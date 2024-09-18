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
#include <optional>
#include <string_view>
#include <utility>

#include "atom/utils/cstring.hpp"
#include "macro.hpp"

namespace atom::meta {
struct alignas(128) FunctionSignature {
public:
    constexpr FunctionSignature(
        std::string_view name,
        std::array<std::pair<std::string_view, std::string_view>, 2> parameters,
        std::optional<std::string_view> returnType)
        : name_(name),
          parameters_(std::move(parameters)),
          returnType_(returnType) {}

    [[nodiscard]] auto getName() const -> std::string_view { return name_; }

    [[nodiscard]] auto getParameters() const
        -> const std::array<std::pair<std::string_view, std::string_view>, 2>& {
        return parameters_;
    }

    [[nodiscard]] auto getReturnType() const
        -> std::optional<std::string_view> {
        return returnType_;
    }

private:
    std::string_view name_;
    std::array<std::pair<std::string_view, std::string_view>, 2> parameters_;
    std::optional<std::string_view> returnType_;
};

constexpr auto parseFunctionDefinition(
    const std::string_view DEFINITION) noexcept
    -> std::optional<FunctionSignature> {
    constexpr std::string_view DEF_PREFIX = "def ";
    constexpr std::string_view ARROW = " -> ";

    if (DEFINITION.substr(0, DEF_PREFIX.size()) != DEF_PREFIX) {
        return std::nullopt;
    }

    size_t nameStart = DEF_PREFIX.size();
    size_t nameEnd = DEFINITION.find('(', nameStart);
    if (nameEnd == std::string_view::npos || nameEnd == nameStart) {
        return std::nullopt;  // No function name present
    }

    std::string_view name = DEFINITION.substr(nameStart, nameEnd - nameStart);
    size_t paramsStart = nameEnd + 1;
    size_t paramsEnd = DEFINITION.find(')', paramsStart);
    if (paramsEnd == std::string_view::npos) {
        return std::nullopt;
    }

    std::string_view params =
        DEFINITION.substr(paramsStart, paramsEnd - paramsStart);
    size_t arrowPos = DEFINITION.find(ARROW, paramsEnd + 1);
    std::optional<std::string_view> returnType = "none";
    if (arrowPos != std::string_view::npos) {
        returnType = DEFINITION.substr(arrowPos + ARROW.size());
    }

    std::array<std::pair<std::string_view, std::string_view>, 2> parameters{};
    size_t paramStart = 0;
    size_t paramIndex = 0;

    while (paramStart < params.size() && paramIndex < parameters.size()) {
        size_t paramEnd = params.size();
        int bracketCount = 0;
#pragma unroll
        for (size_t i = paramStart; i < params.size(); ++i) {
            if (params[i] == ',' && bracketCount == 0) {
                paramEnd = i;
                break;
            }
            if (params[i] == '[') {
                ++bracketCount;
            }
            if (params[i] == ']') {
                --bracketCount;
            }
        }

        std::string_view param =
            params.substr(paramStart, paramEnd - paramStart);
        size_t colonPos = param.find(':');
        std::string_view paramName;
        std::string_view paramType = "any";  // Default type

        if (colonPos != std::string_view::npos) {
            paramName = atom::utils::trim(param.substr(0, colonPos));
            paramType = atom::utils::trim(param.substr(colonPos + 1));
        } else {
            paramName = atom::utils::trim(param);
        }

        parameters[paramIndex++] = {paramName, paramType};
        paramStart = paramEnd + 1;
    }

    return FunctionSignature{name, parameters, returnType};
}

}  // namespace atom::meta

#endif
