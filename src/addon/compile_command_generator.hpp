#ifndef COMPILE_COMMAND_GENERATOR_HPP
#define COMPILE_COMMAND_GENERATOR_HPP

#include <memory>
#include <string>

namespace lithium {

/**
 * @brief A class to generate compile commands based on configured options.
 *
 * This class facilitates the generation of compile commands by allowing users
 * to set various compiler options, define targets, add conditional options,
 * and manage dependencies. It supports loading configurations from files and
 * generating compile commands in a structured JSON format.
 */
class CompileCommandGenerator {
public:
    CompileCommandGenerator();
    ~CompileCommandGenerator();

    /**
     * @brief Sets a global option for the compile commands.
     *
     * @param key The option key.
     * @param value The option value.
     * @return Reference to the current instance for method chaining.
     */
    auto setOption(const std::string& key,
                   const std::string& value) -> CompileCommandGenerator&;

    /**
     * @brief Adds a new target to the generator.
     *
     * @param target_name The name of the target.
     * @return Reference to the current instance for method chaining.
     */
    auto addTarget(const std::string& target_name) -> CompileCommandGenerator&;

    /**
     * @brief Sets an option specific to a target.
     *
     * @param target_name The name of the target.
     * @param key The option key.
     * @param value The option value.
     * @return Reference to the current instance for method chaining.
     */
    auto setTargetOption(const std::string& target_name, const std::string& key,
                         const std::string& value) -> CompileCommandGenerator&;

    /**
     * @brief Adds a conditional option based on a specified condition.
     *
     * @param condition The condition string.
     * @param key The option key.
     * @param value The option value.
     * @return Reference to the current instance for method chaining.
     */
    auto addConditionalOption(const std::string& condition,
                              const std::string& key, const std::string& value)
        -> CompileCommandGenerator&;

    /**
     * @brief Adds a macro definition to the compile commands.
     *
     * @param define The macro definition.
     * @return Reference to the current instance for method chaining.
     */
    auto addDefine(const std::string& define) -> CompileCommandGenerator&;

    /**
     * @brief Adds a compiler flag to the compile commands.
     *
     * @param flag The compiler flag.
     * @return Reference to the current instance for method chaining.
     */
    auto addFlag(const std::string& flag) -> CompileCommandGenerator&;

    /**
     * @brief Adds a library dependency to the compile commands.
     *
     * @param library_path The path to the library.
     * @return Reference to the current instance for method chaining.
     */
    auto addLibrary(const std::string& libraryPath)
        -> CompileCommandGenerator&;

    /**
     * @brief Sets the command template for generating compile commands.
     *
     * The template can include placeholders like {compiler}, {include},
     * {output}, and {file}.
     *
     * @param template_str The command template string.
     * @return Reference to the current instance for method chaining.
     */
    auto setCommandTemplate(const std::string& templateStr)
        -> CompileCommandGenerator&;

    /**
     * @brief Sets the compiler to be used for generating compile commands.
     *
     * @param compiler The compiler executable name or path.
     * @return Reference to the current instance for method chaining.
     */
    auto setCompiler(const std::string& compiler) -> CompileCommandGenerator&;

    /**
     * @brief Loads configuration options from a JSON file.
     *
     * The configuration file can specify options, defines, libraries, targets,
     * etc.
     *
     * @param config_path The path to the configuration file.
     */
    void loadConfigFromFile(const std::string& configPath);

    /**
     * @brief Generates the compile commands based on the configured options.
     *
     * The generated commands are saved to a specified output path in JSON
     * format.
     */
    void generate();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace lithium

#endif  // COMPILE_COMMAND_GENERATOR_HPP