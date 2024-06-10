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
#include <filesystem>
#include <iomanip>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "atom/log/loguru.hpp"

namespace atom::utils {
Env::Env(int argc, char **argv) {
    std::filesystem::path exe_path;

#ifdef _WIN32
    wchar_t buf[MAX_PATH];
    GetModuleFileName(NULL, buf, MAX_PATH);
    exe_path = buf;
#else
    char link_buf[1024];
    ssize_t count = readlink("/proc/self/exe", link_buf, sizeof(link_buf));
    if (count != -1) {
        link_buf[count] = '\0';
        exe_path = link_buf;
    }
#endif

    m_exe = exe_path.string();

    m_cwd = exe_path.parent_path().string() + '/';

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

std::shared_ptr<Env> Env::createShared(int argc, char **argv) {
    return std::make_shared<Env>(argc, argv);
}

std::unique_ptr<Env> Env::createUnique(int argc, char **argv) {
    return std::make_unique<Env>(argc, argv);
}

void Env::add(const std::string &key, const std::string &val) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (has(key)) {
        LOG_F(ERROR, "Env::add: Duplicate key: {}", key);
    } else {
        DLOG_F(INFO, "Env::add: Add key: {} with value: {}", key, val);
        m_args[key] = val;
    }
}

bool Env::has(const std::string &key) {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_args.count(key) > 0;
}

void Env::del(const std::string &key) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_args.erase(key);
    DLOG_F(INFO, "Env::del: Remove key: {}", key);
}

std::string Env::get(const std::string &key, const std::string &default_value) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_args.find(key);
    if (it == m_args.end()) {
        DLOG_F(INFO, "Env::get: Key: {} not found, return default value: {}",
               key, default_value);
        return "";
    }
    return it != m_args.end() ? it->second : default_value;
}

void Env::addHelp(const std::string &key, const std::string &desc) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_helps.push_back(std::make_pair(key, desc));
    DLOG_F(INFO, "Env::addHelp: Add key: {} with description: {}", key, desc);
}

void Env::removeHelp(const std::string &key) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_helps.erase(
        std::remove_if(m_helps.begin(), m_helps.end(),
                       [&](const std::pair<std::string, std::string> &p) {
                           return p.first == key;
                       }),
        m_helps.end());
    DLOG_F(INFO, "Env::removeHelp: Remove key: {}", key);
}

void Env::printHelp() {
    std::lock_guard<std::mutex> lock(m_mutex);
    DLOG_F(INFO, "Usage: {} [options]", m_program);
    for (const auto &i : m_helps) {
        DLOG_F(INFO, "    {} : {}", i.first, i.second);
    }
}

bool Env::setEnv(const std::string &key, const std::string &val) {
    std::lock_guard<std::mutex> lock(m_mutex);
    DLOG_F(INFO, "Env::setEnv: Set key: {} with value: {}", key, val);
#ifdef _WIN32
    return SetEnvironmentVariableA(key.c_str(), val.c_str()) != 0;
#else
    return setenv(key.c_str(), val.c_str(), 1) == 0;
#endif
}

std::string Env::getEnv(const std::string &key,
                        const std::string &default_value) {
    std::lock_guard<std::mutex> lock(m_mutex);
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

std::string Env::getAbsolutePath(const std::string &path) const {
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

std::string Env::getAbsoluteWorkPath(const std::string &path) const {
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

std::string Env::getConfigPath() { return getAbsolutePath(get("c", "config")); }

std::unordered_map<std::string, std::string> Env::Environ() {
    std::unordered_map<std::string, std::string> env;

#ifdef _WIN32
    LPWCH variables = GetEnvironmentStringsW();
    if (variables != nullptr) {
        LPWSTR currentVariable = variables;
        while (*currentVariable != L'\0') {
            std::wstring wideString(currentVariable);
            std::string variable(wideString.begin(), wideString.end());
            size_t delimiterPos = variable.find('=');
            if (delimiterPos != std::string::npos) {
                std::string key = variable.substr(0, delimiterPos);
                std::string value = variable.substr(delimiterPos + 1);
                env[key] = value;
            }
            currentVariable += wideString.size() + 1;
        }

        FreeEnvironmentStringsW(variables);
    }
#else
    extern char **environ;
    char **currentVariable = environ;
    while (*currentVariable != nullptr) {
        std::string variable(*currentVariable);
        size_t delimiterPos = variable.find('=');
        if (delimiterPos != std::string::npos) {
            std::string key = variable.substr(0, delimiterPos);
            std::string value = variable.substr(delimiterPos + 1);
            env[key] = value;
        }
        ++currentVariable;
    }
#endif

    return env;
}
}  // namespace atom::utils
