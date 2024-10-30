#include "compile_command_generator.hpp"

#include <filesystem>
#include <fstream>
#include <mutex>
#include <regex>
#include <utility>

#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"

using json = nlohmann::json;

namespace lithium {

constexpr int K_ALIGNMENT = 128;

struct alignas(K_ALIGNMENT) CompileCommand {
private:
    std::string directory_;
    std::string command_;
    std::string file_;

public:
    [[nodiscard]] auto toJson() const -> json {
        return json{
            {"directory", directory_}, {"command", command_}, {"file", file_}};
    }

    void fromJson(const json& json_obj) {
        directory_ = json_obj["directory"].get<std::string>();
        command_ = json_obj["command"].get<std::string>();
        file_ = json_obj["file"].get<std::string>();
    }

    CompileCommand() = default;
    CompileCommand(std::string directory, std::string command, std::string file)
        : directory_(std::move(directory)),
          command_(std::move(command)),
          file_(std::move(file)) {}
};

struct alignas(K_ALIGNMENT) CompileCommandGenerator::Impl {
    std::string sourceDir = "./src";
    std::vector<std::string> extensions = {".cpp", ".c"};
    std::unordered_map<std::string, std::string> options;
    std::unordered_map<std::string,
                       std::unordered_map<std::string, std::string>>
        targetOptions;
    std::unordered_map<std::string,
                       std::unordered_map<std::string, std::string>>
        conditionalOptions;
    std::vector<std::string> defines;
    std::vector<std::string> flags;
    std::vector<std::string> libraries;
    std::string commandTemplate = "{compiler} {include} {output} {file}";
    std::string compiler = "g++";  // 默认编译器
    std::mutex commandMutex;

    auto getSourceFiles() -> std::vector<std::string> {
        LOG_F(INFO, "Scanning source directory: {}", sourceDir);
        std::vector<std::string> source_files;
        for (const auto& entry :
             std::filesystem::directory_iterator(sourceDir)) {
            if (entry.is_regular_file()) {
                const auto& path = entry.path();
                for (const auto& ext : extensions) {
                    if (path.extension() == ext) {
                        source_files.push_back(path.string());
                        LOG_F(INFO, "Found source file: {}", path.string());
                    }
                }
            }
        }
        LOG_F(INFO, "Total source files found: {}", source_files.size());
        return source_files;
    }

    auto applyOptions(const std::string& target_name,
                      const std::string& file_path) -> std::string {
        LOG_F(INFO, "Applying options for target: {}, file: {}", target_name,
              file_path);
        std::string command = commandTemplate;

        // 处理通用选项、目标选项和条件选项
        options["file"] = file_path;
        options["compiler"] = compiler;  // 使用设置的编译器
        auto& targetOpts = targetOptions[target_name];
        for (const auto& [key, value] : options) {
            command = std::regex_replace(
                command, std::regex("\\{" + key + "\\}"), value);
        }
        for (const auto& [key, value] : targetOpts) {
            command = std::regex_replace(
                command, std::regex("\\{" + key + "\\}"), value);
        }

        // 处理宏定义和编译标志
        for (const auto& define : defines) {
            command += " -D" + define;
        }
        for (const auto& flag : flags) {
            command += " " + flag;
        }

        // 添加库依赖
        for (const auto& lib : libraries) {
            command += " -l" + lib;
        }

        LOG_F(INFO, "Generated command: {}", command);
        return command;
    }

