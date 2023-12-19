/*
 * compiler.hpp
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

Date: 2023-3-29

Description: Compiler

**************************************************/

#pragma once

#include <string>

#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

namespace Lithium
{

    class Compiler
    {
    public:
        /**
         * \brief 编译 C++ 代码为共享库，并加载到内存中
         *
         * \param code 要编译的代码
         * \param moduleName 模块名
         * \param functionName 入口函数名
         * \return 编译是否成功
         */
        bool CompileToSharedLibrary(const std::string &code, const std::string &moduleName, const std::string &functionName, const std::string &optionsFile = "compile_options.json");

        bool CompileToSharedLibraryAllinOne(const std::string &code, const std::string &moduleName, const std::string &functionName);

    private:
        // 检查参数是否有效
        bool CheckParameters(const std::string &code, const std::string &moduleName, const std::string &functionName);
        // 检查模块是否已经编译并缓存
        bool IsModuleCached(const std::string &moduleName, const std::string &functionName, std::unordered_map<std::string, std::string> &cache_);

        // 创建输出目录
        bool CreateOutputDirectory(const std::string &outputDir);

        // 从 JSON 文件中读取编译选项
        std::string ReadCompileOptions(const std::string &optionsFile);

        // 语法检查
        bool SyntaxCheck(const std::string &code, const std::string &compiler);
        // 编译代码
        bool CompileCode(const std::string &code, const std::string &compiler, const std::string &compileOptions, const std::string &output);

        // 缓存已编译的模块
        void CacheCompiledModule(const std::string &moduleName, const std::string &functionName, const std::string &output, std::unordered_map<std::string, std::string> &cache_);
        // ----------------------------------------------------
        // 文件、系统操作
        // ----------------------------------------------------
        /**
         * \brief 复制文件
         *
         * \param source 源文件路径
         * \param destination 目标文件路径
         * \return 是否复制成功
         */
        bool CopyFile_(const std::string &source, const std::string &destination);

        /**
         * \brief 运行外部 shell 命令，并将标准输入输出流转发到命令的标准输入输出流中
         *
         * \param command 要运行的命令
         * \param inputStream 标准输入流
         * \param outputStream 标准输出流
         * \return 命令运行的返回值
         */
        int RunShellCommand(const std::string &command, const std::string &input, std::string &output);

#if ENABLE_FASTHASH
        emhash8::HashMap<std::string, std::string> cache_;
#else
        std::unordered_map<std::string, std::string> cache_;
#endif
    };

}
