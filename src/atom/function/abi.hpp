/*
 * abi.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-28

Description: A simple C++ ABI wrapper

**************************************************/

#ifndef ATOM_TYPE_ABI_HPP
#define ATOM_TYPE_ABI_HPP

#include <memory>
#include <string>
#include <string_view>
#include <typeinfo>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <dbghelp.h>
#if !defined(__MINGW32__) && !defined(__MINGW64__)
#pragma comment(lib, "dbghelp.lib")
#endif
#else
#include <cxxabi.h>
#endif

class DemangleHelper {
public:
    template <typename T>
    static std::string DemangleType() {
        return Demangle(typeid(T).name());
    }

    template <typename T>
    static std::string DemangleType(const T& instance) {
        return Demangle(typeid(instance).name());
    }

private:
    static std::string Demangle(std::string_view mangled_name) {
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

#endif
