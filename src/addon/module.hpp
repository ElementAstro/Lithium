/*
 * module.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-8-6

Description: Module Information

**************************************************/

#pragma once

#include <atomic>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

// Determine the platform
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#elif defined(__linux__)
#define PLATFORM_LINUX
#include <dlfcn.h>
#elif defined(__APPLE__)
#define PLATFORM_MACOS
#include <dlfcn.h>
#endif

#include "atom/error/exception.hpp"

namespace lithium {
struct FunctionInfo {
    std::string name;
    void* address;
    std::vector<std::string> parameters;

    FunctionInfo() : name(""), address(nullptr) {}
};

class ModuleInfo {
    // All of the module information
public:
    std::string m_name;
    std::string m_description;
    std::string m_version;
    std::string m_status;
    std::string m_type;
    std::string m_author;
    std::string m_license;
    std::string m_path;
    std::string m_config_path;
    std::string m_config_file;

    // Module enable status
    std::atomic_bool m_enabled;

    // All of the functions in the module(dynamic loaded)
    std::vector<std::unique_ptr<FunctionInfo>> functions;

    // Module handle pointer
    void* handle;
};

class DynamicLibrary {
public:
    explicit DynamicLibrary(const std::string& dllName) {
        loadLibrary(dllName);
    }

    ~DynamicLibrary() { unloadLibrary(); }

    // Function to get a function pointer of any type using template and
    // variadic arguments
    template <typename Func>
    std::function<Func> getFunction(const std::string& funcName) {
        std::lock_guard lock(mutex);  // Ensure thread-safety
        if (hModule == nullptr) {
            THROW_NOT_FOUND("Module not loaded");
        }

#ifdef _WIN32
        FARPROC proc =
            GetProcAddress(static_cast<HMODULE>(hModule), funcName.c_str());
#else
        void* proc = dlsym(hModule, funcName.c_str());
#endif
        if (!proc) {
            THROW_FAIL_TO_LOAD_SYMBOL("Failed to load symbol: " + funcName);
        }
        // We use std::function to wrap the raw function pointer.
        return std::function<Func>(reinterpret_cast<Func*>(proc));
    }

    // Reload or load another library
    void reload(const std::string& dllName) {
        std::lock_guard lock(mutex);  // Ensure thread-safety
        unloadLibrary();
        loadLibrary(dllName);
    }

private:
    void* hModule = nullptr;
    std::mutex mutex;

    void loadLibrary(const std::string& dllName) {
#ifdef _WIN32
        hModule = LoadLibraryA(dllName.c_str());
#else
        hModule = dlopen(dllName.c_str(), RTLD_LAZY);
#endif
        if (!hModule) {
            THROW_FAIL_TO_LOAD_DLL("Failed to load " + dllName);
        }
    }

    void unloadLibrary() {
        if (hModule) {
#ifdef _WIN32
            FreeLibrary(static_cast<HMODULE>(hModule));
#else
            dlclose(hModule);
#endif
            hModule = nullptr;
        }
    }
};

template <typename T>
class LibraryObject {
public:
    LibraryObject(DynamicLibrary& library, const std::string& factoryFuncName) {
        auto factory = library.getFunction<T*(void)>(factoryFuncName);
        object.reset(factory());
    }

    T* operator->() const { return object.get(); }

    T& operator*() const { return *object; }

private:
    std::unique_ptr<T> object;
};
}  // namespace lithium
