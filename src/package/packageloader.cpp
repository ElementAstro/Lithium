/*
 * packageloader.cpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-3-31

Description: Package Manager

**************************************************/

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <cstdio>
#include <cstring>

// 用于判断操作系统的宏
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#include <spdlog/spdlog.h>

#include "packageloader.hpp"

using namespace std;

namespace OpenAPT{

    PackageManager::PackageManager()
    {
        spdlog::info("Package manager loaded successfully");
    }

    bool PackageManager::update()
    {
        string cmd_str = "";
        if (is_windows())
        {
            cmd_str = "choco upgrade all -y";
        }
        else
        { // Linux 系统
            cmd_str = "sudo apt-get update";
        }
        std::vector<std::pair<std::string, std::string>> result;
        bool ret = execute_cmd(cmd_str, &result);
        if (ret)
        {
            spdlog::info("Package list updated successfully.");
        }
        else
        {
            spdlog::error("Failed to update package list.");
        }
        return ret;
    }

    bool PackageManager::install(const string &package_name)
    {
        if (package_name.empty())
        {
            spdlog::error("Invalid package name.");
            return false;
        }

        string cmd_str = "";
        if (is_windows())
        {
            cmd_str = "choco install " + package_name + " -y";
        }
        else
        { // Linux 系统
            cmd_str = "sudo apt-get install " + package_name;
        }
        std::vector<std::pair<std::string, std::string>> result;
        bool ret = execute_cmd(cmd_str, &result);
        if (ret)
        {
            spdlog::info("Package {} installed successfully.", package_name);
        }
        else
        {
            spdlog::error("Failed to install package {}.", package_name);
        }
        return ret;
    }

    bool PackageManager::remove(const string &package_name)
    {
        if (package_name.empty())
        {
            spdlog::error("Invalid package name.");
            return false;
        }

        string cmd_str = "";
        if (is_windows())
        {
            cmd_str = "choco uninstall " + package_name + " -y";
        }
        else
        { // Linux 系统
            cmd_str = "sudo apt-get remove " + package_name;
        }
        std::vector<std::pair<std::string, std::string>> result;
        bool ret = execute_cmd(cmd_str, &result);
        if (ret)
        {
            spdlog::info("Package {} uninstalled successfully.", package_name);
        }
        else
        {
            spdlog::error("Failed to uninstall package {}.", package_name);
        }
        return ret;
    }

    bool PackageManager::search(const string &keyword, vector<pair<string, string>> &result)
    {
        if (keyword.empty())
        {
            spdlog::error("Invalid keyword.");
            return false;
        }

        string cmd_str = "";
        if (is_windows())
        {
            cmd_str = "choco search " + keyword;
        }
        else
        { // Linux 系统
            cmd_str = "apt-cache search " + keyword;
        }

        bool ret = execute_cmd(cmd_str, &result);
        if (ret)
        {
            for (auto &p : result)
            {
                spdlog::debug("{}: {}", p.first, p.second);
            }
        }
        else
        {
            spdlog::error("Failed to search for packages matching {}.", keyword);
        }
        return ret;
    }

    bool PackageManager::install_windows_exe(const string &url, const string &local_file_path = "", const string &local_exe_path = "")
    {
        if (url.empty() && local_file_path.empty() && local_exe_path.empty())
        {
            spdlog::error("Invalid input."); // 错误提示：输入参数无效。
            return false;
        }

        string cmd_str = "";
        if (is_windows())
        { // 判断当前是否在 Windows 操作系统上。
            // 复制 exe 文件到 temp.exe
            if (!local_exe_path.empty())
            { // 如果本地 exe 文件路径不为空，则将其复制到 temp.exe。
                cmd_str = "copy \"" + local_exe_path + "\" temp.exe /y";
            }
            else
            {
                if (!local_file_path.empty())
                {
                    // 如果本地文件路径不为空，则将文件复制到 temp.exe。
                    cmd_str = "copy \"" + local_file_path + "\" temp.exe /y";
                }
                else
                {
                    // 如果 URL 不为空，则从 URL 中下载文件。
                    cmd_str = "powershell -Command \"Invoke-WebRequest -UseBasicParsing -Uri '" + url + "' -OutFile 'temp.exe'\"";
                }
            }
            std::vector<std::pair<std::string, std::string>> result;
        bool ret = execute_cmd(cmd_str, &result); // 执行命令复制或下载 exe 文件。
            if (!ret)
            {
                spdlog::error("Failed to copy or download exe file."); // 错误提示：复制或下载失败。
                return false;
            }

            // 安装 exe 文件
            cmd_str = "temp.exe /S";    // 执行 temp.exe 安装命令。
            ret = execute_cmd(cmd_str, &result); // 执行安装命令。
            if (ret)
            {
                spdlog::info("Exe file installed successfully."); // 成功提示：exe 文件安装成功。
            }
            else
            {
                spdlog::error("Failed to install exe file."); // 错误提示：exe 文件安装失败。
            }
            return ret;
        }
        else
        {
            spdlog::error("This function is not supported on this OS."); // 错误提示：该函数在当前操作系统不受支持。
            return false;
        }
    }

