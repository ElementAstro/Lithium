/*
 * packageloader.hpp
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

#pragma once

#include <string>
#include <vector>

namespace OpenAPT
{

    class PackageManager
    {
    public:
        PackageManager();
        // 更新软件包列表
        bool update();

        // 安装软件包
        bool install(const std::string &package_name);

        // 卸载软件包
        bool remove(const std::string &package_name);

        // 搜索软件包
        bool search(const std::string &keyword, std::vector<std::pair<std::string, std::string>> &result);

        // 安装 Windows exe
        /**
         * 在 Windows 上安装一个 exe 文件。支持从 URL 和本地文件路径中读取。
         *
         * @param url 需要下载的 exe 文件的 URL，可以为空字符串。
         * @param local_file_path 本地 exe 文件的路径，可以为空字符串。
         * @param local_exe_path 本地 exe 文件的完整路径，可以为空字符串。
         * @return 如果安装成功，则返回 true；否则返回 false。
         */
        bool install_windows_exe(const std::string &url, const std::string &local_file_path, const std::string &local_exe_path);

        /**
         * 安装 Mac 应用程序
         *
         * 在 Mac 上使用 Homebrew 安装指定的应用程序。如果本地文件路径参数为空，则从 URL 下载应用程序并安装。
         *
         * @param url 应用程序的下载链接
         * @param local_file_path 本地应用程序文件的路径
         * @return 成功返回 true，失败返回 false
         */
        bool install_mac_app(const std::string &url, const std::string &local_file_path);

    private:
        // 执行命令，并将结果存入 result 中
        bool execute_cmd(const std::string &cmd_str, std::vector<std::pair<std::string, std::string>> *result) const;

        // 判断包名是否合法
        bool is_valid_package_name(const std::string &s) const;

        // 判断关键词是否合法
        bool is_valid_keyword(const std::string &s) const;

        // 判断当前操作系统是否为 Windows
        bool is_windows() const;

        /**
         * 判断当前操作系统是否为 macOS.
         *
         * @return 如果当前操作系统为 macOS，则返回 true；否则返回 false。
         */
        bool is_macos();
    };

}