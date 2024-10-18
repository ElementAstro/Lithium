/*!
 * \file ffi.hpp
 * \brief Enhanced FFI with Lazy Loading, Callbacks, and Timeout Mechanism
 * \author Max Qian <lightapt.com>
 * \date 2023-03-29, Updated 2024-10-14
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_FFI_HPP
#define ATOM_META_FFI_HPP

#include <any>
#include <cstdint>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>
#include <chrono>
#include <future>
#include <iostream>
#include <optional>

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

class FFIException : public atom::error::Exception {
public:
    using atom::error::Exception::Exception;
};

#define THROW_FFI_EXCEPTION(...) \
    throw FFIException(__FILE__, __LINE__, __func__, __VA_ARGS__)

namespace atom::meta {

// Logger for debug information
void log(const std::string& msg) {
    std::cerr << "[LOG] " << msg << std::endl;
}

template <typename T>
constexpr auto getFFIType() -> ffi_type* {
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
    } else if constexpr (std::is_same_v<T, void>) {
        return &ffi_type_void;
    } else if constexpr (std::is_class_v<T>) {
        // Define custom struct ffi_type here if T is a class/struct
        static ffi_type customStructType;
        // Assuming T has a static method to define the ffi_type layout
        customStructType = T::getFFITypeLayout();
        return &customStructType;
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

    auto call(void* funcPtr, Args... args) -> std::optional<ReturnType> {
        std::vector<void*> argsArray = {reinterpret_cast<void*>(&args)...};
        ReturnType result;

        if constexpr (std::is_same_v<ReturnType, void>) {
            ffi_call(&cif_, FFI_FN(funcPtr), nullptr, argsArray.data());
            return std::nullopt;
        } else {
            ffi_call(&cif_, FFI_FN(funcPtr), &result, argsArray.data());
            return result;
        }
    }

    // Overload with timeout
    auto callWithTimeout(void* funcPtr, std::chrono::milliseconds timeout, Args... args)
        -> std::optional<ReturnType> {
        auto future = std::async(std::launch::async, [this, funcPtr, args...] {
            return this->call(funcPtr, args...);
        });
        if (future.wait_for(timeout) == std::future_status::timeout) {
            THROW_FFI_EXCEPTION("Function call timed out.");
        }
        return future.get();
    }

private:
    ffi_cif cif_{};
    std::vector<ffi_type*> argTypes_;
    ffi_type* returnType_;
};

class DynamicLibrary {
public:
    explicit DynamicLibrary(std::string_view libraryPath, bool lazyLoad = false)
        : libraryPath_(libraryPath), isLazyLoad_(lazyLoad) {
        if (!isLazyLoad_) {
            loadLibrary();
        }
    }

    ~DynamicLibrary() { unloadLibrary(); }

    void loadLibrary() {
        if (handle_ != nullptr) {
            return; // Already loaded
        }

#ifdef _MSC_VER
        handle_ = LoadLibraryA(libraryPath_.data());
        if (handle_ == nullptr) {
            THROW_FFI_EXCEPTION("Failed to load dynamic library: " + std::string(libraryPath_));
        }
#else
        handle_ = dlopen(libraryPath_.data(), RTLD_LAZY);
        if (handle_ == nullptr) {
            std::string error = dlerror();
            THROW_FFI_EXCEPTION("Failed to load dynamic library: " + error);
        }
#endif
        log("Library loaded: " + std::string(libraryPath_));
    }

    void unloadLibrary() {
        if (handle_ != nullptr) {
#ifdef _MSC_VER
            FreeLibrary(static_cast<HMODULE>(handle_));
#else
            dlclose(handle_);
#endif
            log("Library unloaded.");
            handle_ = nullptr;
        }
        functionMap_.clear();
    }

    template <typename Func>
    auto getFunction(std::string_view functionName) -> std::function<Func> {
        if (isLazyLoad_ && handle_ == nullptr) {
            loadLibrary();
        }

        std::shared_lock lock(mutex_);
        if (handle_ == nullptr) {
            THROW_FFI_EXCEPTION("Library not loaded");
        }

#ifdef _MSC_VER
        FARPROC proc = GetProcAddress(static_cast<HMODULE>(handle_), functionName.data());
#else
        void* proc = dlsym(handle_, functionName.data());
#endif
        if (!proc) {
            std::string error = dlerror();
            THROW_FFI_EXCEPTION("Failed to load symbol: " + std::string(functionName) + " (" + error + ")");
        }
        log("Loaded function: " + std::string(functionName));
        return std::function<Func>(reinterpret_cast<Func*>(proc));
    }

    // Asynchronously call a function with timeout support
    template <typename ReturnType, typename... Args>
    auto callFunctionWithTimeout(std::string_view functionName, std::chrono::milliseconds timeout,     Args... args) -> std::optional<ReturnType> {
        if (isLazyLoad_ && handle_ == nullptr) {
            loadLibrary();
        }

        std::shared_lock lock(mutex_);
        auto findIt = functionMap_.find(std::string(functionName));
        if (findIt == functionMap_.end()) {
            log("Function not found in map: " + std::string(functionName));
            return std::nullopt;
        }

        void* funcPtr = findIt->second;
        FFIWrapper<ReturnType, Args...> ffiWrapper;

        // Call the function with the specified timeout
        return ffiWrapper.callWithTimeout(funcPtr, timeout, std::forward<Args>(args)...);
    }

    // Normal function call without timeout
    template <typename ReturnType, typename... Args>
    auto callFunction(std::string_view functionName, Args... args) -> std::optional<ReturnType> {
        if (isLazyLoad_ && handle_ == nullptr) {
            loadLibrary();
        }

        std::shared_lock lock(mutex_);
        auto findIt = functionMap_.find(std::string(functionName));
        if (findIt == functionMap_.end()) {
            log("Function not found in map: " + std::string(functionName));
            return std::nullopt;
        }

        void* funcPtr = findIt->second;
        FFIWrapper<ReturnType, Args...> ffiWrapper;

        return ffiWrapper.call(funcPtr, std::forward<Args>(args)...);
    }

    // Register a function in the function map for future calls
    template <typename FuncType>
    void addFunction(std::string_view functionName) {
        if (isLazyLoad_ && handle_ == nullptr) {
            loadLibrary();
        }

        std::unique_lock lock(mutex_);
#ifdef _MSC_VER
        void* funcPtr = reinterpret_cast<void*>(GetProcAddress(static_cast<HMODULE>(handle_), functionName.data()));
#else
        void* funcPtr = dlsym(handle_, functionName.data());
#endif
        if (!funcPtr) {
            THROW_FFI_EXCEPTION("Failed to find symbol: " + std::string(functionName));
        }
        functionMap_[std::string(functionName)] = funcPtr;
    }

    // Check if a function is loaded and present in the function map
    auto hasFunction(std::string_view functionName) const -> bool {
        std::shared_lock lock(mutex_);
        return functionMap_.contains(std::string(functionName));
    }

    // Reload the library
    void reload(const std::string& dllName) {
        std::unique_lock lock(mutex_);
        unloadLibrary();
        libraryPath_ = dllName;
        loadLibrary();
    }

    // Retrieve the dynamic library handle (for advanced users)
    auto getHandle() const -> void* {
        std::shared_lock lock(mutex_);
        return handle_;
    }

private:
    std::string libraryPath_;          // Store the path of the library
    bool isLazyLoad_ = false;          // Lazy loading flag
    void* handle_ = nullptr;           // Dynamic library handle
    mutable std::shared_mutex mutex_;  // Use shared_mutex for more efficient concurrency

    std::unordered_map<std::string, void*> functionMap_;  // Cache of loaded functions
};

class CallbackRegistry {
public:
    // Register a callback function that will be passed to an external library
    template <typename Func>
    void registerCallback(const std::string& callbackName, Func&& func) {
        std::unique_lock lock(mutex_);
        callbackMap_[callbackName] = std::make_any<std::function<Func>>(std::forward<Func>(func));
    }

    // Retrieve a registered callback
    template <typename Func>
    auto getCallback(const std::string& callbackName) -> std::function<Func>* {
        std::shared_lock lock(mutex_);
        auto it = callbackMap_.find(callbackName);
        if (it == callbackMap_.end()) {
            THROW_FFI_EXCEPTION("Callback not found: " + callbackName);
        }
        return std::any_cast<std::function<Func>>(&it->second);
    }

private:
    std::unordered_map<std::string, std::any> callbackMap_;  // Callback map
    mutable std::shared_mutex mutex_;
};

// Example of struct handling in FFI
struct MyStruct {
    int field1;
    double field2;

    static ffi_type* getFFITypeLayout() {
        static ffi_type* elements[] = {&ffi_type_sint, &ffi_type_double, nullptr};
        static ffi_type structType = {sizeof(MyStruct), alignof(MyStruct), FFI_TYPE_STRUCT, elements};
        return &structType;
    }
};

template <typename T>
class LibraryObject {
public:
    LibraryObject(DynamicLibrary& library, const std::string& factoryFuncName) {
        auto factory = library.getFunction<T*(void)>(factoryFuncName);
        if (!factory) {
            THROW_FFI_EXCEPTION("Failed to create object via factory function: " + factoryFuncName);
        }
        object_.reset(factory());
        log("Library object created.");
    }

    auto operator->() const -> T* { return object_.get(); }
    auto operator*() const -> T& { return *object_; }

private:
    std::unique_ptr<T> object_;
};

}  // namespace atom::meta

#endif  // ATOM_META_FFI_HPP
