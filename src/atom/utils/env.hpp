/*
 * env.hpp
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

#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>

class Env
{
public:
    Env(int argc, char **argv);

    void add(const std::string &key, const std::string &val);
    bool has(const std::string &key);
    void del(const std::string &key);
    std::string get(const std::string &key, const std::string &default_value);
    void addHelp(const std::string &key, const std::string &desc);
    void removeHelp(const std::string &key);
    void printHelp();
    bool setEnv(const std::string &key, const std::string &val);
    std::string getEnv(const std::string &key, const std::string &default_value);
    std::string getAbsolutePath(const std::string &path) const;
    std::string getAbsoluteWorkPath(const std::string &path) const;
    std::string getConfigPath();

private:
    std::string m_exe;
    std::string m_cwd;
    std::string m_program;

    std::unordered_map<std::string, std::string> m_args;
    std::vector<std::pair<std::string, std::string>> m_helps;

    mutable std::mutex m_mutex;
};
