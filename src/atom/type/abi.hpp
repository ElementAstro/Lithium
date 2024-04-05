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

#include <cxxabi.h>
#include <memory>
#include <string>
#include <typeinfo>
#include <vector>

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
    static std::string Demangle(const char* mangled_name) {
        int status = -1;
        std::unique_ptr<char, void (*)(void*)> demangled_name(
            abi::__cxa_demangle(mangled_name, nullptr, nullptr, &status),
            std::free);

        if (status == 0) {
            return std::string(demangled_name.get());
        } else {
            return std::string(mangled_name);
        }
    }
};

#endif
