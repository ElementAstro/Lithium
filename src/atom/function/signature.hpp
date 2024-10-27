/*!
 * \file signature.hpp
 * \brief Signature parsing
 * \author Max Qian <lightapt.com>
 * \date 2024-6-7, Updated 2024-10-14
 */

#ifndef ATOM_META_SIGNATURE_HPP
#define ATOM_META_SIGNATURE_HPP

#include <optional>     // Includes for std::optional
#include <string_view>  // Includes for std::string_view
#include <vector>

#include "atom/utils/cstring.hpp"

namespace atom::meta {

struct alignas(128) FunctionSignature {
public:
    constexpr FunctionSignature(
        std::string_view name,
        std::vector<std::pair<std::string_view, std::string_view>> parameters,
        std::optional<std::string_view> returnType,
        std::optional<std::string_view> modifiers,
        std::optional<std::string_view> docComment)
        : name_(name),
          parameters_(std::move(parameters)),
          returnType_(returnType),
          modifiers_(modifiers),
          docComment_(docComment) {}

    [[nodiscard]] auto getName() const -> std::string_view { return name_; }

    [[nodiscard]] auto getParameters() const
        -> const std::vector<std::pair<std::string_view, std::string_view>>& {
        return parameters_;
    }

    [[nodiscard]] auto getReturnType() const
        -> std::optional<std::string_view> {
        return returnType_;
    }

    [[nodiscard]] auto getModifiers() const -> std::optional<std::string_view> {
        return modifiers_;
    }

    [[nodiscard]] auto getDocComment() const
        -> std::optional<std::string_view> {
        return docComment_;
    }

private:
    std::string_view name_;
    std::vector<std::pair<std::string_view, std::string_view>> parameters_;
    std::optional<std::string_view> returnType_;
    std::optional<std::string_view> modifiers_;
    std::optional<std::string_view> docComment_;
};

constexpr auto parseFunctionDefinition(
    const std::string_view DEFINITION) noexcept
    -> std::optional<FunctionSignature> {
    constexpr std::string_view DEF_PREFIX = "def ";
    constexpr std::string_view ARROW = " -> ";
    constexpr std::string_view CONST_MODIFIER = " const";
    constexpr std::string_view NOEXCEPT_MODIFIER = " noexcept";

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

    std::vector<std::pair<std::string_view, std::string_view>> parameters;
    size_t paramStart = 0;

    while (paramStart < params.size()) {
        size_t paramEnd = params.size();
        int bracketCount = 0;
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

        parameters.emplace_back(paramName, paramType);
        paramStart = paramEnd + 1;
    }

    std::optional<std::string_view> modifiers;
    if (DEFINITION.find(CONST_MODIFIER) != std::string_view::npos) {
        modifiers = CONST_MODIFIER;
    } else if (DEFINITION.find(NOEXCEPT_MODIFIER) != std::string_view::npos) {
        modifiers = NOEXCEPT_MODIFIER;
    }

    std::optional<std::string_view> docComment;
    size_t docStart = DEFINITION.find("/**");
    if (docStart != std::string_view::npos) {
        size_t docEnd = DEFINITION.find("*/", docStart);
        if (docEnd != std::string_view::npos) {
            docComment = DEFINITION.substr(docStart, docEnd - docStart + 2);
        }
    }

    return FunctionSignature{name, parameters, returnType, modifiers,
                             docComment};
}

}  // namespace atom::meta

#endif  // ATOM_META_SIGNATURE_HPP