#ifndef LITHIUM_ADDON_TOOLCHAIN_HPP
#define LITHIUM_ADDON_TOOLCHAIN_HPP

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

/**
 * @class Toolchain
 * @brief Represents a toolchain used for building software.
 */
class Toolchain {
public:
    /**
     * @enum ToolchainType
     * @brief Enum representing the type of the toolchain.
     */
    enum class ToolchainType { Compiler, BuildTool, Unknown };

    /**
     * @brief Constructs a Toolchain object.
     * @param name The name of the toolchain.
     * @param compiler The compiler used by the toolchain.
     * @param buildTool The build tool used by the toolchain.
     * @param version The version of the toolchain.
     * @param path The installation path of the toolchain.
     * @param type The type of the toolchain.
     */
    Toolchain(std::string name, std::string compiler, std::string buildTool,
              std::string version, std::string path, ToolchainType type = ToolchainType::Unknown);

    /**
     * @brief Destructor for the Toolchain object.
     */
    ~Toolchain();

    /**
     * @brief Copy constructor for the Toolchain object.
     * @param other The Toolchain object to copy from.
     */
    Toolchain(const Toolchain& other);

    /**
     * @brief Move constructor for the Toolchain object.
     * @param other The Toolchain object to move from.
     */
    Toolchain(Toolchain&& other) noexcept;

    /**
     * @brief Copy assignment operator for the Toolchain object.
     * @param other The Toolchain object to copy from.
     * @return A reference to the assigned Toolchain object.
     */
    auto operator=(const Toolchain& other) -> Toolchain&;

    /**
     * @brief Move assignment operator for the Toolchain object.
     * @param other The Toolchain object to move from.
     * @return A reference to the assigned Toolchain object.
     */
    auto operator=(Toolchain&& other) noexcept -> Toolchain&;

    /**
     * @brief Displays information about the toolchain.
     */
    void displayInfo() const;

    /**
     * @brief Gets the name of the toolchain.
     * @return A constant reference to the name of the toolchain.
     */
    [[nodiscard]] auto getName() const -> const std::string&;

    /**
     * @brief Gets the compiler used by the toolchain.
     * @return A constant reference to the compiler used by the toolchain.
     */
    [[nodiscard]] auto getCompiler() const -> const std::string&;

    /**
     * @brief Gets the build tool used by the toolchain.
     * @return A constant reference to the build tool used by the toolchain.
     */
    [[nodiscard]] auto getBuildTool() const -> const std::string&;

    /**
     * @brief Gets the version of the toolchain.
     * @return A constant reference to the version of the toolchain.
     */
    [[nodiscard]] auto getVersion() const -> const std::string&;

    /**
     * @brief Gets the installation path of the toolchain.
     * @return A constant reference to the installation path of the toolchain.
     */
    [[nodiscard]] auto getPath() const -> const std::string&;

    /**
     * @brief Gets the type of the toolchain.
     * @return The type of the toolchain.
     */
    [[nodiscard]] auto getType() const -> ToolchainType;

    /**
     * @brief Sets the version of the toolchain.
     * @param version The new version of the toolchain.
     */
    void setVersion(const std::string& version);

    /**
     * @brief Sets the installation path of the toolchain.
     * @param path The new installation path of the toolchain.
     */
    void setPath(const std::string& path);

    /**
     * @brief Sets the type of the toolchain.
     * @param type The new type of the toolchain.
     */
    void setType(ToolchainType type);

    /**
     * @brief Checks if the toolchain is compatible with another toolchain.
     * @param other The other toolchain to check compatibility with.
     * @return True if the toolchains are compatible, false otherwise.
     */
    [[nodiscard]] auto isCompatibleWith(const std::shared_ptr<Toolchain>& other) const -> bool;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

/**
 * @class ToolchainManager
 * @brief Manages a collection of toolchains.
 */
class ToolchainManager {
public:
    using ToolchainFilter = std::function<bool(const std::shared_ptr<Toolchain>&)>;

    /**
     * @brief Constructs a ToolchainManager object.
     */
    ToolchainManager();

    /**
     * @brief Destructor for the ToolchainManager object.
     */
    ~ToolchainManager();

    /**
     * @brief Move constructor for the ToolchainManager object.
     * @param other The ToolchainManager object to move from.
     */
    ToolchainManager(ToolchainManager&&) noexcept;

    /**
     * @brief Move assignment operator for the ToolchainManager object.
     * @param other The ToolchainManager object to move from.
     * @return A reference to the assigned ToolchainManager object.
     */
    ToolchainManager& operator=(ToolchainManager&&) noexcept;

