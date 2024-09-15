#ifndef LITHIUM_ADDON_TOOLCHAIN_HPP
#define LITHIUM_ADDON_TOOLCHAIN_HPP

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class Toolchain {
public:
    enum class Type { Compiler, BuildTool, Unknown };

    Toolchain(std::string name, std::string compiler, std::string buildTool,
              std::string version, std::string path, Type type = Type::Unknown);
    ~Toolchain();
    Toolchain(const Toolchain& other);
    Toolchain(Toolchain&& other) noexcept;
    auto operator=(const Toolchain& other) -> Toolchain&;
    auto operator=(Toolchain&& other) noexcept -> Toolchain&;

    void displayInfo() const;
    [[nodiscard]] auto getName() const -> const std::string&;
    [[nodiscard]] auto getCompiler() const -> const std::string&;
    [[nodiscard]] auto getBuildTool() const -> const std::string&;
    [[nodiscard]] auto getVersion() const -> const std::string&;
    [[nodiscard]] auto getPath() const -> const std::string&;
    [[nodiscard]] auto getType() const -> Type;

    void setVersion(const std::string& version);
    void setPath(const std::string& path);
    void setType(Type type);

    [[nodiscard]] auto isCompatibleWith(const Toolchain& other) const -> bool;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

class ToolchainManager {
public:
    using ToolchainFilter = std::function<bool(const Toolchain&)>;

    ToolchainManager();
    ~ToolchainManager();
    ToolchainManager(const ToolchainManager&) = delete;
    ToolchainManager& operator=(const ToolchainManager&) = delete;
    ToolchainManager(ToolchainManager&&) noexcept;
    ToolchainManager& operator=(ToolchainManager&&) noexcept;

    void scanForToolchains();
    void listToolchains() const;
    [[nodiscard]] auto selectToolchain(const std::string& name) const
        -> std::optional<Toolchain>;
    void saveConfig(const std::string& filename) const;
    void loadConfig(const std::string& filename);
    [[nodiscard]] auto getToolchains() const -> const std::vector<Toolchain>&;
    [[nodiscard]] auto getAvailableCompilers() const
        -> std::vector<std::string>;
    [[nodiscard]] auto getAvailableBuildTools() const
        -> std::vector<std::string>;
    void addToolchain(const Toolchain& toolchain);
    void removeToolchain(const std::string& name);
    void updateToolchain(const std::string& name,
                         const Toolchain& updatedToolchain);
    [[nodiscard]] auto findToolchain(const std::string& name) const
        -> std::optional<Toolchain>;
    [[nodiscard]] auto findToolchains(const ToolchainFilter& filter) const
        -> std::vector<Toolchain>;
    [[nodiscard]] auto suggestCompatibleToolchains(const Toolchain& base) const
        -> std::vector<Toolchain>;
    void registerCustomToolchain(const std::string& name,
                                 const std::string& path);
    void setDefaultToolchain(const std::string& name);
    [[nodiscard]] auto getDefaultToolchain() const -> std::optional<Toolchain>;
    void addSearchPath(const std::string& path);
    void removeSearchPath(const std::string& path);
    [[nodiscard]] auto getSearchPaths() const
        -> const std::vector<std::string>&;
    void setToolchainAlias(const std::string& alias,
                           const std::string& toolchainName);
    [[nodiscard]] auto getToolchainByAlias(const std::string& alias) const
        -> std::optional<Toolchain>;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

#endif  // LITHIUM_ADDON_TOOLCHAIN_HPP