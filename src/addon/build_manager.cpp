#include "build_manager.hpp"

#include "platform/cmake.hpp"
#include "platform/meson.hpp"
#include "platform/xmake.hpp"

#include <future>
#include <thread>
#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"

namespace lithium {

// Project class implementation

Project::Project(std::filesystem::path sourceDir,
                 std::filesystem::path buildDirectory, BuildSystemType type)
    : sourceDir_(std::move(sourceDir)),
      buildDir_(std::move(buildDirectory)),
      buildSystemType_(type) {
    if (buildSystemType_ == BuildSystemType::Unknown) {
        detectBuildSystem();
    }
}

void Project::detectBuildSystem() {
    try {
        if (std::filesystem::exists(sourceDir_ / "CMakeLists.txt")) {
            buildSystemType_ = BuildSystemType::CMake;
        } else if (std::filesystem::exists(sourceDir_ / "meson.build")) {
            buildSystemType_ = BuildSystemType::Meson;
        } else if (std::filesystem::exists(sourceDir_ / "xmake.lua")) {
            buildSystemType_ = BuildSystemType::XMake;
        } else {
            buildSystemType_ = BuildSystemType::Unknown;
            THROW_INVALID_ARGUMENT(
                "Unable to detect a supported build system type");
        }
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception during build system detection: {}", e.what());
        buildSystemType_ = BuildSystemType::Unknown;
        THROW_INVALID_ARGUMENT("Build system detection failed.");
    }
}

const std::filesystem::path& Project::getSourceDir() const {
    return sourceDir_;
}

const std::filesystem::path& Project::getBuildDir() const { return buildDir_; }

Project::BuildSystemType Project::getBuildSystemType() const {
    return buildSystemType_;
}

// BuildManager class implementation

BuildManager::BuildManager() = default;

void BuildManager::scanForProjects(const std::filesystem::path& rootDir) {
    LOG_F(INFO, "Scanning for projects in directory {}...", rootDir.string());

    try {
        std::vector<std::future<void>> futures;
        for (const auto& entry :
             std::filesystem::recursive_directory_iterator(rootDir)) {
            if (entry.is_directory()) {
                futures.emplace_back(std::async(std::launch::async, [this,
                                                                     &entry]() {
                    const auto& path = entry.path();
                    if (std::filesystem::exists(path / "CMakeLists.txt") ||
                        std::filesystem::exists(path / "meson.build") ||
                        std::filesystem::exists(path / "xmake.lua")) {
                        try {
                            Project project(path, path / "build");
                            std::lock_guard<std::mutex> lock(projectsMutex_);
                            projects_.push_back(project);
                            LOG_F(INFO, "Found project: {}", path.string());
                        } catch (const std::exception& e) {
                            LOG_F(WARNING, "Unable to add project {}: {}",
                                  path.string(), e.what());
                        }
                    }
                }));
            }
        }

        for (auto& fut : futures) {
            fut.get();
        }
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception during project scanning: {}", e.what());
        THROW_INVALID_ARGUMENT("Project scanning failed.");
    }
}

void BuildManager::addProject(const Project& project) {
    try {
        std::lock_guard<std::mutex> lock(projectsMutex_);
        projects_.push_back(project);
        LOG_F(INFO, "Added project: {}", project.getSourceDir().string());
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Failed to add project {}: {}",
              project.getSourceDir().string(), e.what());
        throw;
    }
}

const std::vector<Project>& BuildManager::getProjects() const {
    return projects_;
}

BuildResult BuildManager::configureProject(
    const Project& project, BuildType buildType,
    const std::vector<std::string>& options,
    const std::map<std::string, std::string>& envVars) {
    LOG_F(INFO, "Configuring project: {}", project.getSourceDir().string());

    std::unique_ptr<BuildSystem> builder;

    try {
        switch (project.getBuildSystemType()) {
            case Project::BuildSystemType::CMake:
                builder = std::make_unique<CMakeBuilder>();
                break;
            case Project::BuildSystemType::Meson:
                builder = std::make_unique<MesonBuilder>();
                break;
            case Project::BuildSystemType::XMake:
                builder = std::make_unique<XMakeBuilder>();
                break;
            default:
                LOG_F(ERROR, "Unsupported build system type for project: {}",
                      project.getSourceDir().string());
                return BuildResult(false, "Unsupported build system type", -1);
        }

        auto result = builder->configureProject(project.getSourceDir(),
                                                project.getBuildDir(),
                                                buildType, options, envVars);
        if (result.isSuccess()) {
            LOG_F(INFO, "Configuration successful for project: {}",
                  project.getSourceDir().string());
        } else {
            LOG_F(ERROR, "Configuration failed for project {}: {}",
                  project.getSourceDir().string(), result.getMessage());
        }
        return result;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Configuration exception for project {}: {}",
              project.getSourceDir().string(), e.what());
        return BuildResult(false, e.what(), -1);
    }
}

BuildResult BuildManager::buildProject(const Project& project,
                                       std::optional<int> jobs) {
    LOG_F(INFO, "Building project: {}", project.getSourceDir().string());

    std::unique_ptr<BuildSystem> builder;

    try {
        switch (project.getBuildSystemType()) {
            case Project::BuildSystemType::CMake:
                builder = std::make_unique<CMakeBuilder>();
                break;
            case Project::BuildSystemType::Meson:
                builder = std::make_unique<MesonBuilder>();
                break;
            case Project::BuildSystemType::XMake:
                builder = std::make_unique<XMakeBuilder>();
                break;
            default:
                LOG_F(ERROR, "Unsupported build system type for project: {}",
                      project.getSourceDir().string());
                return BuildResult(false, "Unsupported build system type", -1);
        }

        auto result = builder->buildProject(project.getBuildDir(), jobs);
        if (result.isSuccess()) {
            LOG_F(INFO, "Build successful for project: {}",
                  project.getSourceDir().string());
        } else {
            LOG_F(ERROR, "Build failed for project {}: {}",
                  project.getSourceDir().string(), result.getMessage());
        }
        return result;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Build exception for project {}: {}",
              project.getSourceDir().string(), e.what());
        return BuildResult(false, e.what(), -1);
    }
}

