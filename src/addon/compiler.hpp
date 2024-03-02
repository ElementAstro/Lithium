/*
 * compiler.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

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

namespace Lithium {
class Compiler {
public:
    /**
     * \brief 编译 C++ 代码为共享库，并加载到内存中
     *
     * \param code 要编译的代码
     * \param moduleName 模块名
     * \param functionName 入口函数名
     * \return 编译是否成功
     */
    bool CompileToSharedLibrary(
        const std::string &code, const std::string &moduleName,
        const std::string &functionName,
        const std::string &optionsFile = "compile_options.json");

    /**
     * \brief 编译 C++ 代码为共享库，并加载到内存中
     *
     * \param code 要编译的代码
     * \param moduleName 模块名
     * \param functionName 入口函数名
     * \return 编译是否成功
     */
    bool CompileToSharedLibraryAllinOne(const std::string &code,
                                        const std::string &moduleName,
                                        const std::string &functionName);

private:
    /**
     * \brief 检查参数
     * \param code 要编译的代码
     * \param moduleName 模块名
     * \param functionName 入口函数名
     * \return 检查是否成功
     */
    bool CheckParameters(const std::string &code, const std::string &moduleName,
                         const std::string &functionName);

    /**
     * \brief 判断模块是否已缓存
     * \param moduleName 模块名
     * \param functionName 入口函数名
     * \return 模块是否已缓存
     */
    bool IsModuleCached(const std::string &moduleName,
                        const std::string &functionName,
                        std::unordered_map<std::string, std::string> &cache_);

    /**
     * \brief 创建输出目录
     * \param outputDir 输出目录路径
     * \return 创建是否成功
     */
    bool CreateOutputDirectory(const std::string &outputDir);

    /**
     * \brief 读取编译选项
     * \param optionsFile 编译选项文件路径
     * \return 编译选项
     */
    std::string ReadCompileOptions(const std::string &optionsFile);

    /**
     * \brief 语法检查
     * \param code 要编译的代码
     * \param compiler 编译器路径
     * \return 语法检查是否成功
     */
    bool SyntaxCheck(const std::string &code, const std::string &compiler);

    /**
     * \brief 编译代码
     * \param code 要编译的代码
     * \param compiler 编译器路径
     * \param compileOptions 编译选项
     * \param output 编译输出路径
     * \return 编译是否成功
     */
    bool CompileCode(const std::string &code, const std::string &compiler,
                     const std::string &compileOptions,
                     const std::string &output);

    /**
     * \brief 缓存编译好的模块
     * \param moduleName 模块名
     * \param functionName 入口函数名
     * \param output 编译输出路径
     * \return 缓存是否成功
     */
    void CacheCompiledModule(
        const std::string &moduleName, const std::string &functionName,
        const std::string &output,
        std::unordered_map<std::string, std::string> &cache_);

    // ----------------------------------------------------
    // File and Directory
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
     * \brief 运行外部 shell
     * 命令，并将标准输入输出流转发到命令的标准输入输出流中
     *
     * \param command 要运行的命令
     * \param inputStream 标准输入流
     * \param outputStream 标准输出流
     * \return 命令运行的返回值
     */
    int RunShellCommand(const std::string &command, const std::string &input,
                        std::string &output);

#if ENABLE_FASTHASH
    emhash8::HashMap<std::string, std::string> cache_;
#else
    std::unordered_map<std::string, std::string> cache_;
#endif
};

}  // namespace Lithium