    bool PackageManager::install_mac_app(const string &url, const string &local_file_path = "")
    {
        if (url.empty() && local_file_path.empty())
        {
            spdlog::error("Invalid input.");
            return false;
        }

        string cmd_str = "";
        if (is_macos())
        {
            // 检查当前用户是否为管理员
            if (getuid() != 0)
            {
                spdlog::error("You must have administrator privileges to install applications. Please run this command with sudo.");
                return false;
            }

            // 检查是否已安装 Homebrew
            cmd_str = "which brew";
            std::vector<std::pair<std::string, std::string>> result;
            bool ret = execute_cmd(cmd_str, &result);
            if (!ret)
            {
                spdlog::info("Homebrew not detected. Installing Homebrew...");
                cmd_str = "/bin/bash -c \"$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\"";
                ret = execute_cmd(cmd_str, &result);
                if (!ret)
                {
                    spdlog::error("Failed to install Homebrew.");
                    return false;
                }
                spdlog::info("Homebrew installed successfully.");
            }

            // 使用 brew 安装应用
            if (!local_file_path.empty())
            {
                cmd_str = "brew install --cask " + local_file_path;
            }
            else
            {
                cmd_str = "brew install --cask " + url;
            }
            ret = execute_cmd(cmd_str, &result);
            if (ret)
            {
                spdlog::info("Application installed successfully.");
            }
            else
            {
                spdlog::error("Failed to install application.");
            }
            return ret;
        }
        else
        {
            spdlog::error("This function is not supported on this OS.");
            return false;
        }
    }

    bool PackageManager::execute_cmd(const string &cmd_str, vector<pair<string, string>> *result = nullptr) const
    {
        FILE *fp;
#ifdef _WIN32
        fp = _popen(cmd_str.c_str(), "r");
#else
        fp = popen(cmd_str.c_str(), "r");
#endif

        if (!fp)
        {
            throw runtime_error(strerror(errno));
        }

        char buffer[1024] = {0};
        while (fgets(buffer, sizeof(buffer), fp))
        {
            string line(buffer);
            line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());

            // 去掉换行符
            if (result && !result->empty())
            {
                stringstream ss(line);
                string name, version;
#ifdef _WIN32
                char *context = nullptr;
                char *name_cstr = strtok_s(buffer, " ", &context);
                ss << name_cstr << " ";
                ss >> name >> ws;
                getline(ss, version);
#else
                ss >> name >> ws;
                getline(ss, version);
#endif
                result->emplace_back(name, version);
            }
        }

        int ret;
#ifdef _WIN32
        ret = _pclose(fp);
#else
        ret = pclose(fp);
#endif

        bool success = WIFEXITED(ret) && (WEXITSTATUS(ret) == 0);
        if (success)
        {
            spdlog::debug("Command executed successfully: {}", cmd_str);
        }
        else
        {
            spdlog::error("Command failed: {}", cmd_str);
        }
        return success;
    }

    bool PackageManager::is_valid_package_name(const string &s) const
    {
        if (s.empty() || s.length() > 100)
        {
            return false;
        }

        if (!isalnum(s[0]) || !isalnum(s[s.length() - 1]))
        {
            return false;
        }

        for (size_t i = 1; i < s.length() - 1; i++)
        {
            if (!isalnum(s[i]) && s[i] != '-' && s[i] != '+')
            {
                return false;
            }
        }

        return true;
    }

    // 判断关键词是否合法
    bool PackageManager::is_valid_keyword(const string &s) const
    {
        if (s.empty() || s.length() > 100)
        {
            return false;
        }

        for (char c : s)
        {
            if (!isalnum(c) && c != '_' && c != '-')
            {
                return false;
            }
        }

        return true;
    }

    // 判断当前操作系统是否为 Windows
    bool PackageManager::is_windows() const
    {
#ifdef _WIN32
        return true;
#else
        return false;
#endif
    }

    /**
     * 判断当前操作系统是否为 macOS.
     *
     * @return 如果当前操作系统为 macOS，则返回 true；否则返回 false。
     */
    bool PackageManager::is_macos()
    {
#ifdef __APPLE__
#if TARGET_OS_MAC || TARGET_OS_IPHONE
        return true;
#endif
#endif
        return false;
    }
}


/*
int main() {
    // 初始化日志记录器
    spdlog::set_pattern("[%l] [%Y-%m-%d %H:%M:%S.%e] [thread %t] %v");
    spdlog::set_level(spdlog::level::debug);

    // 创建包管理器对象
    PackageManager pm;

    // 更新软件包列表
    pm.update();

    // 搜索关键词 c++
    vector<pair<string, string>> result;
    pm.search("c++", result);
    for (const auto& item : result) {
        spdlog::info("Keyword: {}, URL: {}", item.first, item.second);
    }

    return 0;
}
*/
