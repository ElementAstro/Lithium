#include "build_manager.hpp"

#include "platform/cmake.hpp"
#include "platform/meson.hpp"
#include "platform/xmake.hpp"

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"

namespace lithium {

// Project class implementation

Project::Project(std::filesystem::path sourceDir,
                 std::filesystem::path buildDirectory, BuildSystemType type)
    : sourceDir_(std::move(sourceDir)), buildDir_(std::move(buildDirectory)), buildSystemType_(type) {
    if (buildSystemType_ == BuildSystemType::Unknown) {
        detectBuildSystem();
    }
}

void Project::detectBuildSystem() {
    if (std::filesystem::exists(sourceDir_ / "CMakeLists.txt")) {
        buildSystemType_ = BuildSystemType::CMake;
    } else if (std::filesystem::exists(sourceDir_ / "meson.build")) {
        buildSystemType_ = BuildSystemType::Meson;
    } else if (std::filesystem::exists(sourceDir_ / "xmake.lua")) {
        buildSystemType_ = BuildSystemType::XMake;
    } else {
        buildSystemType_ = BuildSystemType::Unknown;
        THROW_INVALID_ARGUMENT("Unable to detect a supported build system type");
    }
}

auto Project::getSourceDir() const -> const std::filesystem::path& {
    return sourceDir_;
}

auto Project::getBuildDir() const -> const std::filesystem::path& {
    return buildDir_;
}

auto Project::getBuildSystemType() const -> BuildSystemType {
    return buildSystemType_;
}

// BuildManager class implementation

BuildManager::BuildManager() = default;

void BuildManager::scanForProjects(const std::filesystem::path& rootDir) {
    LOG_F(INFO, "Scanning for projects in directory %s...", rootDir.string().c_str());
    for (const auto& entry : std::filesystem::recursive_directory_iterator(rootDir)) {
        if (entry.is_directory()) {
            const auto& path = entry.path();
            if (std::filesystem::exists(path / "CMakeLists.txt") ||
                std::filesystem::exists(path / "meson.build") ||
                std::filesystem::exists(path / "xmake.lua")) {
                try {
                    Project project(path, path / "build");
                    projects_.push_back(project);
                    LOG_F(INFO, "Found project: %s", path.string().c_str());
                } catch (const std::exception& e) {
                    LOG_F(WARNING, "Unable to add project %s: %s", path.string().c_str(), e.what());
                }
            }
        }
    }
}

void BuildManager::addProject(const Project& project) {
    projects_.push_back(project);
    LOG_F(INFO, "Added project: %s", project.getSourceDir().string().c_str());
}

auto BuildManager::getProjects() const -> const std::vector<Project>& {
    return projects_;
}

auto BuildManager::configureProject(const Project& project, BuildType buildType,
                                    const std::vector<std::string>& options,
                                    const std::map<std::string, std::string>& envVars) -> BuildResult {
    LOG_F(INFO, "Configuring project: %s", project.getSourceDir().string().c_str());

    std::unique_ptr<BuildSystem> builder;

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
            return BuildResult(false, "Unsupported build system type", -1);
    }

    try {
        auto result = builder->configureProject(project.getSourceDir(), project.getBuildDir(), buildType, options, envVars);
        if (result.isSuccess()) {
            LOG_F(INFO, "Configuration successful.");
        } else {
            LOG_F(ERROR, "Configuration failed: %s", result.getMessage().c_str());
        }
        return result;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Configuration exception: %s", e.what());
        return BuildResult(false, e.what(), -1);
    }
}

auto BuildManager::buildProject(const Project& project, std::optional<int> jobs) -> BuildResult {
    LOG_F(INFO, "Building project: %s", project.getSourceDir().string().c_str());

    std::unique_ptr<BuildSystem> builder;

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
            return BuildResult(false, "Unsupported build system type", -1);
    }

    try {
        auto result = builder->buildProject(project.getBuildDir(), jobs);
        if (result.isSuccess()) {
            LOG_F(INFO, "Build successful.");
        } else {
            LOG_F(ERROR, "Build failed: %s", result.getMessage().c_str());
        }
        return result;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Build exception: %s", e.what());
        return BuildResult(false, e.what(), -1);
    }
}

auto BuildManager::cleanProject(const Project& project) -> BuildResult {
    LOG_F(INFO, "Cleaning project: %s", project.getSourceDir().string().c_str());

    std::unique_ptr<BuildSystem> builder;

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
            return BuildResult(false, "Unsupported build system type", -1);
    }

    try {
        auto result = builder->cleanProject(project.getBuildDir());
        if (result.isSuccess()) {
            LOG_F(INFO, "Clean successful.");
        } else {
            LOG_F(ERROR, "Clean failed: %s", result.getMessage().c_str());
        }
        return result;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Clean exception: %s", e.what());
        return BuildResult(false, e.what(), -1);
    }
}

auto BuildManager::installProject(const Project& project, const std::filesystem::path& installDir) -> BuildResult {
    LOG_F(INFO, "Installing project: %s", project.getSourceDir().string().c_str());

    std::unique_ptr<BuildSystem> builder;

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
            return BuildResult(false, "Unsupported build system type", -1);
    }

    try {
        auto result = builder->installProject(project.getBuildDir(), installDir);
        if (result.isSuccess()) {
            LOG_F(INFO, "Install successful.");
        } else {
            LOG_F(ERROR, "Install failed: %s", result.getMessage().c_str());
        }
        return result;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Install exception: %s", e.what());
        return BuildResult(false, e.what(), -1);
    }
}

auto BuildManager::runTests(const Project& project, const std::vector<std::string>& testNames) -> BuildResult {
    LOG_F(INFO, "Running tests for project: %s", project.getSourceDir().string().c_str());

    std::unique_ptr<BuildSystem> builder;

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
            return BuildResult(false, "Unsupported build system type", -1);
    }

    try {
        auto result = builder->runTests(project.getBuildDir(), testNames);
        if (result.isSuccess()) {
            LOG_F(INFO, "Tests passed.");
        } else {
            LOG_F(ERROR, "Tests failed: %s", result.getMessage().c_str());
        }
        return result;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Test exception: %s", e.what());
        return BuildResult(false, e.what(), -1);
    }
}

auto BuildManager::generateDocs(const Project& project, const std::filesystem::path& outputDir) -> BuildResult {
    LOG_F(INFO, "Generating docs for project: %s", project.getSourceDir().string().c_str());

    std::unique_ptr<BuildSystem> builder;

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
            return BuildResult(false, "Unsupported build system type", -1);
    }

    try {
        auto result = builder->generateDocs(project.getBuildDir(), outputDir);
        if (result.isSuccess()) {
            LOG_F(INFO, "Docs generation successful.");
        } else {
            LOG_F(ERROR, "Docs generation failed: %s", result.getMessage().c_str());
        }
        return result;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Docs generation exception: %s", e.what());
        return BuildResult(false, e.what(), -1);
    }
}

}  // namespace lithium
