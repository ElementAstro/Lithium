#include "compile_command_generator.hpp"

#include <exception>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <regex>
#include <utility>
// #include <algorithm> // Removed unused include

#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"

using json = nlohmann::json;

namespace lithium {

// Constants
constexpr int K_ALIGNMENT = 128;

/**
 * @brief Structure representing a compile command.
 *
 * Each compile command includes the working directory, the compile command,
 * and the source file associated with the command.
 */
struct alignas(K_ALIGNMENT) CompileCommand {
private:
    std::string directory_;
    std::string command_;
    std::string file_;

public:
    /**
     * @brief Converts the compile command to a JSON object.
     *
     * @return JSON representation of the compile command.
     */
    [[nodiscard]] json toJson() const {
        return json{
            {"directory", directory_}, {"command", command_}, {"file", file_}};
    }

    /**
     * @brief Initializes the compile command from a JSON object.
     *
     * @param jsonObj The JSON object containing compile command data.
     */
    void fromJson(const json& jsonObj) {
        directory_ = jsonObj.at("directory").get<std::string>();
        command_ = jsonObj.at("command").get<std::string>();
        file_ = jsonObj.at("file").get<std::string>();
    }

    CompileCommand() = default;

    /**
     * @brief Constructs a compile command with specified directory, command,
     * and file.
     *
     * @param directory The working directory for the compile command.
     * @param command The compile command string.
     * @param file The source file to compile.
     */
    CompileCommand(std::string directory, std::string command, std::string file)
        : directory_(std::move(directory)),
          command_(std::move(command)),
          file_(std::move(file)) {}
};

/**
 * @brief Implementation structure for CompileCommandGenerator using the Pimpl
 * idiom.
 */
struct alignas(K_ALIGNMENT) CompileCommandGenerator::Impl {
private:
    std::string sourceDir_ = "./src";
    std::vector<std::string> extensions_ = {".cpp", ".c"};
    std::unordered_map<std::string, std::string> options_;
    std::unordered_map<std::string,
                       std::unordered_map<std::string, std::string>>
        targetOptions_;
    std::unordered_map<std::string,
                       std::unordered_map<std::string, std::string>>
        conditionalOptions_;
    std::vector<std::string> defines_;
    std::vector<std::string> flags_;
    std::vector<std::string> libraries_;
    std::string commandTemplate_ = "{compiler} {include} {output} {file}";
    std::string compiler_ = "g++";  // Default compiler
    std::mutex commandMutex_;

    /**
     * @brief Retrieves all source files from the specified source directory.
     *
     * @return A vector of source file paths.
     * @throws std::runtime_error if the source directory does not exist.
     */
    std::vector<std::string> getSourceFiles() {
        LOG_F(INFO, "Scanning source directory: {}", sourceDir_);
        std::vector<std::string> sourceFiles;
        if (!std::filesystem::exists(sourceDir_)) {
            LOG_F(ERROR, "Source directory does not exist: {}", sourceDir_);
            throw std::runtime_error("Source directory does not exist: " +
                                     sourceDir_);
        }
        for (const auto& entry :
             std::filesystem::recursive_directory_iterator(sourceDir_)) {
            if (entry.is_regular_file()) {
                const auto& path = entry.path();
                for (const auto& ext : extensions_) {
                    if (path.extension() == ext) {
                        sourceFiles.emplace_back(path.string());
                        LOG_F(INFO, "Found source file: {}", path.string());
                        break;
                    }
                }
            }
        }
        LOG_F(INFO, "Total source files found: {}", sourceFiles.size());
        return sourceFiles;
    }