BuildResult BuildManager::cleanProject(const Project& project) {
    LOG_F(INFO, "Cleaning project: {}", project.getSourceDir().string());

    std::unique_ptr<BuildSystem> builder;

    try {
        switch (project.getBuildSystemType()) {
            case Project::BuildSystemType::CMake:
                builder = std::make_unique<CMakeBuilder>();
                break;
            case Project::BuildSystemType::Meson:
                builder = std::make_unique<MesonBuilder>();
                break;
            case Project::BuildSystemType::XMake:
                builder = std::make_unique<XMakeBuilder>();
                break;
            default:
                LOG_F(ERROR, "Unsupported build system type for project: {}",
                      project.getSourceDir().string());
                return BuildResult(false, "Unsupported build system type", -1);
        }

        auto result = builder->cleanProject(project.getBuildDir());
        if (result.isSuccess()) {
            LOG_F(INFO, "Clean successful for project: {}",
                  project.getSourceDir().string());
        } else {
            LOG_F(ERROR, "Clean failed for project {}: {}",
                  project.getSourceDir().string(), result.getMessage());
        }
        return result;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Clean exception for project {}: {}",
              project.getSourceDir().string(), e.what());
        return BuildResult(false, e.what(), -1);
    }
}

BuildResult BuildManager::installProject(
    const Project& project, const std::filesystem::path& installDir) {
    LOG_F(INFO, "Installing project: {}", project.getSourceDir().string());

    std::unique_ptr<BuildSystem> builder;

    try {
        switch (project.getBuildSystemType()) {
            case Project::BuildSystemType::CMake:
                builder = std::make_unique<CMakeBuilder>();
                break;
            case Project::BuildSystemType::Meson:
                builder = std::make_unique<MesonBuilder>();
                break;
            case Project::BuildSystemType::XMake:
                builder = std::make_unique<XMakeBuilder>();
                break;
            default:
                LOG_F(ERROR, "Unsupported build system type for project: {}",
                      project.getSourceDir().string());
                return BuildResult(false, "Unsupported build system type", -1);
        }

        auto result =
            builder->installProject(project.getBuildDir(), installDir);
        if (result.isSuccess()) {
            LOG_F(INFO, "Install successful for project: {}",
                  project.getSourceDir().string());
        } else {
            LOG_F(ERROR, "Install failed for project {}: {}",
                  project.getSourceDir().string(), result.getMessage());
        }
        return result;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Install exception for project {}: {}",
              project.getSourceDir().string(), e.what());
        return BuildResult(false, e.what(), -1);
    }
}

BuildResult BuildManager::runTests(const Project& project,
                                   const std::vector<std::string>& testNames) {
    LOG_F(INFO, "Running tests for project: {}",
          project.getSourceDir().string());

    std::unique_ptr<BuildSystem> builder;

    try {
        switch (project.getBuildSystemType()) {
            case Project::BuildSystemType::CMake:
                builder = std::make_unique<CMakeBuilder>();
                break;
            case Project::BuildSystemType::Meson:
                builder = std::make_unique<MesonBuilder>();
                break;
            case Project::BuildSystemType::XMake:
                builder = std::make_unique<XMakeBuilder>();
                break;
            default:
                LOG_F(ERROR, "Unsupported build system type for project: {}",
                      project.getSourceDir().string());
                return BuildResult(false, "Unsupported build system type", -1);
        }

        auto result = builder->runTests(project.getBuildDir(), testNames);
        if (result.isSuccess()) {
            LOG_F(INFO, "Tests passed for project: {}",
                  project.getSourceDir().string());
        } else {
            LOG_F(ERROR, "Tests failed for project {}: {}",
                  project.getSourceDir().string(), result.getMessage());
        }
        return result;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Test exception for project {}: {}",
              project.getSourceDir().string(), e.what());
        return BuildResult(false, e.what(), -1);
    }
}

BuildResult BuildManager::generateDocs(const Project& project,
                                       const std::filesystem::path& outputDir) {
    LOG_F(INFO, "Generating docs for project: {}",
          project.getSourceDir().string());

    std::unique_ptr<BuildSystem> builder;

    try {
        switch (project.getBuildSystemType()) {
            case Project::BuildSystemType::CMake:
                builder = std::make_unique<CMakeBuilder>();
                break;
            case Project::BuildSystemType::Meson:
                builder = std::make_unique<MesonBuilder>();
                break;
            case Project::BuildSystemType::XMake:
                builder = std::make_unique<XMakeBuilder>();
                break;
            default:
                LOG_F(ERROR, "Unsupported build system type for project: {}",
                      project.getSourceDir().string());
                return BuildResult(false, "Unsupported build system type", -1);
        }

        auto result = builder->generateDocs(project.getBuildDir(), outputDir);
        if (result.isSuccess()) {
            LOG_F(INFO, "Docs generation successful for project: {}",
                  project.getSourceDir().string());
        } else {
            LOG_F(ERROR, "Docs generation failed for project {}: {}",
                  project.getSourceDir().string(), result.getMessage());
        }
        return result;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Docs generation exception for project {}: {}",
              project.getSourceDir().string(), e.what());
        return BuildResult(false, e.what(), -1);
    }
}

}  // namespace lithium