    /**
     * @brief Scans for available toolchains.
     */
    void scanForToolchains();

    /**
     * @brief Lists all available toolchains.
     */
    void listToolchains() const;

    /**
     * @brief Selects a toolchain by name.
     * @param name The name of the toolchain to select.
     * @return An optional Toolchain object if found.
     */
    [[nodiscard]] auto selectToolchain(const std::string& name) const
        -> std::optional<std::shared_ptr<Toolchain>>;

    /**
     * @brief Saves the current configuration to a file.
     * @param filename The name of the file to save the configuration to.
     */
    void saveConfig(const std::string& filename) const;

    /**
     * @brief Loads a configuration from a file.
     * @param filename The name of the file to load the configuration from.
     */
    void loadConfig(const std::string& filename);

    /**
     * @brief Gets all available toolchains.
     * @return A constant reference to a vector of toolchains.
     */
    [[nodiscard]] auto getToolchains() const -> const std::vector<std::shared_ptr<Toolchain>>&;

    /**
     * @brief Gets all available compilers.
     * @return A vector of available compilers.
     */
    [[nodiscard]] auto getAvailableCompilers() const
        -> std::vector<std::string>;

    /**
     * @brief Gets all available build tools.
     * @return A vector of available build tools.
     */
    [[nodiscard]] auto getAvailableBuildTools() const
        -> std::vector<std::string>;

    /**
     * @brief Adds a new toolchain.
     * @param toolchain The toolchain to add.
     */
    void addToolchain(const Toolchain& toolchain);

    /**
     * @brief Removes a toolchain by name.
     * @param name The name of the toolchain to remove.
     */
    void removeToolchain(const std::string& name);

    /**
     * @brief Updates an existing toolchain.
     * @param name The name of the toolchain to update.
     * @param updatedToolchain The updated toolchain object.
     */
    void updateToolchain(const std::string& name,
                         const Toolchain& updatedToolchain);

    /**
     * @brief Finds a toolchain by name.
     * @param name The name of the toolchain to find.
     * @return An optional Toolchain object if found.
     */
    [[nodiscard]] auto findToolchain(const std::string& name) const
        -> std::optional<std::shared_ptr<Toolchain>>;

    /**
     * @brief Finds toolchains that match a given filter.
     * @param filter The filter function to apply.
     * @return A vector of toolchains that match the filter.
     */
    [[nodiscard]] auto findToolchains(const ToolchainFilter& filter) const
        -> std::vector<std::shared_ptr<Toolchain>>;

    /**
     * @brief Suggests toolchains that are compatible with a given toolchain.
     * @param base The base toolchain to check compatibility with.
     * @return A vector of compatible toolchains.
     */
    [[nodiscard]] auto suggestCompatibleToolchains(const Toolchain& base) const
        -> std::vector<std::shared_ptr<Toolchain>>;

    /**
     * @brief Registers a custom toolchain.
     * @param name The name of the custom toolchain.
     * @param path The installation path of the custom toolchain.
     */
    void registerCustomToolchain(const std::string& name,
                                 const std::string& path);

    /**
     * @brief Sets the default toolchain.
     * @param name The name of the toolchain to set as default.
     */
    void setDefaultToolchain(const std::string& name);

    /**
     * @brief Gets the default toolchain.
     * @return An optional Toolchain object if a default is set.
     */
    [[nodiscard]] auto getDefaultToolchain() const -> std::optional<std::shared_ptr<Toolchain>>;

    /**
     * @brief Adds a search path for toolchains.
     * @param path The search path to add.
     */
    void addSearchPath(const std::string& path);

    /**
     * @brief Removes a search path for toolchains.
     * @param path The search path to remove.
     */
    void removeSearchPath(const std::string& path);

    /**
     * @brief Gets all search paths for toolchains.
     * @return A constant reference to a vector of search paths.
     */
    [[nodiscard]] auto getSearchPaths() const
        -> const std::vector<std::string>&;

    /**
     * @brief Sets an alias for a toolchain.
     * @param alias The alias to set.
     * @param toolchainName The name of the toolchain to alias.
     */
    void setToolchainAlias(const std::string& alias,
                           const std::string& toolchainName);

    /**
     * @brief Gets a toolchain by its alias.
     * @param alias The alias of the toolchain to get.
     * @return An optional Toolchain object if found.
     */
    [[nodiscard]] auto getToolchainByAlias(const std::string& alias) const
        -> std::optional<std::shared_ptr<Toolchain>>;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

#endif  // LITHIUM_ADDON_TOOLCHAIN_HPP