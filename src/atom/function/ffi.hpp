/*!
 * \file ffi.hpp
 * \brief FFI Function Interface
 * \author Max Qian <lightapt.com>
 * \date 2023-03-29
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_FFI_HPP
#define ATOM_META_FFI_HPP

#ifdef _MSC_VER
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#include <ffi.h>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <variant>
#include <vector>

#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

#include "atom/error/exception.hpp"
#include "atom/type/noncopyable.hpp"

class FFIException : atom::error::Exception {
public:
    using atom::error::Exception::Exception;
};

#define THROW_FFI_EXCEPTION(...) \
    throw FFIException(__FILE__, __LINE__, __func__, __VA_ARGS__)

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
    } else if constexpr (std::is_same_v<T, std::string>) {
        return &ffi_type_pointer;
    } else {
        static_assert(std::is_pointer_v<T>,
                      "Unsupported type passed to getFFIType.");
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
            THROW_FFI_EXCEPTION("Failed to prepare FFI call interface.");
        }
    }

    ReturnType call(void *funcPtr, Args... args) {
        std::vector<void *> argsArray = {reinterpret_cast<void *>(&args)...};
        ReturnType result;

        ffi_call(&cif_, FFI_FN(funcPtr), &result, argsArray.data());
        return result;
    }

private:
    ffi_cif cif_;
    std::vector<ffi_type *> argTypes_;
    ffi_type *returnType_;
};

class DynamicLibrary : public NonCopyable {
public:
    explicit DynamicLibrary(std::string_view libraryPath) : handle_(nullptr) {
#ifdef _MSC_VER
        handle_ = LoadLibraryA(libraryPath.data());
#else
        handle_ = dlopen(libraryPath.data(), RTLD_LAZY);
#endif
        if (!handle_) {
            THROW_FFI_EXCEPTION("Failed to load dynamic library.");
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
        functionMap_.clear();
    }

    template <typename FuncType>
    void addFunction(std::string_view functionName) {
#ifdef _MSC_VER
        void *funcPtr =
            (void *)GetProcAddress((HMODULE)handle_, functionName.data());
#else
        void *funcPtr = dlsym(handle_, functionName.data());
#endif
        if (!funcPtr) {
            THROW_FFI_EXCEPTION("Failed to find symbol.");
        }
        functionMap_[std::string(functionName)] = funcPtr;
    }

    template <typename ReturnType, typename... Args>
    std::optional<ReturnType> callFunction(std::string_view functionName,
                                           Args... args) {
        auto it = functionMap_.find(std::string(functionName));
        if (it == functionMap_.end()) {
            return std::nullopt;
        }

        void *funcPtr = it->second;
        FFIWrapper<ReturnType, Args...> ffiWrapper;

        return ffiWrapper.call(funcPtr, args...);
    }

private:
    void *handle_;
#if ENABLE_FASTHASH
    emhash8::HashMap<std::string, void *> functionMap_;
#else
    std::unordered_map<std::string, void *> functionMap_;
#endif
};

#endif