    /**
     * @brief Applies configured options to generate a compile command for a
     * specific target and file.
     *
     * @param targetName The target name for which to generate the command.
     * @param filePath The source file path.
     * @return The generated compile command string.
     * @throws std::invalid_argument if required options are missing.
     */
    std::string applyOptions(const std::string& targetName,
                             const std::string& filePath) {
        LOG_F(INFO, "Applying options for target: {}, file: {}", targetName,
              filePath);
        std::lock_guard<std::mutex> lock(commandMutex_);
        std::string command = commandTemplate_;

        // Replace placeholders with actual values
        std::unordered_map<std::string, std::string> placeholders = {
            {"compiler", compiler_},
            {"file", filePath},
            {"directory", options_.at("directory")},
            {"output", options_.at("output")}};

        for (const auto& [key, value] : placeholders) {
            command = std::regex_replace(
                command, std::regex("\\{" + key + "\\}"), value);
        }

        // Apply global options
        for (const auto& [key, value] : options_) {
            if (key != "compiler" && key != "file" && key != "directory" &&
                key != "output") {
                command = std::regex_replace(
                    command, std::regex("\\{" + key + "\\}"), value);
            }
        }

        // Apply target-specific options
        auto targetIt = targetOptions_.find(targetName);
        if (targetIt != targetOptions_.end()) {
            for (const auto& [key, value] : targetIt->second) {
                command = std::regex_replace(
                    command, std::regex("\\{" + key + "\\}"), value);
            }
        }

        // Apply conditional options
        for (const auto& [condition, opts] : conditionalOptions_) {
            if (evaluateCondition(condition)) {
                for (const auto& [key, value] : opts) {
                    command = std::regex_replace(
                        command, std::regex("\\{" + key + "\\}"), value);
                }
            }
        }

        // Append defines
        for (const auto& define : defines_) {
            command += " -D" + define;
        }

        // Append flags
        for (const auto& flag : flags_) {
            command += " " + flag;
        }

        // Append libraries
        for (const auto& lib : libraries_) {
            command += " -l" + lib;
        }

        LOG_F(INFO, "Generated command for target {}: {}", targetName, command);
        return command;
    }

    /**
     * @brief Evaluates a condition string.
     *
     * @param condition The condition to evaluate.
     * @return True if the condition is met, false otherwise.
     *
     * @note Currently supports simple environment variable checks.
     */
    static bool evaluateCondition(const std::string& condition) {
        LOG_F(INFO, "Evaluating condition: {}", condition);
        // Example implementation: check if an environment variable is set
        if (condition.find("ENV:") == 0) {
            std::string envVar = condition.substr(4);
            // getenv is not thread-safe; consider using thread-safe
            // alternatives if available
            const char* val = std::getenv(envVar.c_str());
            return val != nullptr;
        }
        LOG_F(WARNING, "Unsupported condition format: {}", condition);
        return false;
    }

