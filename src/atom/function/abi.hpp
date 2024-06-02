/*!
 * \file abi.hpp
 * \brief A simple C++ ABI wrapper
 * \author Max Qian <lightapt.com>
 * \date 2024-5-25
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_ABI_HPP
#define ATOM_META_ABI_HPP

#include <cctype>
#include <memory>
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
class DemangleHelper {
public:
    template <typename T>
    static std::string DemangleType() {
        return DemangleInternal(typeid(T).name());
    }

    template <typename T>
    static std::string DemangleType(const T& instance) {
        return DemangleInternal(typeid(instance).name());
    }

    static std::string Demangle(
        std::string_view mangled_name,
        const std::optional<std::source_location>& location = std::nullopt) {
        std::string demangled = DemangleInternal(mangled_name);

        if (location) {
            demangled += " (";
            demangled += location->file_name();
            demangled += ":";
            demangled += std::to_string(location->line());
            demangled += ")";
        }

        return demangled;
    }

    static std::vector<std::string> DemangleMany(
        const std::vector<std::string_view>& mangled_names,
        const std::optional<std::source_location>& location = std::nullopt) {
        std::vector<std::string> demangled_names;
        demangled_names.reserve(mangled_names.size());

        for (const auto& name : mangled_names) {
            demangled_names.push_back(Demangle(name, location));
        }

        return demangled_names;
    }

#if ENABLE_DEBUG
    static std::string Visualize(const std::string& demangled_name) {
        return VisualizeType(demangled_name);
    }
#endif

private:
    static std::string DemangleInternal(std::string_view mangled_name) {
#ifdef _MSC_VER
        char buffer[1024];
        DWORD length = UnDecorateSymbolName(mangled_name.data(), buffer,
                                            sizeof(buffer), UNDNAME_COMPLETE);

        return (length > 0) ? std::string(buffer, length)
                            : std::string(mangled_name);
#else
        int status = -1;
        std::unique_ptr<char, void (*)(void*)> demangled_name(
            abi::__cxa_demangle(mangled_name.data(), nullptr, nullptr, &status),
            std::free);

        return (status == 0) ? std::string(demangled_name.get())
                             : std::string(mangled_name);
#endif
    }

#if ENABLE_DEBUG
    static std::string VisualizeType(const std::string& type_name,
                                     int indent_level = 0) {
        std::string indent(indent_level * 4, ' ');  // 4 spaces per indent level
        std::string result;

        // Regular expressions for parsing
        std::regex template_regex(R"((\w+)<(.*)>)");
        std::regex function_regex(R"(\((.*)\)\s*->\s*(.*))");
        std::regex ptr_regex(R"((.+)\s*\*\s*)");
        std::regex array_regex(R"((.+)\s*\[(\d+)\])");
        std::smatch match;

        if (std::regex_match(type_name, match, template_regex)) {
            // Template type
            result += indent + "`-- " + match[1].str() + " [template]\n";
            std::string params = match[2].str();
            result += VisualizeTemplateParams(params, indent_level + 1);
        } else if (std::regex_match(type_name, match, function_regex)) {
            // Function type
            result += indent + "`-- function\n";
            std::string params = match[1].str();
            std::string return_type = match[2].str();
            result += VisualizeFunctionParams(params, indent_level + 1);
            result += indent + "    `-- R: " +
                      VisualizeType(return_type, indent_level + 1)
                          .substr(indent.size() + 4);
        } else if (std::regex_match(type_name, match, ptr_regex)) {
            // Pointer type
            result += indent + "`-- ptr\n";
            result += VisualizeType(match[1].str(), indent_level + 1);
        } else if (std::regex_match(type_name, match, array_regex)) {
            // Array type
            result += indent + "`-- array [N = " + match[2].str() + "]\n";
            result += VisualizeType(match[1].str(), indent_level + 1);
        } else {
            // Simple type
            result += indent + "`-- " + type_name + "\n";
        }

        return result;
    }

    static std::string VisualizeTemplateParams(const std::string& params,
                                               int indent_level) {
        std::string indent(indent_level * 4, ' ');
        std::string result;
        int param_index = 0;

        size_t start = 0;
        size_t end = 0;
        int angle_brackets = 0;

        for (size_t i = 0; i < params.size(); ++i) {
            if (params[i] == '<') {
                ++angle_brackets;
            } else if (params[i] == '>') {
                --angle_brackets;
            } else if (params[i] == ',' && angle_brackets == 0) {
                end = i;
                result += indent + "|-- " + std::to_string(param_index++) +
                          ": " +
                          VisualizeType(params.substr(start, end - start),
                                        indent_level + 1)
                              .substr(indent.size() + 4);
                start = i + 1;
            }
        }

        result += indent + "|-- " + std::to_string(param_index++) + ": " +
                  VisualizeType(params.substr(start), indent_level + 1)
                      .substr(indent.size() + 4);

        return result;
    }

    static std::string VisualizeFunctionParams(const std::string& params,
                                               int indent_level) {
        std::string indent(indent_level * 4, ' ');
        std::string result;
        int param_index = 0;

        size_t start = 0;
        size_t end = 0;
        int angle_brackets = 0;

        for (size_t i = 0; i < params.size(); ++i) {
            if (params[i] == '<') {
                ++angle_brackets;
            } else if (params[i] == '>') {
                --angle_brackets;
            } else if (params[i] == ',' && angle_brackets == 0) {
                end = i;
                result += indent + "|-- " + std::to_string(param_index++) +
                          ": " +
                          VisualizeType(params.substr(start, end - start),
                                        indent_level + 1)
                              .substr(indent.size() + 4);
                start = i + 1;
            }
        }

        result += indent + "|-- " + std::to_string(param_index++) + ": " +
                  VisualizeType(params.substr(start), indent_level + 1)
                      .substr(indent.size() + 4);

        return result;
    }
#endif
};
}  // namespace atom::meta

#endif  // ATOM_META_ABI_HPP