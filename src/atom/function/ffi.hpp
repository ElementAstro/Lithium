/*
 * ffi.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: FFI Function

**************************************************/

#ifndef ATOM_FUNCTION_FFI_HPP
#define ATOM_FUNCTION_FFI_HPP

#ifdef _MSC_VER
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#include <ffi.h>
#include <memory>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "atom/error/exception.hpp"
#include "atom/type/noncopyable.hpp"

class FFIException : public std::exception {
public:
    explicit FFIException(const std::string &message) : message_(message) {}
    const char *what() const noexcept override { return message_.c_str(); }

private:
    std::string message_;
};

template <typename T>
constexpr ffi_type *getFFIType() {
    if constexpr (std::is_same_v<T, int>) {
        return &ffi_type_sint;
    } else if constexpr (std::is_same_v<T, float>) {
        return &ffi_type_float;
    } else if constexpr (std::is_same_v<T, double>) {
        return &ffi_type_double;
    } else if constexpr (std::is_same_v<T, const char *>) {
        return &ffi_type_pointer;
    } else {
        return &ffi_type_pointer;
    }
}

template <typename ReturnType, typename... Args>
class FFIWrapper {
public:
    FFIWrapper() {
        argTypes_ = {getFFIType<Args>()...};
        returnType_ = getFFIType<ReturnType>();

        if (ffi_prep_cif(&cif_, FFI_DEFAULT_ABI, sizeof...(Args), returnType_,
                         argTypes_.data()) != FFI_OK) {
            throw FFIException("Failed to prepare FFI call interface.");
        }
    }

    ReturnType call(void *funcPtr, Args... args) {
        void *argsArray[] = {&args...};
        ReturnType result;

        ffi_call(&cif_, FFI_FN(funcPtr), &result, argsArray);
        return result;
    }

private:
    ffi_cif cif_;
    std::vector<ffi_type *> argTypes_;
    ffi_type *returnType_;
};

class DynamicLibrary : public NonCopyable {
public:
    explicit DynamicLibrary(const std::string &libraryPath) : handle_(nullptr) {
#ifdef _MSC_VER
        handle_ = LoadLibrary(libraryPath.c_str());
#else
        handle_ = dlopen(libraryPath.c_str(), RTLD_LAZY);
#endif
        if (!handle_) {
            throw FFIException("Failed to load dynamic library.");
        }
    }

    ~DynamicLibrary() {
        if (handle_) {
#ifdef _MSC_VER
            FreeLibrary((HMODULE)handle_);
#else
            dlclose(handle_);
#endif
        }
    }

    template <typename FuncType>
    void addFunction(const std::string &functionName) {
#ifdef _MSC_VER
        void *funcPtr =
            (void *)GetProcAddress((HMODULE)handle_, functionName.c_str());
#else
        void *funcPtr = dlsym(handle_, functionName.c_str());
#endif
        if (!funcPtr) {
            throw FFIException("Failed to find symbol.");
        }
        functionMap_[functionName] = funcPtr;
    }

    template <typename ReturnType, typename... Args>
    ReturnType callFunction(const std::string &functionName, Args... args) {
        auto it = functionMap_.find(functionName);
        if (it == functionMap_.end()) {
            throw FFIException("Function not found in the library.");
        }

        void *funcPtr = it->second;
        FFIWrapper<ReturnType, Args...> ffiWrapper;

        return ffiWrapper.call(funcPtr, args...);
    }

private:
    void *handle_;
    std::unordered_map<std::string, void *> functionMap_;
};

#endif
