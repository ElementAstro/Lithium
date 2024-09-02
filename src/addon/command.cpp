#include "command.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <nlohmann/json.hpp>
#include <thread>

using json = nlohmann::json;

struct CompileCommand {
    std::string directory;
    std::string command;
    std::string file;

    [[nodiscard]] auto toJson() const -> json {
        return json{
            {"directory", directory}, {"command", command}, {"file", file}};
    }
};

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

    std::vector<std::string> getSourceFiles() {
        std::vector<std::string> sourceFiles;
        for (const auto& entry :
             std::filesystem::directory_iterator(sourceDir)) {
            if (entry.is_regular_file()) {
                const auto& path = entry.path();
                for (const auto& ext : extensions) {
                    if (path.extension() == ext) {
                        sourceFiles.push_back(path.string());
                    }
                }
            }
        }
        return sourceFiles;
    }

    std::vector<CompileCommand> parseExistingCommands() {
        std::vector<CompileCommand> commands;
        std::ifstream ifs(existingCommandsPath);

        if (ifs.is_open()) {
            json j;
            ifs >> j;
            for (const auto& cmd : j["commands"]) {
                commands.emplace_back(cmd["directory"].get<std::string>(),
                                    cmd["command"].get<std::string>(),
                                    cmd["file"].get<std::string>());
            }
            ifs.close();
        } else {
            std::cerr << "Could not open " << existingCommandsPath << std::endl;
        }

        return commands;
    }

    void generateCompileCommand(const std::string& file, json& j_commands) {
        std::string command =
            compiler + " " + includeFlag + " " + outputFlag + " " + file;
        CompileCommand cmd{sourceDir, command, file};

        std::lock_guard lock(output_mutex);
        j_commands.push_back(cmd.toJson());
    }

    void saveCommandsToFile(const json& j) {
        std::ofstream ofs(outputPath);
        if (ofs.is_open()) {
            ofs << j.dump(4);
            ofs.close();
            std::cout << "compile_commands.json generated successfully at "
                      << outputPath << "." << std::endl;
        } else {
            std::cerr << "Failed to create compile_commands.json." << std::endl;
        }
    }
};

CompileCommandGenerator::CompileCommandGenerator() : pImpl(new Impl) {}

CompileCommandGenerator::~CompileCommandGenerator() { delete pImpl; }

void CompileCommandGenerator::setSourceDir(const std::string& dir) {
    pImpl->sourceDir = dir;
}

void CompileCommandGenerator::setCompiler(const std::string& compiler) {
    pImpl->compiler = compiler;
}

void CompileCommandGenerator::setIncludeFlag(const std::string& flag) {
    pImpl->includeFlag = flag;
}

void CompileCommandGenerator::setOutputFlag(const std::string& flag) {
    pImpl->outputFlag = flag;
}

void CompileCommandGenerator::setProjectName(const std::string& name) {
    pImpl->projectName = name;
}

void CompileCommandGenerator::setProjectVersion(const std::string& version) {
    pImpl->projectVersion = version;
}

void CompileCommandGenerator::addExtension(const std::string& ext) {
    pImpl->extensions.push_back(ext);
}

void CompileCommandGenerator::setOutputPath(const std::string& path) {
    pImpl->outputPath = path;
}

void CompileCommandGenerator::setExistingCommandsPath(const std::string& path) {
    pImpl->existingCommandsPath = path;
}

void CompileCommandGenerator::generate() {
    std::vector<CompileCommand> commands;

    // 解析现有的 compile_commands.json
    if (!pImpl->existingCommandsPath.empty()) {
        auto existing_commands = pImpl->parseExistingCommands();
        commands.insert(commands.end(), existing_commands.begin(),
                        existing_commands.end());
    }

    auto source_files = pImpl->getSourceFiles();
    json j_commands = json::array();

    // 多线程处理构建命令
    std::vector<std::thread> threads;
    for (const auto& file : source_files) {
        threads.emplace_back(&Impl::generateCompileCommand, pImpl, file,
                             std::ref(j_commands));
    }

    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }

    // 构建最终的 JSON
    json j = {{"version", 4},
              {"project_name", pImpl->projectName},
              {"project_version", pImpl->projectVersion},
              {"commands", j_commands}};

    // 保存到文件
    pImpl->saveCommandsToFile(j);
}
