/*
 * env.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-16

Description: Environment variable management

**************************************************/

#include "env.hpp"

#include <algorithm>
#include <cstdlib>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#endif

#include "atom/log/loguru.hpp"
#include "atom/utils/argsview.hpp"

namespace fs = std::filesystem;

namespace atom::utils {
class Env::Impl {
public:
    std::string mExe;      ///< Full path of the executable file.
    std::string mCwd;      ///< Working directory.
    std::string mProgram;  ///< Program name.

    std::unordered_map<std::string, std::string>
        mArgs;                  ///< List of command-line arguments.
    mutable std::mutex mMutex;  ///< Mutex to protect member variables.
    ArgumentParser mParser;     ///< Argument parser for command-line arguments.
};

Env::Env() : Env(0, nullptr) { LOG_F(INFO, "Env default constructor called"); }

Env::Env(int argc, char **argv) {
    LOG_F(INFO, "Env constructor called with argc: {}, argv: {}", argc, argv);
    fs::path exePath;

#ifdef _WIN32
    wchar_t buf[MAX_PATH];
    if (GetModuleFileNameW(nullptr, buf, MAX_PATH) == 0U) {
        LOG_F(ERROR, "GetModuleFileNameW failed with error {}", GetLastError());
        exePath = buf;
    } else {
        exePath = buf;
    }
#else
    char linkBuf[1024];
    ssize_t count = readlink("/proc/self/exe", linkBuf, sizeof(linkBuf));
    if (count != -1) {
        linkBuf[count] = '\0';
        exePath = linkBuf;
    }
#endif

    impl_->mExe = exePath.string();
    impl_->mCwd = exePath.parent_path().string() + '/';
    impl_->mProgram = argv[0];

    LOG_F(INFO, "Executable path: {}", impl_->mExe);
    LOG_F(INFO, "Current working directory: {}", impl_->mCwd);
    LOG_F(INFO, "Program name: {}", impl_->mProgram);

    if (argc > 1) {
        int i = 1;
        int j;
        for (j = 2; j < argc; ++j) {
            if (argv[i][0] == '-' && argv[j][0] == '-') {
                add(std::string(argv[i] + 1), "");
                i = j;
            } else if (argv[i][0] == '-' && argv[j][0] != '-') {
                add(std::string(argv[i] + 1), std::string(argv[j]));
                ++j;
                i = j;
            } else {
                return;
            }
        }

        if (i < argc) {
            if (argv[i][0] == '-') {
                add(std::string(argv[i] + 1), "");
            } else {
                return;
            }
        }
    }
    LOG_F(INFO, "Env constructor completed");
}

auto Env::createShared(int argc, char **argv) -> std::shared_ptr<Env> {
    LOG_F(INFO, "Env::createShared called with argc: {}, argv: {}", argc, argv);
    return std::make_shared<Env>(argc, argv);
}

void Env::add(const std::string &key, const std::string &val) {
    LOG_F(INFO, "Env::add called with key: {}, val: {}", key, val);
    std::lock_guard lock(impl_->mMutex);
    if (has(key)) {
        LOG_F(ERROR, "Env::add: Duplicate key: {}", key);
    } else {
        DLOG_F(INFO, "Env::add: Add key: {} with value: {}", key, val);
        impl_->mArgs[key] = val;
    }
}

auto Env::has(const std::string &key) -> bool {
    LOG_F(INFO, "Env::has called with key: {}", key);
    std::lock_guard lock(impl_->mMutex);
    bool result = impl_->mArgs.contains(key);
    LOG_F(INFO, "Env::has returning: {}", result);
    return result;
}

void Env::del(const std::string &key) {
    LOG_F(INFO, "Env::del called with key: {}", key);
    std::lock_guard lock(impl_->mMutex);
    impl_->mArgs.erase(key);
    DLOG_F(INFO, "Env::del: Remove key: {}", key);
}

auto Env::get(const std::string &key,
              const std::string &default_value) -> std::string {
    LOG_F(INFO, "Env::get called with key: {}, default_value: {}", key,
          default_value);
    std::lock_guard lock(impl_->mMutex);
    auto it = impl_->mArgs.find(key);
    if (it == impl_->mArgs.end()) {
        DLOG_F(INFO, "Env::get: Key: {} not found, return default value: {}",
               key, default_value);
        return default_value;
    }
    std::string value = it->second;
    LOG_F(INFO, "Env::get returning: {}", value);
    return value;
}

auto Env::setEnv(const std::string &key, const std::string &val) -> bool {
    LOG_F(INFO, "Env::setEnv called with key: {}, val: {}", key, val);
    std::lock_guard lock(impl_->mMutex);
    DLOG_F(INFO, "Env::setEnv: Set key: {} with value: {}", key, val);
#ifdef _WIN32
    bool result = SetEnvironmentVariableA(key.c_str(), val.c_str()) != 0;
#else
    bool result = setenv(key.c_str(), val.c_str(), 1) == 0;
#endif
    LOG_F(INFO, "Env::setEnv returning: {}", result);
    return result;
}

auto Env::getEnv(const std::string &key,
                 const std::string &default_value) -> std::string {
    LOG_F(INFO, "Env::getEnv called with key: {}, default_value: {}", key,
          default_value);
    std::lock_guard lock(impl_->mMutex);
    DLOG_F(INFO, "Env::getEnv: Get key: {} with default value: {}", key,
           default_value);
#ifdef _WIN32
    char buf[1024];
    DWORD ret = GetEnvironmentVariableA(key.c_str(), buf, sizeof(buf));
    if (ret == 0 || ret >= sizeof(buf)) {
        LOG_F(ERROR, "Env::getEnv: Get key: {} failed", key);
        return default_value;
    }
    DLOG_F(INFO, "Env::getEnv: Get key: {} with value: {}", key, buf);
    return buf;
#else
    const char *v = getenv(key.c_str());
    if (v == nullptr) {
        LOG_F(ERROR, "Env::getEnv: Get key: {} failed", key);
        return default_value;
    }
    DLOG_F(INFO, "Env::getEnv: Get key: {} with value: {}", key, v);
    return v;
#endif
}

auto Env::Environ() -> std::unordered_map<std::string, std::string> {
    LOG_F(INFO, "Env::Environ called");
    std::unordered_map<std::string, std::string> envMap;

#ifdef _WIN32
    LPCH envStrings = GetEnvironmentStrings();
    if (envStrings == nullptr) {
        LOG_F(ERROR, "Env::Environ: GetEnvironmentStrings failed");
        return envMap;
    }

    LPCH var = envStrings;
    while (*var != '\0') {
        std::string_view envVar(var);
        auto pos = envVar.find('=');
        if (pos != std::string_view::npos) {
            std::string key = std::string(envVar.substr(0, pos));
            std::string value = std::string(envVar.substr(pos + 1));
            envMap.emplace(key, value);
        }
        var += envVar.length() + 1;
    }

    FreeEnvironmentStrings(envStrings);

#elif __APPLE__ || __linux__ || __ANDROID__
    // Use POSIX API to get environment variables
    for (char **current = environ; *current; ++current) {
        std::string_view envVar(*current);
        auto pos = envVar.find('=');
        if (pos != std::string_view::npos) {
            std::string key = std::string(envVar.substr(0, pos));
            std::string value = std::string(envVar.substr(pos + 1));
            envMap.emplace(key, value);
        }
    }
#endif

    LOG_F(INFO, "Env::Environ returning environment map with {} entries",
          envMap.size());
    return envMap;
}

void Env::unsetEnv(const std::string &name) {
    LOG_F(INFO, "Env::unsetVariable called with name: {}", name);
#if defined(_WIN32) || defined(_WIN64)
    if (SetEnvironmentVariableA(name.c_str(), nullptr) == 0) {
        LOG_F(ERROR, "Failed to unset environment variable: {}", name);
    }
#else
    if (unsetenv(name.c_str()) != 0) {
        LOG_F(ERROR, "Failed to unset environment variable: {}", name);
    }
#endif
}

auto Env::listVariables() -> std::vector<std::string> {
    LOG_F(INFO, "Env::listVariables called");
    std::vector<std::string> vars;

#if defined(_WIN32) || defined(_WIN64)
    LPCH envStrings = GetEnvironmentStringsA();
    if (envStrings != nullptr) {
        for (LPCH var = envStrings; *var != '\0'; var += strlen(var) + 1) {
            vars.emplace_back(var);
        }
        FreeEnvironmentStringsA(envStrings);
    }
#else
    char **env = environ;
    while (*env != nullptr) {
        vars.emplace_back(*env);
        ++env;
    }
#endif

    LOG_F(INFO, "Env::listVariables returning {} variables", vars.size());
    return vars;
}

#if ATOM_ENABLE_DEBUG
void Env::printAllVariables() {
    LOG_F(INFO, "Env::printAllVariables called");
    std::vector<std::string> vars = listVariables();
    for (const auto &var : vars) {
        DLOG_F(INFO, "{}", var);
    }
}
#endif
}  // namespace atom::utils
