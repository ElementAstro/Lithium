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

namespace fs = std::filesystem;

namespace atom::utils {
Env::Env(int argc, char **argv) {
    fs::path exePath;

#ifdef _WIN32
    wchar_t buf[MAX_PATH];
    if (GetModuleFileNameW(nullptr, buf, MAX_PATH) == 0U) {
        LOG_F(ERROR, "GetModuleFileNameW failed with error {}", GetLastError());
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

    m_exe = exePath.string();

    m_cwd = exePath.parent_path().string() + '/';

    m_program = argv[0];

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
}

auto Env::createShared(int argc, char **argv) -> std::shared_ptr<Env> {
    return std::make_shared<Env>(argc, argv);
}

void Env::add(const std::string &key, const std::string &val) {
    std::lock_guard lock(m_mutex);
    if (has(key)) {
        LOG_F(ERROR, "Env::add: Duplicate key: {}", key);
    } else {
        DLOG_F(INFO, "Env::add: Add key: {} with value: {}", key, val);
        m_args[key] = val;
    }
}

auto Env::has(const std::string &key) -> bool {
    std::lock_guard lock(m_mutex);
    return m_args.contains(key);
}

void Env::del(const std::string &key) {
    std::lock_guard lock(m_mutex);
    m_args.erase(key);
    DLOG_F(INFO, "Env::del: Remove key: {}", key);
}

auto Env::get(const std::string &key,
              const std::string &default_value) -> std::string {
    std::lock_guard lock(m_mutex);
    auto it = m_args.find(key);
    if (it == m_args.end()) {
        DLOG_F(INFO, "Env::get: Key: {} not found, return default value: {}",
               key, default_value);
        return "";
    }
    return it != m_args.end() ? it->second : default_value;
}

void Env::addHelp(const std::string &key, const std::string &desc) {
    std::lock_guard lock(m_mutex);
    m_helps.emplace_back(key, desc);
    DLOG_F(INFO, "Env::addHelp: Add key: {} with description: {}", key, desc);
}

void Env::removeHelp(const std::string &key) {
    std::lock_guard lock(m_mutex);
    m_helps.erase(
        std::remove_if(m_helps.begin(), m_helps.end(),
                       [&](const std::pair<std::string, std::string> &p) {
                           return p.first == key;
                       }),
        m_helps.end());
    DLOG_F(INFO, "Env::removeHelp: Remove key: {}", key);
}

void Env::printHelp() {
    std::lock_guard lock(m_mutex);
    DLOG_F(INFO, "Usage: {} [options]", m_program);
    for (const auto &i : m_helps) {
        DLOG_F(INFO, "    {} : {}", i.first, i.second);
    }
}

auto Env::setEnv(const std::string &key, const std::string &val) -> bool {
    std::lock_guard lock(m_mutex);
    DLOG_F(INFO, "Env::setEnv: Set key: {} with value: {}", key, val);
#ifdef _WIN32
    return SetEnvironmentVariableA(key.c_str(), val.c_str()) != 0;
#else
    return setenv(key.c_str(), val.c_str(), 1) == 0;
#endif
}

auto Env::getEnv(const std::string &key,
                 const std::string &default_value) -> std::string {
    std::lock_guard lock(m_mutex);
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

auto Env::getAbsolutePath(const std::string &path) const -> std::string {
    if (path.empty()) {
        return "/";
    }
#ifdef _WIN32
    if (path[1] == ':') {
        return path;
    }
#else
    if (path[0] == '/') {
        return path;
    }
#endif
    return m_cwd + path;
}

auto Env::getAbsoluteWorkPath(const std::string &path) const -> std::string {
    if (!path.empty()) {
#ifdef _WIN32
        if (path[1] == ':') {
            return path;
        }
#else
        if (path[0] == '/') {
            return path;
        }
#endif
    }
    return "/";
}

auto Env::getConfigPath() -> std::string {
    return getAbsolutePath(get("c", "config"));
}

auto Env::Environ() -> std::unordered_map<std::string, std::string> {
    std::unordered_map<std::string, std::string> envMap;

#ifdef _WIN32
    LPCH envStrings = GetEnvironmentStrings();
    if (envStrings == nullptr) {
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

    return envMap;
}

void Env::setVariable(const std::string &name, const std::string &value,
                      bool overwrite) {
#if defined(_WIN32) || defined(_WIN64)
    if (overwrite || (getenv(name.c_str()) == nullptr)) {
        if (SetEnvironmentVariableA(name.c_str(), value.c_str()) == 0) {
            LOG_F(ERROR, "Failed to set environment variable: {}", name);
        }
    }
#else
    if (setenv(name.c_str(), value.c_str(), overwrite ? 1 : 0) != 0) {
        LOG_F(ERROR, "Failed to set environment variable: {}", name);
    }
#endif
}

// 获取环境变量
auto Env::getVariable(const std::string &name) -> std::string {
#if defined(_WIN32) || defined(_WIN64)
    char buffer[32767];
    if (GetEnvironmentVariableA(name.c_str(), buffer, 32767) > 0) {
        return std::string(buffer);
    }
#else
    const char *value = getenv(name.c_str());
    if (value != nullptr) {
        return value;
    }
#endif
    LOG_F(ERROR, "Environment variable not found: {}", name);
    return "";
}

// 删除环境变量
void Env::unsetVariable(const std::string &name) {
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

// 列出所有环境变量
auto Env::listVariables() -> std::vector<std::string> {
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

    return vars;
}

// 输出所有环境变量
void Env::printAllVariables() {
    std::vector<std::string> vars = listVariables();
    for (const auto &var : vars) {
        DLOG_F(INFO, "{}", var);
    }
}
}  // namespace atom::utils
