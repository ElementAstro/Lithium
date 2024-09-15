#include "command.hpp"

#include <atomic>
#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>
#include <ranges>

#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"
using json = nlohmann::json;

#include "macro.hpp"

namespace lithium {
struct CompileCommand {
    std::string directory;
    std::string command;
    std::string file;

    [[nodiscard]] auto toJson() const -> json {
        return json{
            {"directory", directory}, {"command", command}, {"file", file}};
    }

    void fromJson(const json& j) {
        directory = j["directory"].get<std::string>();
        command = j["command"].get<std::string>();
        file = j["file"].get<std::string>();
    }
} ATOM_ALIGNAS(128);

struct CompileCommandGenerator::Impl {
    std::string sourceDir = "./src";
    std::vector<std::string> extensions = {".cpp", ".c"};
    std::string compiler = "g++ -std=c++20";
    std::string includeFlag = "-I./include";
    std::string outputFlag = "-o output";
    std::string outputPath = "compile_commands.json";
    std::string existingCommandsPath;
    std::string projectName = "MyProject";
    std::string projectVersion = "1.0.0";
    std::mutex output_mutex;
    std::atomic<int> commandCounter{0};

    // 获取源文件
    auto getSourceFiles() -> std::vector<std::string> {
        LOG_F(INFO, "Scanning source directory: {}",
              sourceDir);  // Log scanning process
        std::vector<std::string> sourceFiles;
        for (const auto& entry :
             std::filesystem::directory_iterator(sourceDir)) {
            if (entry.is_regular_file()) {
                const auto& path = entry.path();
                for (const auto& ext : extensions) {
                    if (path.extension() == ext) {
                        sourceFiles.push_back(path.string());
                        LOG_F(INFO, "Found source file: {}",
                              path.string());  // Log found file
                    }
                }
            }
        }
        LOG_F(INFO, "Total source files found: {}", sourceFiles.size());
        return sourceFiles;
    }

    [[nodiscard]] auto parseExistingCommands() const
        -> std::vector<CompileCommand> {
        std::vector<CompileCommand> commands;
        if (existingCommandsPath.empty() ||
            !std::filesystem::exists(existingCommandsPath)) {
            LOG_F(WARNING, "No existing compile commands found at {}",
                  existingCommandsPath);
            return commands;
        }

        LOG_F(INFO, "Parsing existing compile commands from {}",
              existingCommandsPath);
        std::ifstream ifs(existingCommandsPath);
        if (ifs.is_open()) {
            json j;
            ifs >> j;
            for (const auto& cmd : j["commands"]) {
                CompileCommand c;
                c.fromJson(cmd);
                commands.push_back(c);
            }
            ifs.close();
            LOG_F(INFO, "Parsed {} existing compile commands", commands.size());
        } else {
            LOG_F(ERROR, "Failed to open {}", existingCommandsPath);
        }
        return commands;
    }

    void generateCompileCommand(const std::string& file, json& j_commands) {
        std::string command =
            compiler + " " + includeFlag + " " + outputFlag + " " + file;
        CompileCommand cmd{sourceDir, command, file};

        LOG_F(INFO, "Generating compile command for file: {}", file);
        std::lock_guard lock(output_mutex);
        j_commands.push_back(cmd.toJson());
        int currentCount =
            commandCounter.fetch_add(1, std::memory_order_relaxed) + 1;
        LOG_F(INFO, "Total commands generated so far: {}", currentCount);
    }

    void saveCommandsToFile(const json& j) {
        LOG_F(INFO, "Saving compile commands to file: {}", outputPath);
        std::ofstream ofs(outputPath);
        if (ofs.is_open()) {
            ofs << j.dump(4);
            ofs.close();
            LOG_F(INFO,
                  "compile_commands.json generated successfully with {} "
                  "commands at {}.",
                  commandCounter.load(std::memory_order_relaxed),
                  outputPath);  // Log success
        } else {
            LOG_F(ERROR, "Failed to open {} for writing.", outputPath);
        }
    }
} ATOM_ALIGNAS(128);

CompileCommandGenerator::CompileCommandGenerator()
    : impl_(std::make_unique<Impl>()) {}

CompileCommandGenerator::~CompileCommandGenerator() = default;

void CompileCommandGenerator::setSourceDir(const std::string& dir) {
    LOG_F(INFO, "Setting source directory to {}",
          dir);  // Log set source directory
    impl_->sourceDir = dir;
}

void CompileCommandGenerator::setCompiler(const std::string& compiler) {
    LOG_F(INFO, "Setting compiler to {}", compiler);
    impl_->compiler = compiler;
}

void CompileCommandGenerator::setIncludeFlag(const std::string& flag) {
    LOG_F(INFO, "Setting include flag to {}", flag);
    impl_->includeFlag = flag;
}

void CompileCommandGenerator::setOutputFlag(const std::string& flag) {
    LOG_F(INFO, "Setting output flag to {}", flag);
    impl_->outputFlag = flag;
}

void CompileCommandGenerator::setProjectName(const std::string& name) {
    LOG_F(INFO, "Setting project name to {}", name);
    impl_->projectName = name;
}

void CompileCommandGenerator::setProjectVersion(const std::string& version) {
    LOG_F(INFO, "Setting project version to {}",
          version);  // Log set project version
    impl_->projectVersion = version;
}

void CompileCommandGenerator::addExtension(const std::string& ext) {
    LOG_F(INFO, "Adding file extension: {}", ext);
    impl_->extensions.push_back(ext);
}

void CompileCommandGenerator::setOutputPath(const std::string& path) {
    LOG_F(INFO, "Setting output path to {}", path);
    impl_->outputPath = path;
}

void CompileCommandGenerator::setExistingCommandsPath(const std::string& path) {
    LOG_F(INFO, "Setting existing commands path to {}",
          path);  // Log set existing commands path
    impl_->existingCommandsPath = path;
}

void CompileCommandGenerator::generate() {
    LOG_F(INFO, "Starting compile command generation");
    std::vector<CompileCommand> commands;

    // 解析现有的 compile_commands.json
    if (!impl_->existingCommandsPath.empty()) {
        auto existingCommands = impl_->parseExistingCommands();
        commands.insert(commands.end(), existingCommands.begin(),
                        existingCommands.end());
    }

    auto sourceFiles = impl_->getSourceFiles();
    json jCommands = json::array();

    LOG_F(INFO, "Generating compile commands for {} source files",
          sourceFiles.size());  // Log number of files
    std::ranges::for_each(sourceFiles.begin(), sourceFiles.end(),
                          [&](const std::string& file) {
                              impl_->generateCompileCommand(file, jCommands);
                          });

    // 构建最终的 JSON
    json j = {{"version", 4},
              {"project_name", impl_->projectName},
              {"project_version", impl_->projectVersion},
              {"commands", jCommands}};

    // 保存到文件
    impl_->saveCommandsToFile(j);
    LOG_F(INFO, "Compile command generation complete");
}

}  // namespace lithium
