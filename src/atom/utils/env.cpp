/*
 * env.cpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-12-16

Description: Environment variable management

**************************************************/

#include "env.hpp"

#include <iomanip>
#include <filesystem>
#include <algorithm>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "atom/log/loguru.hpp"

Env::Env(int argc, char **argv)
{
    std::filesystem::path exe_path;

#ifdef _WIN32
    char buf[MAX_PATH];
    GetModuleFileName(NULL, buf, MAX_PATH);
    exe_path = buf;
#else
    char link_buf[1024];
    ssize_t count = readlink("/proc/self/exe", link_buf, sizeof(link_buf));
    if (count != -1)
    {
        link_buf[count] = '\0';
        exe_path = link_buf;
    }
#endif

    m_exe = exe_path.string();

    m_cwd = exe_path.parent_path().string() + '/';

    m_program = argv[0];

    if (argc > 1)
    {
        int i = 1;
        int j;
        for (j = 2; j < argc; ++j)
        {
            if (argv[i][0] == '-' && argv[j][0] == '-')
            {
                add(std::string(argv[i] + 1), "");
                i = j;
            }
            else if (argv[i][0] == '-' && argv[j][0] != '-')
            {
                add(std::string(argv[i] + 1), std::string(argv[j]));
                ++j;
                i = j;
            }
            else
            {
                return;
            }
        }

        if (i < argc)
        {
            if (argv[i][0] == '-')
            {
                add(std::string(argv[i] + 1), "");
            }
            else
            {
                return;
            }
        }
    }
}

void Env::add(const std::string &key, const std::string &val)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_args[key] = val;
}

bool Env::has(const std::string &key)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_args.count(key) > 0;
}

void Env::del(const std::string &key)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_args.erase(key);
}

std::string Env::get(const std::string &key, const std::string &default_value)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_args.find(key);
    return it != m_args.end() ? it->second : default_value;
}

void Env::addHelp(const std::string &key, const std::string &desc)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_helps.push_back(std::make_pair(key, desc));
}

void Env::removeHelp(const std::string &key)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_helps.erase(std::remove_if(m_helps.begin(), m_helps.end(),
                                 [&](const std::pair<std::string, std::string> &p)
                                 {
                                     return p.first == key;
                                 }),
                  m_helps.end());
}

void Env::printHelp()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    LOG_F(INFO, "Usage: {} [options]", m_program);
    for (const auto &i : m_helps)
    {
        LOG_F(INFO, "    {} : {}", i.first, i.second);
    }
}

bool Env::setEnv(const std::string &key, const std::string &val)
{
#ifdef _WIN32
    return SetEnvironmentVariableA(key.c_str(), val.c_str()) != 0;
#else
    return setenv(key.c_str(), val.c_str(), 1) == 0;
#endif
}

std::string Env::getEnv(const std::string &key, const std::string &default_value)
{
#ifdef _WIN32
    char buf[1024];
    DWORD ret = GetEnvironmentVariableA(key.c_str(), buf, sizeof(buf));
    if (ret == 0 || ret >= sizeof(buf))
    {
        return default_value;
    }
    return buf;
#else
    const char *v = getenv(key.c_str());
    if (v == nullptr)
    {
        return default_value;
    }
    return v;
#endif
}

std::string Env::getAbsolutePath(const std::string &path) const
{
    if (path.empty())
    {
        return "/";
    }
#ifdef _WIN32
    if (path[1] == ':')
    {
        return path;
    }
#else
    if (path[0] == '/')
    {
        return path;
    }
#endif
    return m_cwd + path;
}

std::string Env::getAbsoluteWorkPath(const std::string &path) const
{
    if (path.empty())
    {
        return "/";
    }
#ifdef _WIN32
    if (path[1] == ':')
    {
        return path;
    }
#else
    if (path[0] == '/')
    {
        return path;
    }
#endif
    return path;
}

std::string Env::getConfigPath()
{
    return getAbsolutePath(get("c", "config"));
}

/*
int main(int argc, char **argv)
{
    johnsonli::Env env(argc, argv);
    env.addHelp("h", "Print help message");
    env.addHelp("c", "Set config file");

    if (env.has("h"))
    {
        env.printHelp();
        return 0;
    }

    std::cout << "Config Path: " << env.getConfigPath() << std::endl;

    return 0;
}
*/
