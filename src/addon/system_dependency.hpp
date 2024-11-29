#ifndef LITHIUM_ADDON_DEPENDENCY_MANAGER_HPP
#define LITHIUM_ADDON_DEPENDENCY_MANAGER_HPP

#include <exception>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace lithium {

/**
 * @class DependencyException
 * @brief Exception class for dependency-related errors.
 */
class DependencyException : public std::exception {
public:
    /**
     * @brief Constructs a DependencyException with a given message.
     * @param message The error message.
     */
    explicit DependencyException(std::string message)
        : message_(std::move(message)) {}

    /**
     * @brief Returns the error message.
     * @return The error message as a C-string.
     */
    [[nodiscard]] auto what() const noexcept -> const char* override {
        return message_.c_str();
    }

private:
    std::string message_;  ///< The error message.
};

/**
 * @struct DependencyInfo
 * @brief Structure to hold information about a dependency.
 */
struct DependencyInfo {
    std::string name;            ///< The name of the dependency.
    std::string version;         ///< The version of the dependency (optional).
    std::string packageManager;  ///< The specified package manager.
};

/**
 * @struct PackageManagerInfo
 * @brief Structure to hold information about a package manager.
 */
struct PackageManagerInfo {
    std::string name;  ///< The name of the package manager.
    std::function<std::string(const DependencyInfo&)>
        getCheckCommand;  ///< Function to get the check command.
    std::function<std::string(const DependencyInfo&)>
        getInstallCommand;  ///< Function to get the install command.
    std::function<std::string(const DependencyInfo&)>
        getUninstallCommand;  ///< Function to get the uninstall command.
    std::function<std::string(const std::string&)>
        getSearchCommand;  ///< Function to get the search command.
};

/**
 * @class DependencyManager
 * @brief Class to manage dependencies.
 */
class DependencyManager {
public:
    /**
     * @brief Constructs a DependencyManager.
     */
    DependencyManager();

    /**
     * @brief Destructs the DependencyManager.
     */
    ~DependencyManager();

    // Delete copy constructor and copy assignment operator
    DependencyManager(const DependencyManager&) = delete;
    DependencyManager& operator=(const DependencyManager&) = delete;

    /**
     * @brief Checks and installs dependencies.
     */
    void checkAndInstallDependencies();

    /**
     * @brief Sets a custom install command for a dependency.
     * @param dep The name of the dependency.
     * @param command The custom install command.
     */
    void setCustomInstallCommand(const std::string& dep,
                                 const std::string& command);

    /**
     * @brief Generates a report of the dependencies.
     * @return A string containing the dependency report.
     */
    auto generateDependencyReport() const -> std::string;

    /**
     * @brief Uninstalls a dependency.
     * @param dep The name of the dependency to uninstall.
     */
    void uninstallDependency(const std::string& dep);

    /**
     * @brief Gets the current platform.
     * @return A string representing the current platform.
     */
    auto getCurrentPlatform() const -> std::string;

    /**
     * @brief Installs a dependency asynchronously.
     * @param dep The dependency information.
     */
    void installDependencyAsync(const DependencyInfo& dep);

    /**
     * @brief Cancels the installation of a dependency.
     * @param dep The name of the dependency to cancel installation for.
     */
    void cancelInstallation(const std::string& dep);

    /**
     * @brief Adds a dependency.
     * @param dep The dependency information.
     */
    void addDependency(const DependencyInfo& dep);

    /**
     * @brief Removes a dependency.
     * @param depName The name of the dependency to remove.
     */
    void removeDependency(const std::string& depName);

    /**
     * @brief Searches for a dependency.
     * @param depName The name of the dependency to search for.
     * @return A vector of strings containing search results.
     */
    auto searchDependency(const std::string& depName)
        -> std::vector<std::string>;

    /**
     * @brief Loads system package managers.
     */
    void loadSystemPackageManagers();

    /**
     * @brief Gets the package managers.
     * @return A vector of PackageManagerInfo structures.
     */
    auto getPackageManagers() const -> std::vector<PackageManagerInfo>;

private:
    class Impl;  ///< Forward declaration of the implementation class.
    std::unique_ptr<Impl> pImpl_;  ///< Pointer to the implementation.
};

}  // namespace lithium

#endif  // LITHIUM_ADDON_DEPENDENCY_MANAGER_HPP