    /**
     * @brief Saves the generated compile commands to a JSON file.
     *
     * @param commands The list of compile commands to save.
     * @param outputPath The path to the output JSON file.
     * @throws std::runtime_error if the file cannot be opened.
     */
    static void saveCommandsToFile(const std::vector<CompileCommand>& commands,
                                   const std::string& outputPath) {
        LOG_F(INFO, "Saving compile commands to file: {}", outputPath);
        json jCommands = json::array();
        for (const auto& cmd : commands) {
            jCommands.push_back(cmd.toJson());
        }
        json outputJson = {{"commands", jCommands}};

        std::ofstream ofs(outputPath);
        if (!ofs.is_open()) {
            LOG_F(ERROR, "Failed to open output file: {}", outputPath);
            throw std::runtime_error("Failed to open output file: " +
                                     outputPath);
        }
        ofs << outputJson.dump(4);
        ofs.close();
        LOG_F(INFO, "Compile commands successfully saved to {}", outputPath);
    }

public:
    /**
     * @brief Loads and parses the configuration from a JSON file.
     *
     * @param configPath The path to the configuration JSON file.
     * @throws std::runtime_error if the file cannot be opened or parsed.
     */
    void loadConfiguration(const std::string& configPath) {
        LOG_F(INFO, "Loading configuration from file: {}", configPath);
        std::ifstream ifs(configPath);
        if (!ifs.is_open()) {
            LOG_F(ERROR, "Cannot open configuration file: {}", configPath);
            throw std::runtime_error("Cannot open configuration file: " +
                                     configPath);
        }

        json config;
        try {
            ifs >> config;
        } catch (const json::parse_error& e) {
            LOG_F(ERROR, "JSON parsing error in file {}: {}", configPath,
                  e.what());
            throw std::runtime_error("JSON parsing error in file " +
                                     configPath + ": " + e.what());
        }

        // Set global options
        if (config.contains("options")) {
            for (const auto& [key, value] : config["options"].items()) {
                if (value.is_string()) {
                    setOption(key, value.get<std::string>());
                }
            }
        }

        // Set defines
        if (config.contains("defines") && config["defines"].is_array()) {
            for (const auto& def : config["defines"]) {
                if (def.is_string()) {
                    addDefine(def.get<std::string>());
                }
            }
        }

        // Set flags
        if (config.contains("flags") && config["flags"].is_array()) {
            for (const auto& flag : config["flags"]) {
                if (flag.is_string()) {
                    addFlag(flag.get<std::string>());
                }
            }
        }

        // Set libraries
        if (config.contains("libraries") && config["libraries"].is_array()) {
            for (const auto& lib : config["libraries"]) {
                if (lib.is_string()) {
                    addLibrary(lib.get<std::string>());
                }
            }
        }

        // Set command template
        if (config.contains("commandTemplate") &&
            config["commandTemplate"].is_string()) {
            setCommandTemplate(config["commandTemplate"].get<std::string>());
        }

        // Set compiler
        if (config.contains("compiler") && config["compiler"].is_string()) {
            setCompiler(config["compiler"].get<std::string>());
        }

        // Add targets
        if (config.contains("targets") && config["targets"].is_object()) {
            for (const auto& [targetName, targetConfig] :
                 config["targets"].items()) {
                addTarget(targetName);
                if (targetConfig.contains("options") &&
                    targetConfig["options"].is_object()) {
                    for (const auto& [key, value] :
                         targetConfig["options"].items()) {
                        if (value.is_string()) {
                            setTargetOption(targetName, key,
                                            value.get<std::string>());
                        }
                    }
                }
                if (targetConfig.contains("conditionalOptions") &&
                    targetConfig["conditionalOptions"].is_object()) {
                    for (const auto& [condition, opts] :
                         targetConfig["conditionalOptions"].items()) {
                        if (opts.is_object()) {
                            for (const auto& [key, value] : opts.items()) {
                                if (value.is_string()) {
                                    addConditionalOption(
                                        condition, key,
                                        value.get<std::string>());
                                }
                            }
                        }
                    }
                }
            }
        }

        LOG_F(INFO, "Configuration loaded successfully from {}", configPath);
    }

public:
    /**
     * @brief Sets a global option.
     *
     * @param key The option key.
     * @param value The option value.
     */
    void setOption(const std::string& key, const std::string& value) {
        LOG_F(INFO, "Setting global option: {} = {}", key, value);
        options_[key] = value;
    }

    /**
     * @brief Adds a target.
     *
     * @param targetName The name of the target to add.
     */
    void addTarget(const std::string& targetName) {
        LOG_F(INFO, "Adding target: {}", targetName);
        if (targetOptions_.find(targetName) != targetOptions_.end()) {
            LOG_F(WARNING, "Target {} already exists. Overwriting options.",
                  targetName);
        }
        targetOptions_[targetName] = {};
    }

    /**
     * @brief Sets an option for a specific target.
     *
     * @param targetName The target name.
     * @param key The option key.
     * @param value The option value.
     */
    void setTargetOption(const std::string& targetName, const std::string& key,
                         const std::string& value) {
        LOG_F(INFO, "Setting option for target {}: {} = {}", targetName, key,
              value);
        if (targetOptions_.find(targetName) == targetOptions_.end()) {
            LOG_F(WARNING, "Target {} does not exist. Adding target.",
                  targetName);
            addTarget(targetName);
        }
        targetOptions_[targetName][key] = value;
    }

