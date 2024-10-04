#ifndef COMPILE_COMMAND_GENERATOR_H
#define COMPILE_COMMAND_GENERATOR_H

#include <memory>
#include <string>

#include "atom/type/json.hpp"
using json = nlohmann::json;

namespace lithium {

/**
 * @class CompileCommandGenerator
 * @brief Generates compile commands for a project.
 *
 * This class provides an interface to configure and generate compile commands
 * for a project, including setting source directories, compiler options, and
 * output paths.
 */
class CompileCommandGenerator {
public:
    /**
     * @brief Constructs a CompileCommandGenerator object.
     */
    CompileCommandGenerator();

    /**
     * @brief Destroys the CompileCommandGenerator object.
     */
    ~CompileCommandGenerator();

    /**
     * @brief Sets the source directory for the project.
     *
     * @param dir The path to the source directory.
     */
    void setSourceDir(const std::string& dir);

    /**
     * @brief Sets the compiler to be used for the project.
     *
     * @param compiler The name or path of the compiler.
     */
    void setCompiler(const std::string& compiler);

    /**
     * @brief Sets the include flag for the compiler.
     *
     * @param flag The include flag (e.g., -I for GCC/Clang).
     */
    void setIncludeFlag(const std::string& flag);

    /**
     * @brief Sets the output flag for the compiler.
     *
     * @param flag The output flag (e.g., -o for GCC/Clang).
     */
    void setOutputFlag(const std::string& flag);

    /**
     * @brief Sets the project name.
     *
     * @param name The name of the project.
     */
    void setProjectName(const std::string& name);

    /**
     * @brief Sets the project version.
     *
     * @param version The version of the project.
     */
    void setProjectVersion(const std::string& version);

    /**
     * @brief Adds a file extension to be included in the compile commands.
     *
     * @param ext The file extension (e.g., .cpp, .c).
     */
    void addExtension(const std::string& ext);

    /**
     * @brief Sets the output path for the generated compile commands.
     *
     * @param path The path where the compile commands will be saved.
     */
    void setOutputPath(const std::string& path);

    /**
     * @brief Sets the path to existing compile commands.
     *
     * @param path The path to the existing compile commands file.
     */
    void setExistingCommandsPath(const std::string& path);

    /**
     * @brief Generates the compile commands based on the configured settings.
     */
    void generate();

private:
    struct Impl; /**< Forward declaration of the implementation struct. */
    std::unique_ptr<Impl> impl_; /**< Pointer to the implementation. */
};

}  // namespace lithium

#endif  // COMPILE_COMMAND_GENERATOR_H