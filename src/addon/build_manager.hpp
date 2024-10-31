#ifndef LITHIUM_ADDON_BUILDER_HPP
#define LITHIUM_ADDON_BUILDER_HPP

#include <filesystem>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "platform/base.hpp"

namespace lithium {

/**
 * @class Project
 * @brief 表示一个项目，包含源代码目录、构建目录和构建系统类型。
 */
class Project {
public:
    /**
     * @enum BuildSystemType
     * @brief 表示构建系统的类型。
     */
    enum class BuildSystemType {
        CMake,  /**< CMake 构建系统。 */
        Meson,  /**< Meson 构建系统。 */
        XMake,  /**< XMake 构建系统。 */
        Unknown /**< 未知的构建系统。 */
    };

    /**
     * @brief 构造函数，创建一个项目对象。
     *
     * @param sourceDir 项目的源代码目录。
     * @param buildDir 项目的构建目录。
     * @param type 构建系统类型，可选，默认为 Unknown，将自动检测。
     */
    Project(std::filesystem::path sourceDir,
            std::filesystem::path buildDirectory,
            BuildSystemType type = BuildSystemType::Unknown);

    /**
     * @brief 自动检测构建系统类型。
     */
    void detectBuildSystem();

    /**
     * @brief 获取源代码目录。
     *
     * @return 源代码目录的路径。
     */
    const std::filesystem::path& getSourceDir() const;

    /**
     * @brief 获取构建目录。
     *
     * @return 构建目录的路径。
     */
    const std::filesystem::path& getBuildDir() const;

    /**
     * @brief 获取构建系统类型。
     *
     * @return 构建系统类型。
     */
    BuildSystemType getBuildSystemType() const;

private:
    std::filesystem::path sourceDir_;
    std::filesystem::path buildDir_;
    BuildSystemType buildSystemType_;
};

/**
 * @class BuildManager
 * @brief 管理多个项目的构建过程，支持各种构建系统。
 */
class BuildManager {
public:
    /**
     * @typedef BuildTask
     * @brief 定义了一个构建任务类型，使用 std::function。
     *
     * 每个构建任务是返回 BuildResult 的可调用对象。
     */
    using BuildTask = std::function<BuildResult()>;

    /**
     * @brief 构造函数，创建一个 BuildManager 对象。
     */
    BuildManager();

    /**
     * @brief 扫描指定目录，自动检测并管理项目。
     *
     * @param rootDir 要扫描的根目录。
     */
    void scanForProjects(const std::filesystem::path& rootDir);

    /**
     * @brief 添加一个项目到管理器。
     *
     * @param project 要添加的项目。
     */
    void addProject(const Project& project);

    /**
     * @brief 获取所有管理的项目。
     *
     * @return 项目列表。
     */
    const std::vector<Project>& getProjects() const;

    // 针对单个项目的构建操作
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

    // 异常处理和性能优化已在实现中考虑

private:
    std::vector<Project> projects_; /**< 管理的项目列表。 */
};

}  // namespace lithium

#endif  // LITHIUM_ADDON_BUILDER_HPP