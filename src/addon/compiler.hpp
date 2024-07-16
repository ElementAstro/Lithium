/*
 * compiler.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: Compiler

**************************************************/

#ifndef LITHIUM_ADDON_COMPILER_HPP
#define LITHIUM_ADDON_COMPILER_HPP

#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "atom/log/loguru.hpp"

namespace lithium {

class Compiler {
public:
    /**
     * 编译 C++ 代码为共享库，并加载到内存中
     * @param code 要编译的代码
     * @param moduleName 模块名
     * @param functionName 入口函数名
     * @param optionsFile 编译选项文件路径,默认为 "compile_options.json"
     * @return 编译是否成功
     */
    [[nodiscard]] auto compileToSharedLibrary(
        std::string_view code, std::string_view moduleName,
        std::string_view functionName,
        std::string_view optionsFile = "compile_options.json") -> bool;

    /**
     * 添加自定义编译选项
     * @param options 编译选项
     */
    void addCompileOptions(const std::string& options);

    /**
     * 获取可用编译器列表
     * @return 编译器列表
     */
    [[nodiscard]] auto getAvailableCompilers() const -> std::vector<std::string>;

private:
    /**
     * 创建输出目录
     * @param outputDir 输出目录路径
     */
    void createOutputDirectory(const std::filesystem::path& outputDir);

    /**
     * 语法检查
     * @param code 要编译的代码
     * @param compiler 编译器路径
     * @return 语法检查是否成功
     */
    [[nodiscard]] auto syntaxCheck(std::string_view code,
                                   std::string_view compiler) -> bool;

    /**
     * 编译代码
     * @param code 要编译的代码
     * @param compiler 编译器路径
     * @param compileOptions 编译选项
     * @param output 编译输出路径
     * @return 编译是否成功
     */
    [[nodiscard]] bool compileCode(std::string_view code,
                                   std::string_view compiler,
                                   std::string_view compileOptions,
                                   const std::filesystem::path& output);

    /**
     * 查找可用的编译器
     * @return 可用的编译器列表
     */
    [[nodiscard]] auto findAvailableCompilers() -> std::vector<std::string>;

    /**
     * 运行外部 shell 命令，并将标准输入输出流转发到命令的标准输入输出流中
     * @param command 要运行的命令
     * @param input 标准输入
     * @param output 标准输出
     * @return 命令运行的返回值
     */
    auto runCommand(std::string_view command, std::string_view input,
                   std::string& output) -> int;

    std::unordered_map<std::string, std::filesystem::path> cache_;
    std::string customCompileOptions_;
};

}  // namespace lithium

#endif