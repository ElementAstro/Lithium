/*!
 * \file signature.hpp
 * \brief Signature parsing
 * \author Max Qian <lightapt.com>
 * \date 2024-6-7
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_SIGNATURE_HPP
#define ATOM_META_SIGNATURE_HPP

#include <optional>
#include <regex>
#include <string>
#include <vector>

#if ENABLE_DEBUG
#include <iostream>
#endif

namespace atom::meta {

struct FunctionSignature {
    std::string name;
    std::vector<std::pair<std::string, std::string>> parameters;
    std::optional<std::string> returnType;

#if ENABLE_DEBUG
    void print() const {
        std::cout << "Function Name: " << name << "\n";
        std::cout << "Parameters:\n";
        for (const auto& [paramName, paramType] : parameters) {
            std::cout << "  " << paramName << ": " << paramType << "\n";
        }
        if (returnType.has_value()) {
            std::cout << "Returns: " << returnType.value() << "\n";
        } else {
            std::cout << "Returns: None\n";
        }
    }
#endif
};

std::optional<FunctionSignature> parseFunctionDefinition(
    const std::string& definition) {
    std::regex pattern(R"(def\s+(\w+)\((.*?)\)\s*->\s*(\w+))");
    std::smatch matches;

    if (std::regex_search(definition, matches, pattern)) {
        FunctionSignature signature;
        signature.name = matches[1];

        std::string params = matches[2];
        std::regex paramPattern(R"((\w+)\s*:\s*(\w+))");
        std::smatch paramMatches;
        std::string::const_iterator searchStart(params.cbegin());

        while (std::regex_search(searchStart, params.cend(), paramMatches,
                                 paramPattern)) {
            signature.parameters.emplace_back(paramMatches[1], paramMatches[2]);
            searchStart = paramMatches.suffix().first;
        }

        signature.returnType = matches[3];

        return signature;
    }

    return std::nullopt;
}

}  // namespace atom::meta

#endif