    /**
     * @brief Adds a conditional option.
     *
     * @param condition The condition string.
     * @param key The option key.
     * @param value The option value.
     */
    void addConditionalOption(const std::string& condition,
                              const std::string& key,
                              const std::string& value) {
        LOG_F(INFO, "Adding conditional option: if ({}) then {} = {}",
              condition, key, value);
        conditionalOptions_[condition][key] = value;
    }

    /**
     * @brief Adds a define directive.
     *
     * @param define The define string.
     */
    void addDefine(const std::string& define) {
        LOG_F(INFO, "Adding define: {}", define);
        defines_.emplace_back(define);
    }

    /**
     * @brief Adds a compiler flag.
     *
     * @param flag The flag string.
     */
    void addFlag(const std::string& flag) {
        LOG_F(INFO, "Adding flag: {}", flag);
        flags_.emplace_back(flag);
    }

    /**
     * @brief Adds a library.
     *
     * @param libraryPath The path to the library.
     */
    void addLibrary(const std::string& libraryPath) {
        LOG_F(INFO, "Adding library: {}", libraryPath);
        libraries_.emplace_back(libraryPath);
    }

    /**
     * @brief Sets the command template.
     *
     * @param templateStr The command template string.
     */
    void setCommandTemplate(const std::string& templateStr) {
        LOG_F(INFO, "Setting command template: {}", templateStr);
        commandTemplate_ = templateStr;
    }

    /**
     * @brief Sets the compiler.
     *
     * @param compiler The compiler string.
     */
    void setCompiler(const std::string& compiler) {
        LOG_F(INFO, "Setting compiler: {}", compiler);
        compiler_ = compiler;
    }

    /**
     * @brief Generates compile commands and saves them to the specified output
     * path.
     */
    void generate() {
        LOG_F(INFO, "Starting generation of compile commands.");
        try {
            auto sourceFiles = getSourceFiles();
            if (sourceFiles.empty()) {
                LOG_F(WARNING, "No source files found in directory: {}",
                      sourceDir_);
                return;
            }

            std::vector<CompileCommand> commands;
            commands.reserve(sourceFiles.size());

            // Determine output path
            std::string outputPath;
            if (options_.find("outputPath") != options_.end()) {
                outputPath = options_.at("outputPath");
            } else {
                LOG_F(ERROR,
                      "Output path not specified. Set 'outputPath' option.");
                throw std::runtime_error(
                    "Output path not specified. Set 'outputPath' option.");
            }

            // Generate compile commands for each target
            for (const auto& [targetName, _] : targetOptions_) {
                for (const auto& file : sourceFiles) {
                    std::string command = applyOptions(targetName, file);
                    commands.emplace_back(options_.at("directory"), command,
                                          file);
                }
            }

            // Handle default target if no targets are defined
            if (targetOptions_.empty()) {
                for (const auto& file : sourceFiles) {
                    std::string command = applyOptions("default", file);
                    commands.emplace_back(options_.at("directory"), command,
                                          file);
                }
            }

            // Save commands to file
            saveCommandsToFile(commands, outputPath);
            LOG_F(INFO, "Compile commands generated successfully.");
        } catch (const std::exception& e) {
            LOG_F(ERROR, "Error during compile command generation: {}",
                  e.what());
            throw;  // Re-throw exception after logging
        }
    }
};

/**
 * @brief Constructs a CompileCommandGenerator instance.
 */
CompileCommandGenerator::CompileCommandGenerator()
    : impl_(std::make_unique<Impl>()) {
    LOG_F(INFO, "CompileCommandGenerator instance created.");
}

/**
 * @brief Destructs the CompileCommandGenerator instance.
 */
CompileCommandGenerator::~CompileCommandGenerator() {
    LOG_F(INFO, "CompileCommandGenerator instance destroyed.");
}

void CompileCommandGenerator::loadConfigFromFile(
    const std::string& configPath) {
    impl_->loadConfiguration(configPath);
}

void CompileCommandGenerator::generate() { impl_->generate(); }

}  // namespace lithium