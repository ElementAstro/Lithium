/*!
 * \file abi.hpp
 * \brief A simple C++ ABI wrapper
 * \author Max Qian <lightapt.com>
 * \date 2024-5-25
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_ABI_HPP
#define ATOM_META_ABI_HPP

#include <memory>
#include <optional>
#include <source_location>
#include <string>
#include <string_view>
#include <typeinfo>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <dbghelp.h>
#ifdef _MSC_VER
#pragma comment(lib, "dbghelp.lib")
#endif
#else
#include <cxxabi.h>
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

        // If source location information is provided, append it to the
        // demangled name.
        if (location.has_value()) {
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

private:
    static std::string DemangleInternal(std::string_view mangled_name) {
#ifdef _WIN32
        char buffer[1024];
        DWORD length = UnDecorateSymbolName(mangled_name.data(), buffer,
                                            sizeof(buffer), UNDNAME_COMPLETE);

        if (length > 0) {
            return std::string(buffer, length);
        } else {
            return std::string(mangled_name);
        }
#else
        int status = -1;
        std::size_t length = 0;
        std::unique_ptr<char, void (*)(void*)> demangled_name(
            abi::__cxa_demangle(mangled_name.data(), nullptr, &length, &status),
            std::free);

        if (status == 0) {
            return std::string(demangled_name.get(), length);
        } else {
            return std::string(mangled_name);
        }
#endif
    }
};
}  // namespace atom::meta

#endif  // ATOM_META_ABI_HPP