    static void saveCommandsToFile(const std::vector<CompileCommand>& commands,
                                   const std::string& output_path) {
        LOG_F(INFO, "Saving commands to file: {}", output_path);
        json jCommands = json::array();
        for (const auto& cmd : commands) {
            jCommands.push_back(cmd.toJson());
        }
        std::ofstream ofs(output_path);
        if (ofs.is_open()) {
            ofs << json{{"commands", jCommands}}.dump(4);
            LOG_F(INFO, "Commands successfully saved to {}", output_path);
        } else {
            LOG_F(ERROR, "Failed to open file: {}", output_path);
        }
    }
};

CompileCommandGenerator::CompileCommandGenerator()
    : impl_(std::make_unique<Impl>()) {
    LOG_F(INFO, "CompileCommandGenerator created");
}

CompileCommandGenerator::~CompileCommandGenerator() {
    LOG_F(INFO, "CompileCommandGenerator destroyed");
}

auto CompileCommandGenerator::setOption(const std::string& key,
                                        const std::string& value)
    -> CompileCommandGenerator& {
    LOG_F(INFO, "Setting option: {} = {}", key, value);
    impl_->options[key] = value;
    return *this;
}

auto CompileCommandGenerator::addTarget(const std::string& target_name)
    -> CompileCommandGenerator& {
    LOG_F(INFO, "Adding target: {}", target_name);
    impl_->targetOptions[target_name] = {};
    return *this;
}

auto CompileCommandGenerator::setTargetOption(
    const std::string& target_name, const std::string& key,
    const std::string& value) -> CompileCommandGenerator& {
    LOG_F(INFO, "Setting target option for target: {}, {} = {}", target_name,
          key, value);
    impl_->targetOptions[target_name][key] = value;
    return *this;
}

auto CompileCommandGenerator::addConditionalOption(
    const std::string& condition, const std::string& key,
    const std::string& value) -> CompileCommandGenerator& {
    LOG_F(INFO, "Adding conditional option: if {} then {} = {}", condition, key,
          value);
    impl_->conditionalOptions[condition][key] = value;
    return *this;
}

auto CompileCommandGenerator::addDefine(const std::string& define)
    -> CompileCommandGenerator& {
    LOG_F(INFO, "Adding define: {}", define);
    impl_->defines.push_back(define);
    return *this;
}

auto CompileCommandGenerator::addFlag(const std::string& flag)
    -> CompileCommandGenerator& {
    LOG_F(INFO, "Adding flag: {}", flag);
    impl_->flags.push_back(flag);
    return *this;
}

auto CompileCommandGenerator::addLibrary(const std::string& library_path)
    -> CompileCommandGenerator& {
    LOG_F(INFO, "Adding library: {}", library_path);
    impl_->libraries.push_back(library_path);
    return *this;
}

auto CompileCommandGenerator::setCommandTemplate(
    const std::string& template_str) -> CompileCommandGenerator& {
    LOG_F(INFO, "Setting command template: {}", template_str);
    impl_->commandTemplate = template_str;
    return *this;
}

auto CompileCommandGenerator::setCompiler(const std::string& compiler)
    -> CompileCommandGenerator& {
    LOG_F(INFO, "Setting compiler: {}", compiler);
    impl_->compiler = compiler;
    return *this;
}

void CompileCommandGenerator::loadConfigFromFile(
    const std::string& config_path) {
    LOG_F(INFO, "Loading config from file: {}", config_path);
    std::ifstream ifs(config_path);
    if (ifs.is_open()) {
        json config;
        ifs >> config;
        for (const auto& [key, value] : config.items()) {
            if (value.is_string()) {
                setOption(key, value.get<std::string>());
            } else if (key == "defines" && value.is_array()) {
                for (const auto& def : value) {
                    addDefine(def.get<std::string>());
                }
            } else if (key == "libraries" && value.is_array()) {
                for (const auto& lib : value) {
                    addLibrary(lib.get<std::string>());
                }
            }
        }
        LOG_F(INFO, "Config successfully loaded from {}", config_path);
    } else {
        LOG_F(ERROR, "Failed to open config file: {}", config_path);
    }
}

void CompileCommandGenerator::generate() {
    LOG_F(INFO, "Generating compile commands");
    auto source_files = impl_->getSourceFiles();
    std::vector<CompileCommand> commands;
    commands.reserve(source_files.size());
    for (const auto& file : source_files) {
        commands.emplace_back(impl_->sourceDir,
                              impl_->applyOptions("default", file), file);
    }
    Impl::saveCommandsToFile(commands, impl_->options["outputPath"]);
    LOG_F(INFO, "Compile commands generation complete");
}

}  // namespace lithium