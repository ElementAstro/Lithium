/**
 * @file compiler.hpp
 *
 * @brief Contains the compiler definitions and declarations.
 *
 * This file includes all necessary definitions and declarations related to the
 * compiler. It is part of the codebase developed by Max Qian.
 *
 * @copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 * @date 2023-03-29
 */

#ifndef LITHIUM_ADDON_COMPILER_HPP
#define LITHIUM_ADDON_COMPILER_HPP

#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include "macro.hpp"

#include "atom/type/json_fwd.hpp"
using json = nlohmann::json;

namespace lithium {

class CompilerImpl;

class Compiler {
public:
    Compiler();
    ~Compiler();

    /**
     * 编译 C++ 代码为共享库，并加载到内存中
     * @param code 要编译的代码
     * @param moduleName 模块名
     * @param functionName 入口函数名
     * @param optionsFile 编译选项文件路径,默认为 "compile_options.json"
     * @return 编译是否成功
     */
    ATOM_NODISCARD auto compileToSharedLibrary(
        std::string_view code, std::string_view moduleName,
        std::string_view functionName,
        std::string_view optionsFile = "compile_options.json") -> bool;

    /**
     * 添加自定义编译选项
     * @param options 编译选项
     */
    void addCompileOptions(const std::string &options);

    /**
     * 获取可用编译器列表
     * @return 编译器列表
     */
    ATOM_NODISCARD auto getAvailableCompilers() const
        -> std::vector<std::string>;

private:
    std::unique_ptr<CompilerImpl> impl_;
};
}  // namespace lithium

#endif
