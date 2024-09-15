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
class CompileCommandGenerator;
class Compiler {
public:
    Compiler();
    ~Compiler();

    /**
     * Compile C++ code into a shared library and load it into memory
     * @param code Code to compile
     * @param moduleName Module name
     * @param functionName Entry function name
     * @param optionsFile Compilation options file path, default is
     * "compile_options.json"
     * @return Whether compilation was successful
     */
    ATOM_NODISCARD auto compileToSharedLibrary(
        std::string_view code, std::string_view moduleName,
        std::string_view functionName,
        std::string_view optionsFile = "compile_options.json") -> bool;

    /**
     * Add custom compilation options
     * @param options Compilation options
     */
    void addCompileOptions(const std::string& options);

    /**
     * Get list of available compilers
     * @return List of compilers
     */
    ATOM_NODISCARD auto getAvailableCompilers() const
        -> std::vector<std::string>;

    /**
     * Generate compile commands for a given source directory
     * @param sourceDir Source directory path
     */
    void generateCompileCommands(const std::string& sourceDir);

private:
    std::unique_ptr<CompilerImpl> impl_;
};
}  // namespace lithium

#endif
