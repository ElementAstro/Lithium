/*
 * command.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-24

Description: Simple wrapper for executing commands.

**************************************************/

#include "command.hpp"

#include <array>
#include <cstdio>
#include <cstring>
#include <memory>
#include <sstream>
#include <stdexcept>
#ifdef _WIN32
#include <windows.h>
#define SETENV(name, value) SetEnvironmentVariableA(name, value)
#define UNSETENV(name) SetEnvironmentVariableA(name, NULL)
#else
#include <cstdio>
#include <cstring>
#include <sys/wait.h>
#include <unistd.h>
#define SETENV(name, value) setenv(name, value, 1)
#define UNSETENV(name) unsetenv(name)
#endif

namespace Atom::System
{
    std::string executeCommand(const std::string &command)
    {
        if (command.empty())
        {
            return "";
        }
        auto pipeDeleter = [](FILE *pipe)
        { if (pipe) pclose(pipe); };
        std::unique_ptr<FILE, decltype(pipeDeleter)> pipe(popen(command.c_str(), "r"), pipeDeleter);

        if (!pipe)
        {
            throw std::runtime_error("Error: failed to run command '" + command + "'.");
        }

        std::array<char, 4096> buffer{};
        std::ostringstream output;

        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
        {
            output << buffer.data();
        }

        return output.str();
    }

    std::string executeCommandWithEnv(const std::string &command, const std::map<std::string, std::string> &envVars)
    {
        if (command.empty())
        {
            return "";
        }
        // 保存当前环境变量的状态
        std::map<std::string, std::string> oldEnvVars;
        for (const auto &var : envVars)
        {
            char *oldValue = std::getenv(var.first.c_str());
            if (oldValue)
            {
                oldEnvVars[var.first] = oldValue;
            }
#if defined(_WIN32) || defined(_WIN64)
            SETENV(var.first.c_str(), var.second.c_str());
#else
            SETENV(var.first.c_str(), var.second.c_str());
#endif
        }

        // 执行命令
        auto result = executeCommand(command);

        // 清理：恢复环境变量到之前的状态
        for (const auto &var : envVars)
        {
            if (oldEnvVars.find(var.first) != oldEnvVars.end())
            {
                // 恢复旧值
                SETENV(var.first.c_str(), oldEnvVars[var.first].c_str());
            }
            else
            {
                // 如果之前不存在，则删除
                UNSETENV(var.first.c_str());
            }
        }

        return result;
    }

    std::pair<std::string, int> executeCommandWithStatus(const std::string &command)
    {
        if (command.empty())
        {
            return {"", -1};
        }
        std::array<char, 4096> buffer{};
        std::ostringstream output;

#ifdef _WIN32
        // Windows implementation using _popen and _pclose
        std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(command.c_str(), "r"), _pclose);
#else
        // Linux implementation using popen and pclose
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
#endif

        if (!pipe)
        {
            throw std::runtime_error("Error: failed to run command '" + command + "'.");
        }

        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
        {
            output << buffer.data();
        }

        int status = -1;
#ifdef _WIN32
        // Windows exit code retrieval
        if (pipe)
        {
            status = 0;
        }
#else
        // Linux exit code retrieval
        if (pipe)
        {
            status = WEXITSTATUS(pclose(pipe.get()));
        }
#endif

        return {output.str(), status};
    }
}
