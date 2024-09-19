/*!
 * \file abi.hpp
 * \brief A simple C++ ABI wrapper
 * \author Max Qian <lightapt.com>
 * \date 2024-5-25
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_ABI_HPP
#define ATOM_META_ABI_HPP

#include <array>
#include <optional>
#include <source_location>
#include <string>
#include <string_view>
#include <typeinfo>
#include <vector>

#ifdef _MSC_VER
// clang-format off
#include <windows.h>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")
// clang-format on
#else
#include <cxxabi.h>
#endif

#if ENABLE_DEBUG
#include <iostream>
#include <regex>
#endif

namespace atom::meta {

constexpr std::size_t BUFFER_SIZE = 1024;

class DemangleHelper {
public:
    template <typename T>
    static auto demangleType() -> std::string {
        return demangleInternal(typeid(T).name());
    }

    template <typename T>
    static auto demangleType(const T& instance) -> std::string {
        return demangleInternal(typeid(instance).name());
    }

    static auto demangle(std::string_view mangled_name,
                         const std::optional<std::source_location>& location =
                             std::nullopt) -> std::string {
        std::string demangled = demangleInternal(mangled_name);

        if (location) {
            demangled += " (";
            demangled += location->file_name();
            demangled += ":";
            demangled += std::to_string(location->line());
            demangled += ")";
        }

        return demangled;
    }

    static auto demangleMany(
        const std::vector<std::string_view>& mangled_names,
        const std::optional<std::source_location>& location = std::nullopt)
        -> std::vector<std::string> {
        std::vector<std::string> demangledNames;
        demangledNames.reserve(mangled_names.size());

        for (const auto& name : mangled_names) {
            demangledNames.push_back(demangle(name, location));
        }

        return demangledNames;
    }

#if ENABLE_DEBUG
    static auto visualize(const std::string& demangled_name) -> std::string {
        return visualizeType(demangled_name);
    }
#endif

private:
    static auto demangleInternal(std::string_view mangled_name) -> std::string {
#ifdef _MSC_VER
        std::array<char, BUFFER_SIZE> buffer;
        DWORD length = UnDecorateSymbolName(mangled_name.data(), buffer.data(),
                                            buffer.size(), UNDNAME_COMPLETE);

        return (length > 0) ? std::string(buffer.data(), length)
                            : std::string(mangled_name);
#else
        int status = -1;
        std::unique_ptr<char, void (*)(void*)> demangledName(
            abi::__cxa_demangle(mangled_name.data(), nullptr, nullptr, &status),
            std::free);

        return (status == 0) ? std::string(demangledName.get())
                             : std::string(mangled_name);
#endif
    }

#if ENABLE_DEBUG
    static auto visualizeType(const std::string& type_name,
                              int indent_level = 0) -> std::string {
        std::string indent(static_cast<long>(indent_level) * 4,
                           ' ');  // 4 spaces per indent level
        std::string result;

        // Regular expressions for parsing
        std::regex templateRegex(R"((\w+)<(.*)>)");
        std::regex functionRegex(R"(\((.*)\)\s*->\s*(.*))");
        std::regex ptrRegex(R"((.+)\s*\*\s*)");
        std::regex arrayRegex(R"((.+)\s*\[(\d+)\])");
        std::smatch match;

        if (std::regex_match(type_name, match, templateRegex)) {
            // Template type
            result += indent + "`-- " + match[1].str() + " [template]\n";
            std::string params = match[2].str();
            result += visualizeTemplateParams(params, indent_level + 1);
        } else if (std::regex_match(type_name, match, functionRegex)) {
            // Function type
            result += indent + "`-- function\n";
            std::string params = match[1].str();
            std::string returnType = match[2].str();
            result += visualizeFunctionParams(params, indent_level + 1);
            result += indent + "    `-- R: " +
                      visualizeType(returnType, indent_level + 1)
                          .substr(indent.size() + 4);
        } else if (std::regex_match(type_name, match, ptrRegex)) {
            // Pointer type
            result += indent + "`-- ptr\n";
            result += visualizeType(match[1].str(), indent_level + 1);
        } else if (std::regex_match(type_name, match, arrayRegex)) {
            // Array type
            result += indent + "`-- array [N = " + match[2].str() + "]\n";
            result += visualizeType(match[1].str(), indent_level + 1);
        } else {
            // Simple type
            result += indent + "`-- " + type_name + "\n";
        }

        return result;
    }

    static auto visualizeTemplateParams(const std::string& params,
                                        int indent_level) -> std::string {
        std::string indent(static_cast<long>(indent_level) * 4, ' ');
        std::string result;
        int paramIndex = 0;

        size_t start = 0;
        size_t end = 0;
        int angleBrackets = 0;

        for (size_t i = 0; i < params.size(); ++i) {
            if (params[i] == '<') {
                ++angleBrackets;
            } else if (params[i] == '>') {
                --angleBrackets;
            } else if (params[i] == ',' && angleBrackets == 0) {
                end = i;
                result += indent + "|-- " + std::to_string(paramIndex++) +
                          ": " +
                          visualizeType(params.substr(start, end - start),
                                        indent_level + 1)
                              .substr(indent.size() + 4);
                start = i + 1;
            }
        }

        result += indent + "|-- " + std::to_string(paramIndex++) + ": " +
                  visualizeType(params.substr(start), indent_level + 1)
                      .substr(indent.size() + 4);

        return result;
    }

    static auto visualizeFunctionParams(const std::string& params,
                                        int indent_level) -> std::string {
        std::string indent(static_cast<long>(indent_level) * 4, ' ');
        std::string result;
        int paramIndex = 0;

        size_t start = 0;
        size_t end = 0;
        int angleBrackets = 0;

        for (size_t i = 0; i < params.size(); ++i) {
            if (params[i] == '<') {
                ++angleBrackets;
            } else if (params[i] == '>') {
                --angleBrackets;
            } else if (params[i] == ',' && angleBrackets == 0) {
                end = i;
                result += indent + "|-- " + std::to_string(paramIndex++) +
                          ": " +
                          visualizeType(params.substr(start, end - start),
                                        indent_level + 1)
                              .substr(indent.size() + 4);
                start = i + 1;
            }
        }

        result += indent + "|-- " + std::to_string(paramIndex++) + ": " +
                  visualizeType(params.substr(start), indent_level + 1)
                      .substr(indent.size() + 4);

        return result;
    }
#endif
};
}  // namespace atom::meta

#endif  // ATOM_META_ABI_HPP
