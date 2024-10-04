#ifndef LITHIUM_ADDON_BUILDER_HPP
#define LITHIUM_ADDON_BUILDER_HPP

#include <filesystem>
#include <functional>
#include <memory>
#include <optional>

#include "platform/base.hpp"

namespace lithium {

/**
 * @class BuildManager
 * @brief Manages the build process for various build systems.
 *
 * This class provides an interface to configure, build, clean, install, and
 * test projects using different build systems like CMake, Meson, and XMake.
 */
class BuildManager {
public:
    /**
     * @enum BuildSystemType
     * @brief Represents the type of build system.
     */
    enum class BuildSystemType {
        CMake, /**< CMake build system. */
        Meson, /**< Meson build system. */
        XMake  /**< XMake build system. */
    };

    /**
     * @brief Constructs a BuildManager object.
     *
     * @param type The type of build system to use.
     */
    BuildManager(BuildSystemType type);

    /**
     * @brief Configures the project.
     *
     * @param sourceDir The source directory of the project.
     * @param buildDir The build directory where the project will be configured.
     * @param buildType The type of build (e.g., Debug, Release).
     * @param options Additional options for the build system.
     * @return A BuildResult indicating the success or failure of the
     * configuration.
     */
    auto configureProject(
        const std::filesystem::path& sourceDir,
        const std::filesystem::path& buildDir, BuildType buildType,
        const std::vector<std::string>& options) -> BuildResult;

    /**
     * @brief Builds the project.
     *
     * @param buildDir The build directory where the project is configured.
     * @param jobs The number of parallel jobs to use for building (optional).
     * @return A BuildResult indicating the success or failure of the build.
     */
    auto buildProject(const std::filesystem::path& buildDir,
                      std::optional<int> jobs = std::nullopt) -> BuildResult;

    /**
     * @brief Cleans the project.
     *
     * @param buildDir The build directory where the project is configured.
     * @return A BuildResult indicating the success or failure of the clean
     * operation.
     */
    auto cleanProject(const std::filesystem::path& buildDir) -> BuildResult;

    /**
     * @brief Installs the project.
     *
     * @param buildDir The build directory where the project is configured.
     * @param installDir The directory where the project will be installed.
     * @return A BuildResult indicating the success or failure of the install
     * operation.
     */
    auto installProject(const std::filesystem::path& buildDir,
                        const std::filesystem::path& installDir) -> BuildResult;

    /**
     * @brief Runs tests for the project.
     *
     * @param buildDir The build directory where the project is configured.
     * @param testNames The names of the tests to run (optional).
     * @return A BuildResult indicating the success or failure of the test run.
     */
    auto runTests(const std::filesystem::path& buildDir,
                  const std::vector<std::string>& testNames = {})
        -> BuildResult;

    /**
     * @brief Generates documentation for the project.
     *
     * @param buildDir The build directory where the project is configured.
     * @param outputDir The directory where the documentation will be generated.
     * @return A BuildResult indicating the success or failure of the
     * documentation generation.
     */
    auto generateDocs(const std::filesystem::path& buildDir,
                      const std::filesystem::path& outputDir) -> BuildResult;

    /**
     * @brief Loads a build configuration from a file.
     *
     * @param configPath The path to the configuration file.
     * @return True if the configuration was loaded successfully, false
     * otherwise.
     */
    auto loadConfig(const std::filesystem::path& configPath) -> bool;

    /**
     * @brief Sets a callback function for logging build messages.
     *
     * @param callback The callback function to use for logging.
     */
    auto setLogCallback(std::function<void(const std::string&)> callback)
        -> void;

    /**
     * @brief Gets the available build targets.
     *
     * @param buildDir The build directory where the project is configured.
     * @return A vector of strings representing the available build targets.
     */
    auto getAvailableTargets(const std::filesystem::path& buildDir)
        -> std::vector<std::string>;

    /**
     * @brief Builds a specific target.
     *
     * @param buildDir The build directory where the project is configured.
     * @param target The name of the target to build.
     * @param jobs The number of parallel jobs to use for building (optional).
     * @return A BuildResult indicating the success or failure of the build.
     */
    auto buildTarget(const std::filesystem::path& buildDir,
                     const std::string& target,
                     std::optional<int> jobs = std::nullopt) -> BuildResult;

    /**
     * @brief Gets the cache variables for the build.
     *
     * @param buildDir The build directory where the project is configured.
     * @return A vector of pairs representing the cache variables and their
     * values.
     */
    auto getCacheVariables(const std::filesystem::path& buildDir)
        -> std::vector<std::pair<std::string, std::string>>;

    /**
     * @brief Sets a cache variable for the build.
     *
     * @param buildDir The build directory where the project is configured.
     * @param name The name of the cache variable.
     * @param value The value of the cache variable.
     * @return True if the cache variable was set successfully, false otherwise.
     */
    auto setCacheVariable(const std::filesystem::path& buildDir,
                          const std::string& name,
                          const std::string& value) -> bool;

private:
    std::unique_ptr<BuildSystem>
        builder_; /**< Pointer to the build system implementation. */
};

}  // namespace lithium

#endif  // LITHIUM_ADDON_BUILDER_HPP