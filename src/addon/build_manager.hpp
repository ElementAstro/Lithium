#ifndef LITHIUM_ADDON_BUILDER_HPP
#define LITHIUM_ADDON_BUILDER_HPP

#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <map>

#include "platform/base.hpp"

namespace lithium {

/**
 * @class BuildManager
 * @brief Manages the build process for various build systems with enhanced features.
 *
 * This class provides an interface to configure, build, clean, install, test, and
 * generate documentation for projects using different build systems like CMake, Meson, and XMake.
 * It also supports defining and executing build task chains, managing environment variables,
 * and enhanced logging capabilities.
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
     * @typedef BuildTask
     * @brief Defines a type for build tasks using std::function.
     *
     * Each build task is a callable that returns a BuildResult.
     */
    using BuildTask = std::function<BuildResult()>;

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
     * @param envVars Environment variables to set during configuration.
     * @return A BuildResult indicating the success or failure of the configuration.
     */
    BuildResult configureProject(
        const std::filesystem::path& sourceDir,
        const std::filesystem::path& buildDir,
        BuildType buildType,
        const std::vector<std::string>& options = {},
        const std::map<std::string, std::string>& envVars = {});

    /**
     * @brief Builds the project.
     *
     * @param buildDir The build directory where the project is configured.
     * @param jobs The number of parallel jobs to use for building (optional).
     * @return A BuildResult indicating the success or failure of the build.
     */
    BuildResult buildProject(const std::filesystem::path& buildDir,
                             std::optional<int> jobs = std::nullopt);

    /**
     * @brief Cleans the project.
     *
     * @param buildDir The build directory where the project is configured.
     * @return A BuildResult indicating the success or failure of the clean operation.
     */
    BuildResult cleanProject(const std::filesystem::path& buildDir);

    /**
     * @brief Installs the project.
     *
     * @param buildDir The build directory where the project is configured.
     * @param installDir The directory where the project will be installed.
     * @return A BuildResult indicating the success or failure of the install operation.
     */
    BuildResult installProject(const std::filesystem::path& buildDir,
                               const std::filesystem::path& installDir);

    /**
     * @brief Runs tests for the project.
     *
     * @param buildDir The build directory where the project is configured.
     * @param testNames The names of the tests to run (optional).
     * @return A BuildResult indicating the success or failure of the test run.
     */
    BuildResult runTests(const std::filesystem::path& buildDir,
                        const std::vector<std::string>& testNames = {});

    /**
     * @brief Generates documentation for the project.
     *
     * @param buildDir The build directory where the project is configured.
     * @param outputDir The directory where the documentation will be generated.
     * @return A BuildResult indicating the success or failure of the documentation generation.
     */
    BuildResult generateDocs(const std::filesystem::path& buildDir,
                             const std::filesystem::path& outputDir);

    /**
     * @brief Loads a build configuration from a file.
     *
     * @param configPath The path to the configuration file.
     * @return True if the configuration was loaded successfully, false otherwise.
     */
    bool loadConfig(const std::filesystem::path& configPath);

    /**
     * @brief Gets the available build targets.
     *
     * @param buildDir The build directory where the project is configured.
     * @return A vector of strings representing the available build targets.
     */
    std::vector<std::string> getAvailableTargets(const std::filesystem::path& buildDir);

    /**
     * @brief Builds a specific target.
     *
     * @param buildDir The build directory where the project is configured.
     * @param target The name of the target to build.
     * @param jobs The number of parallel jobs to use for building (optional).
     * @return A BuildResult indicating the success or failure of the build.
     */
    BuildResult buildTarget(const std::filesystem::path& buildDir,
                            const std::string& target,
                            std::optional<int> jobs = std::nullopt);

    /**
     * @brief Gets the cache variables for the build.
     *
     * @param buildDir The build directory where the project is configured.
     * @return A vector of pairs representing the cache variables and their values.
     */
    std::vector<std::pair<std::string, std::string>> getCacheVariables(
        const std::filesystem::path& buildDir);

    /**
     * @brief Sets a cache variable for the build.
     *
     * @param buildDir The build directory where the project is configured.
     * @param name The name of the cache variable.
     * @param value The value of the cache variable.
     * @return True if the cache variable was set successfully, false otherwise.
     */
    bool setCacheVariable(const std::filesystem::path& buildDir,
                          const std::string& name,
                          const std::string& value);

    /**
     * @brief Adds a build task to the task chain.
     *
     * @param task The build task to add.
     */
    void addBuildTask(const BuildTask& task);

    /**
     * @brief Executes the defined build task chain sequentially.
     *
     * @return A BuildResult indicating the success or failure of the task chain execution.
     */
    BuildResult executeTaskChain();

    /**
     * @brief Clears all build tasks from the task chain.
     */
    void clearTaskChain();

private:
    std::unique_ptr<BuildSystem> builder_; /**< Pointer to the build system implementation. */
    std::vector<BuildTask> taskChain_; /**< Sequence of build tasks to execute. */
};

}  // namespace lithium

#endif  // LITHIUM_ADDON_BUILDER_HPP
