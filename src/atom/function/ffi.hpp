/*!
 * \file ffi.hpp
 * \brief FFI Function Interface
 * \author Max Qian <lightapt.com>
 * \date 2023-03-29
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_FFI_HPP
#define ATOM_META_FFI_HPP

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#ifdef _MSC_VER
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <ffi.h>

#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

#include "atom/error/exception.hpp"
#include "atom/type/noncopyable.hpp"

class FFIException : public atom::error::Exception {
public:
    using atom::error::Exception::Exception;
};

#define THROW_FFI_EXCEPTION(...) \
    throw FFIException(__FILE__, __LINE__, __func__, __VA_ARGS__)

template <typename T>
constexpr ffi_type* getFFIType() {
    if constexpr (std::is_same_v<T, int>) {
        return &ffi_type_sint;
    } else if constexpr (std::is_same_v<T, float>) {
        return &ffi_type_float;
    } else if constexpr (std::is_same_v<T, double>) {
        return &ffi_type_double;
    } else if constexpr (std::is_same_v<T, uint8_t>) {
        return &ffi_type_uint8;
    } else if constexpr (std::is_same_v<T, uint16_t>) {
        return &ffi_type_uint16;
    } else if constexpr (std::is_same_v<T, uint32_t>) {
        return &ffi_type_uint32;
    } else if constexpr (std::is_same_v<T, uint64_t>) {
        return &ffi_type_uint64;
    } else if constexpr (std::is_same_v<T, int8_t>) {
        return &ffi_type_sint8;
    } else if constexpr (std::is_same_v<T, int16_t>) {
        return &ffi_type_sint16;
    } else if constexpr (std::is_same_v<T, int32_t>) {
        return &ffi_type_sint32;
    } else if constexpr (std::is_same_v<T, const char*> ||
                         std::is_same_v<T, std::string> ||
                         std::is_same_v<T, std::string_view>) {
        return &ffi_type_pointer;
    } else if constexpr (std::is_pointer_v<T>) {
        return &ffi_type_pointer;
    } else {
        static_assert(false, "Unsupported type passed to getFFIType.");
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

    auto call(void* funcPtr, Args... args) -> ReturnType {
        std::vector<void*> argsArray = {reinterpret_cast<void*>(&args)...};
        ReturnType result;

        ffi_call(&cif_, FFI_FN(funcPtr), &result, argsArray.data());
        return result;
    }

private:
    ffi_cif cif_{};
    std::vector<ffi_type*> argTypes_;
    ffi_type* returnType_;
};

class DynamicLibrary : public NonCopyable {
public:
    explicit DynamicLibrary(std::string_view libraryPath) {
#ifdef _MSC_VER
        handle_ = LoadLibraryA(libraryPath.data());
#else
        handle_ = dlopen(libraryPath.data(), RTLD_LAZY);
#endif
        if (handle_ == nullptr) {
            THROW_FFI_EXCEPTION("Failed to load dynamic library.");
        }
    }

    ~DynamicLibrary() {
        if (handle_ != nullptr) {
#ifdef _MSC_VER
            FreeLibrary(static_cast<HMODULE>(handle_));
#else
            dlclose(handle_);
#endif
        }
        functionMap_.clear();
    }

    template <typename FuncType>
    void addFunction(std::string_view functionName) {
#ifdef _MSC_VER
        void* funcPtr = reinterpret_cast<void*>(
            GetProcAddress(static_cast<HMODULE>(handle_), functionName.data()));
#else
        void* funcPtr = dlsym(handle_, functionName.data());
#endif
        if (!funcPtr) {
            THROW_FFI_EXCEPTION("Failed to find symbol.");
        }
        functionMap_[std::string(functionName)] = funcPtr;
    }

    template <typename ReturnType, typename... Args>
    auto callFunction(std::string_view functionName,
                      Args... args) -> std::optional<ReturnType> {
        auto findIt = functionMap_.find(std::string(functionName));
        if (findIt == functionMap_.end()) {
            return std::nullopt;
        }

        void* funcPtr = findIt->second;
        FFIWrapper<ReturnType, Args...> ffiWrapper;

        return ffiWrapper.call(funcPtr, std::forward<Args>(args)...);
    }

    // New methods
    auto hasFunction(std::string_view functionName) const -> bool {
        return functionMap_.find(std::string(functionName)) !=
               functionMap_.end();
    }

    template <typename Func>
    auto getFunction(std::string_view functionName) -> std::function<Func> {
        auto findIt = functionMap_.find(std::string(functionName));
        if (findIt == functionMap_.end()) {
            THROW_FFI_EXCEPTION("Function not found.");
        }
        return reinterpret_cast<Func*>(findIt->second);
    }

    template <typename ReturnType, typename... Args>
    auto bindFunction(std::string_view functionName)
        -> std::function<ReturnType(Args...)> {
        return [this, functionName](Args... args) -> ReturnType {
            auto result = this->callFunction<ReturnType, Args...>(
                functionName, std::forward<Args>(args)...);
            if (!result) {
                THROW_FFI_EXCEPTION("Failed to call function.");
            }
            return *result;
        };
    }

private:
    void* handle_{};
#if ENABLE_FASTHASH
    emhash8::HashMap<std::string, void*> functionMap_;
#else
    std::unordered_map<std::string, void*> functionMap_;
#endif
};

#endif
