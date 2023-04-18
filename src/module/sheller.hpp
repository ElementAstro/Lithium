/*
 * sheller.hpp
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

Date: 2023-4-9

Description: Shell Manager

**************************************************/

#pragma once

#include <vector>
#include <string>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace OpenAPT
{

    enum class ScriptType
    {
        Sh, // Shell 脚本
        Ps  // PowerShell 脚本
    };

    class ScriptManager
    {
    public:
        /**
         * @brief 构造函数
         * @param path 脚本文件所在的目录
         */
        explicit ScriptManager(const std::string &path);

        /**
         * @brief 遍历给定目录下所有的脚本文件，包括子文件夹
         * @return 所有脚本文件的路径
         */
        std::vector<std::string> getScriptFiles() const;

        /**
         * @brief 从脚本文件中读取脚本内容
         * @param path 脚本文件的路径
         * @return 脚本内容
         */
        std::string readScriptFromFile(const std::string &path) const;

        /**
         * @brief 校验脚本是否正确
         * @param script 脚本内容
         * @param scriptType 脚本类型
         * @return 如果脚本正确返回 true，否则返回 false
         */
        bool validateScript(const std::string &script, ScriptType scriptType) const;

        /**
         * @brief 将脚本名字和路径存储到 JSON 变量中
         * @param files 所有脚本文件的路径
         * @return 存储脚本名字和路径的 JSON 变量
         */
        json getScriptsJson(const std::vector<std::string> &files) const;

        /**
         * @brief 通过名字获取脚本并校验，然后执行该脚本
         * @param scriptName 脚本的名字
         * @param async 是否异步执行，默认为 false（同步执行）
         * @return 如果执行成功返回 true，否则返回 false
         */
        bool runScript(const std::string &scriptName, bool async = false) const;

    private:
        const std::string m_path;               // 脚本文件所在的目录
        const std::vector<std::string> m_files; // 所有脚本文件的路径
        const json m_scriptsJson;               // 存储脚本名字和路径的 JSON 变量

        /**
         * @brief 根据文件扩展名判断脚本类型
         * @param path 脚本文件的路径
         * @return 脚本类型
         */
        ScriptType getScriptType(const std::string &path) const;

        /**
         * @brief 根据操作系统不同构建执行脚本的命令
         * @param scriptPath 脚本文件的路径
         * @return 执行脚本的命令
         */
        std::string buildCommand(const std::string &scriptPath) const;

        /**
         * @brief 执行命令并获取输出
         * @param command 要执行的命令
         * @return 命令执行的输出
         */
        std::string executeCommand(const std::string &command) const;
    };

}
