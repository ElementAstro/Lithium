#ifndef LITHIUM_ADDON_TOOLCHAIN_HPP
#define LITHIUM_ADDON_TOOLCHAIN_HPP

#include <cstdlib>
#include <string>
#include <vector>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#define CMD_EXTENSION ".exe"
#else
#define CMD_EXTENSION ""
#endif

class Toolchain {
public:
    Toolchain(std::string name, std::string compiler, std::string buildTool,
              std::string version, std::string path);

    void displayInfo() const;

    [[nodiscard]] auto getName() const -> const std::string&;

private:
    std::string name_;
    std::string compiler_;
    std::string buildTool_;
    std::string version_;
    std::string path_;
};

class ToolchainManager {
public:
    void scanForToolchains();
    void listToolchains() const;
    [[nodiscard]] auto selectToolchain(const std::string& name) const -> bool;
    void saveConfig(const std::string& filename);
    void loadConfig(const std::string& filename);

    [[nodiscard]] auto getToolchains() const -> const std::vector<Toolchain>& {
        return toolchains_;
    }

    auto getAvailableCompilers() const -> std::vector<std::string>;

private:
    std::vector<Toolchain> toolchains_;

    auto getCompilerVersion(const std::string& path) -> std::string;
    void scanBuildTools();
};

#endif  // LITHIUM_ADDON_TOOLCHAIN_HPP
