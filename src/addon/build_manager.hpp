#ifndef LITHIUM_ADDON_BUILDER_HPP
#define LITHIUM_ADDON_BUILDER_HPP

#include <filesystem>
#include <functional>
#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "platform/base.hpp"

namespace lithium {

/**
 * @class Project
 * @brief Represents a project with source directory, build directory, and build
 * system type.
 */
class Project {
public:
    /**
     * @enum BuildSystemType
     * @brief Represents the type of build system.
     */
    enum class BuildSystemType {
        CMake,  /**< CMake build system. */
        Meson,  /**< Meson build system. */
        XMake,  /**< XMake build system. */
        Unknown /**< Unknown build system. */
    };

    /**
     * @brief Constructor to create a Project object.
     *
     * @param sourceDir Path to the project's source directory.
     * @param buildDir Path to the project's build directory.
     * @param type Build system type, optional. Defaults to Unknown and
     * auto-detects.
     */
    Project(std::filesystem::path sourceDir,
            std::filesystem::path buildDirectory,
            BuildSystemType type = BuildSystemType::Unknown);

    /**
     * @brief Automatically detects the build system type.
     */
    void detectBuildSystem();

    /**
     * @brief Gets the source directory.
     *
     * @return Path to the source directory.
     */
    const std::filesystem::path& getSourceDir() const;

    /**
     * @brief Gets the build directory.
     *
     * @return Path to the build directory.
     */
    const std::filesystem::path& getBuildDir() const;

    /**
     * @brief Gets the build system type.
     *
     * @return Build system type.
     */
    BuildSystemType getBuildSystemType() const;

private:
    std::filesystem::path sourceDir_;
    std::filesystem::path buildDir_;
    BuildSystemType buildSystemType_;
};

/**
 * @class BuildManager
 * @brief Manages the build processes of multiple projects, supporting various
 * build systems.
 */
class BuildManager {
public:
    /**
     * @typedef BuildTask
     * @brief Defines a build task type using std::function.
     *
     * Each build task is a callable object that returns a BuildResult.
     */
    using BuildTask = std::function<BuildResult()>;

    /**
     * @brief Constructor to create a BuildManager object.
     */
    BuildManager();

    /**
     * @brief Scans the specified directory to automatically detect and manage
     * projects.
     *
     * @param rootDir Root directory to scan.
     */
    void scanForProjects(const std::filesystem::path& rootDir);

    /**
     * @brief Adds a project to the manager.
     *
     * @param project The project to add.
     */
    void addProject(const Project& project);

    /**
     * @brief Retrieves all managed projects.
     *
     * @return List of projects.
     */
    const std::vector<Project>& getProjects() const;

    // Build operations for individual projects
    BuildResult configureProject(
        const Project& project, BuildType buildType,
        const std::vector<std::string>& options = {},
        const std::map<std::string, std::string>& envVars = {});

    BuildResult buildProject(const Project& project,
                             std::optional<int> jobs = std::nullopt);

    BuildResult cleanProject(const Project& project);

    BuildResult installProject(const Project& project,
                               const std::filesystem::path& installDir);

    BuildResult runTests(const Project& project,
                         const std::vector<std::string>& testNames = {});

    BuildResult generateDocs(const Project& project,
                             const std::filesystem::path& outputDir);

private:
    std::vector<Project> projects_; /**< List of managed projects. */
    mutable std::mutex
        projectsMutex_; /**< Mutex for thread-safe access to projects. */
};

}  // namespace lithium

#endif  // LITHIUM_ADDON_BUILDER_HPP