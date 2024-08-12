#include "toolchain.hpp"

#include <array>
#include <filesystem>
#include <fstream>
#include <memory>

#include "atom/log/loguru.hpp"

Toolchain::Toolchain(std::string name, std::string compiler,
                     std::string buildTool, std::string version,
                     std::string path)
    : name_(std::move(name)),
      compiler_(std::move(compiler)),
      buildTool_(std::move(buildTool)),
      version_(std::move(version)),
      path_(std::move(path)) {}

void Toolchain::displayInfo() const {
    LOG_F(INFO, "Toolchain Information for {}", name_);
    LOG_F(INFO, "Compiler: {}", compiler_);
    LOG_F(INFO, "Build Tool: {}", buildTool_);
    LOG_F(INFO, "Version: {}", version_);
    LOG_F(INFO, "Path: {}", path_);
}

auto Toolchain::getName() const -> const std::string& { return name_; }

void ToolchainManager::scanForToolchains() {
    std::vector<std::string> searchPaths;

#if defined(_WIN32) || defined(_WIN64)
    searchPaths = {"C:\\Program Files",        "C:\\Program Files (x86)",
                   "C:\\MinGW\\bin",           "C:\\LLVM\\bin",
                   "C:\\msys64\\mingw64\\bin", "C:\\msys64\\mingw32\\bin",
                   "C:\\msys64\\clang64\\bin", "C:\\msys64\\clang32\\bin"};
#else
    searchPaths = {"/usr/bin", "/usr/local/bin"};
#endif

    for (const auto& path : searchPaths) {
        if (std::filesystem::exists(path)) {
            for (const auto& entry :
                 std::filesystem::directory_iterator(path)) {
                if (entry.is_regular_file()) {
                    std::string filename = entry.path().filename().string();
                    if (filename.starts_with("gcc") ||
                        filename.starts_with("g++") ||
                        filename.starts_with("clang") ||
                        filename.starts_with("clang++")) {
                        std::string version =
                            getCompilerVersion(entry.path().string());
                        toolchains_.emplace_back(filename, filename, filename,
                                                 version,
                                                 entry.path().string());
                    }
                }
            }
        }
    }

    scanBuildTools();
}

void ToolchainManager::scanBuildTools() {
    std::vector<std::string> buildTools = {"make", "ninja", "cmake", "gmake",
                                           "msbuild"};

    for (const auto& tool : buildTools) {
        if (std::filesystem::exists(tool)) {
            std::string version = getCompilerVersion(tool);
            toolchains_.emplace_back(tool, tool, tool, version, tool);
        }
    }
}

void ToolchainManager::listToolchains() const {
    LOG_F(INFO, "Available Toolchains:");
    for (const auto& tc : toolchains_) {
        LOG_F(INFO, "- {}", tc.getName());
    }
}

bool ToolchainManager::selectToolchain(const std::string& name) const {
    for (const auto& tc : toolchains_) {
        if (tc.getName() == name) {
            tc.displayInfo();
            return true;
        }
    }
    return false;
}

void ToolchainManager::saveConfig(const std::string& filename) {
    std::ofstream file(filename);
    for (const auto& tc : toolchains_) {
        file << tc.getName() << "\n";
    }
    file.close();
    LOG_F(INFO, "Configuration saved to {}", filename);
}

void ToolchainManager::loadConfig(const std::string& filename) {
    std::ifstream file(filename);
    std::string toolchainName;
    while (std::getline(file, toolchainName)) {
        selectToolchain(toolchainName);
    }
    file.close();
}

std::string ToolchainManager::getCompilerVersion(const std::string& path) {
    std::string command = path + " --version";
#if defined(_WIN32) || defined(_WIN64)
    command = "\"" + path + "\"" + " --version";
#endif

    std::array<char, 128> buffer;
    std::string result;

    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"),
                                                  pclose);
    if (!pipe) {
        return "Unknown version";
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return result.empty() ? "Unknown version" : result;
}

std::vector<std::string> ToolchainManager::getAvailableCompilers() const {
    std::vector<std::string> compilers;
    for (const auto& toolchain : toolchains_) {
        compilers.push_back(toolchain.getName()); // 收集每个可用工具链的名称
    }
    return compilers;
}
