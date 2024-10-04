#ifndef LITHIUM_ADDON_BUILDBASE_HPP
#define LITHIUM_ADDON_BUILDBASE_HPP

#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "atom/macro.hpp"

namespace lithium {

/**
 * @brief Enum representing different build types.
 */
enum class BuildType {
    DEBUG,              ///< Debug build
    RELEASE,            ///< Release build
    REL_WITH_DEB_INFO,  ///< Release build with debug information
    MIN_SIZE_REL        ///< Minimum size release build
};

/**
 * @brief Structure representing the result of a build operation.
 */
struct BuildResult {
    bool success;        ///< Indicates if the build was successful
    std::string output;  ///< Standard output from the build process
    std::string error;   ///< Standard error from the build process
} ATOM_ALIGNAS(128);

/**
 * @brief Abstract base class for build systems.
 *
 * This class defines the interface for various build systems.
 */
class BuildSystem {
public:
    /**
     * @brief Virtual destructor for BuildSystem.
     */
    virtual ~BuildSystem() = default;

    /**
     * @brief Configures the project for building.
     *
     * @param sourceDir The source directory of the project.
     * @param buildDir The build directory.
     * @param buildType The type of build (e.g., Debug, Release).
     * @param options Additional options for configuration.
     * @return A BuildResult indicating the success or failure of the
     * configuration.
     */
    virtual auto configureProject(
        const std::filesystem::path& sourceDir,
        const std::filesystem::path& buildDir, BuildType buildType,
        const std::vector<std::string>& options) -> BuildResult = 0;

    /**
     * @brief Builds the project.
     *
     * @param buildDir The build directory.
     * @param jobs Optional number of parallel jobs to use for building.
     * @return A BuildResult indicating the success or failure of the build.
     */
    virtual auto buildProject(const std::filesystem::path& buildDir,
                              std::optional<int> jobs = std::nullopt)
        -> BuildResult = 0;

    /**
     * @brief Cleans the project build directory.
     *
     * @param buildDir The build directory.
     * @return A BuildResult indicating the success or failure of the clean
     * operation.
     */
    virtual auto cleanProject(const std::filesystem::path& buildDir)
        -> BuildResult = 0;

    /**
     * @brief Installs the built project to the specified directory.
     *
     * @param buildDir The build directory.
     * @param installDir The installation directory.
     * @return A BuildResult indicating the success or failure of the
     * installation.
     */
    virtual auto installProject(const std::filesystem::path& buildDir,
                                const std::filesystem::path& installDir)
        -> BuildResult = 0;

    /**
     * @brief Runs tests for the project.
     *
     * @param buildDir The build directory.
     * @param testNames Optional list of specific test names to run.
     * @return A BuildResult indicating the success or failure of the test run.
     */
    virtual auto runTests(const std::filesystem::path& buildDir,
                          const std::vector<std::string>& testNames = {})
        -> BuildResult = 0;

    /**
     * @brief Generates documentation for the project.
     *
     * @param buildDir The build directory.
     * @param outputDir The directory where the documentation will be generated.
     * @return A BuildResult indicating the success or failure of the
     * documentation generation.
     */
    virtual auto generateDocs(const std::filesystem::path& buildDir,
                              const std::filesystem::path& outputDir)
        -> BuildResult = 0;

    /**
     * @brief Loads a configuration file for the build system.
     *
     * @param configPath The path to the configuration file.
     * @return True if the configuration was successfully loaded, false
     * otherwise.
     */
    virtual auto loadConfig(const std::filesystem::path& configPath)
        -> bool = 0;

    /**
     * @brief Sets a callback function for logging.
     *
     * @param callback The callback function to be used for logging.
     */
    virtual auto setLogCallback(
        std::function<void(const std::string&)> callback) -> void = 0;

    /**
     * @brief Gets the available build targets.
     *
     * @param buildDir The build directory.
     * @return A vector of available build targets.
     */
    virtual auto getAvailableTargets(const std::filesystem::path& buildDir)
        -> std::vector<std::string> = 0;

    /**
     * @brief Builds a specific target.
     *
     * @param buildDir The build directory.
     * @param target The name of the target to build.
     * @param jobs Optional number of parallel jobs to use for building.
     * @return A BuildResult indicating the success or failure of the build.
     */
    virtual auto buildTarget(
        const std::filesystem::path& buildDir, const std::string& target,
        std::optional<int> jobs = std::nullopt) -> BuildResult = 0;

    /**
     * @brief Gets the cache variables for the build system.
     *
     * @param buildDir The build directory.
     * @return A vector of pairs representing the cache variable names and their
     * values.
     */
    virtual auto getCacheVariables(const std::filesystem::path& buildDir)
        -> std::vector<std::pair<std::string, std::string>> = 0;

    /**
     * @brief Sets a cache variable for the build system.
     *
     * @param buildDir The build directory.
     * @param name The name of the cache variable.
     * @param value The value of the cache variable.
     * @return True if the cache variable was successfully set, false otherwise.
     */
    virtual auto setCacheVariable(const std::filesystem::path& buildDir,
                                  const std::string& name,
                                  const std::string& value) -> bool = 0;
};

}  // namespace lithium

#endif  // LITHIUM_ADDON_BUILDBASE_HPP