#include "build_manager.hpp"

#include "platform/cmake.hpp"
#include "platform/meson.hpp"
#include "platform/xmake.hpp"

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"

namespace lithium {
BuildManager::BuildManager(BuildSystemType type) {
    switch (type) {
        case BuildSystemType::CMake:
            builder_ = std::make_unique<CMakeBuilder>();
            break;
        case BuildSystemType::Meson:
            builder_ = std::make_unique<MesonBuilder>();
            break;
        case BuildSystemType::XMake:
            builder_ = std::make_unique<XMakeBuilder>();
            break;
        default:
            THROW_INVALID_ARGUMENT("Unsupported build system type");
    }
}

BuildResult BuildManager::configureProject(
    const std::filesystem::path& sourceDir,
    const std::filesystem::path& buildDir, BuildType buildType,
    const std::vector<std::string>& options,
    const std::map<std::string, std::string>& envVars) {
    LOG_F(INFO, "Configuring project...");
    auto result = builder_->configureProject(sourceDir, buildDir, buildType,
                                             options, envVars);
    if (result.isSuccess()) {
        LOG_F(INFO, "Configuration successful.");
    } else {
        LOG_F(ERROR, "Configuration failed: {}", result.getMessage());
    }
    return result;
}

BuildResult BuildManager::buildProject(const std::filesystem::path& buildDir,
                                       std::optional<int> jobs) {
    LOG_F(INFO, "Building project...");
    auto result = builder_->buildProject(buildDir, jobs);
    if (result.isSuccess()) {
        LOG_F(INFO, "Build successful.");
    } else {
        LOG_F(ERROR, "Build failed: {}", result.getMessage());
    }
    return result;
}

BuildResult BuildManager::cleanProject(const std::filesystem::path& buildDir) {
    LOG_F(INFO, "Cleaning project...");
    auto result = builder_->cleanProject(buildDir);
    if (result.isSuccess()) {
        LOG_F(INFO, "Clean successful.");
    } else {
        LOG_F(ERROR, "Clean failed: {}", result.getMessage());
    }
    return result;
}

BuildResult BuildManager::installProject(
    const std::filesystem::path& buildDir,
    const std::filesystem::path& installDir) {
    LOG_F(INFO, "Installing project...");
    auto result = builder_->installProject(buildDir, installDir);
    if (result.isSuccess()) {
        LOG_F(INFO, "Install successful.");
    } else {
        LOG_F(ERROR, "Install failed: {}", result.getMessage());
    }
    return result;
}

BuildResult BuildManager::runTests(const std::filesystem::path& buildDir,
                                   const std::vector<std::string>& testNames) {
    LOG_F(INFO, "Running tests...");
    auto result = builder_->runTests(buildDir, testNames);
    if (result.isSuccess()) {
        LOG_F(INFO, "Tests passed.");
    } else {
        LOG_F(ERROR, "Tests failed: {}", result.getMessage());
    }
    return result;
}

BuildResult BuildManager::generateDocs(const std::filesystem::path& buildDir,
                                       const std::filesystem::path& outputDir) {
    LOG_F(INFO, "Generating documentation...");
    auto result = builder_->generateDocs(buildDir, outputDir);
    if (result.isSuccess()) {
        LOG_F(INFO, "Documentation generation successful.");
    } else {
        LOG_F(ERROR, "Documentation generation failed: {}",
              result.getMessage());
    }
    return result;
}

bool BuildManager::loadConfig(const std::filesystem::path& configPath) {
    LOG_F(INFO, "Loading configuration from {}", configPath.string());
    bool success = builder_->loadConfig(configPath);
    if (success) {
        LOG_F(INFO, "Configuration loaded successfully.");
    } else {
        LOG_F(ERROR, "Failed to load configuration.");
    }
    return success;
}

std::vector<std::string> BuildManager::getAvailableTargets(
    const std::filesystem::path& buildDir) {
    LOG_F(INFO, "Retrieving available build targets...");
    auto targets = builder_->getAvailableTargets(buildDir);
    LOG_F(INFO, "Available targets retrieved.");
    return targets;
}

BuildResult BuildManager::buildTarget(const std::filesystem::path& buildDir,
                                      const std::string& target,
                                      std::optional<int> jobs) {
    LOG_F(INFO, "Building target: {}", target);
    auto result = builder_->buildTarget(buildDir, target, jobs);
    if (result.isSuccess()) {
        LOG_F(INFO, "Target build successful.");
    } else {
        LOG_F(ERROR, "Target build failed: {}", result.getMessage());
    }
    return result;
}

std::vector<std::pair<std::string, std::string>>
BuildManager::getCacheVariables(const std::filesystem::path& buildDir) {
    LOG_F(INFO, "Retrieving cache variables...");
    auto cacheVars = builder_->getCacheVariables(buildDir);
    LOG_F(INFO, "Cache variables retrieved.");
    return cacheVars;
}

bool BuildManager::setCacheVariable(const std::filesystem::path& buildDir,
                                    const std::string& name,
                                    const std::string& value) {
    LOG_F(INFO, "Setting cache variable: {} = {}", name, value);
    bool success = builder_->setCacheVariable(buildDir, name, value);
    if (success) {
        LOG_F(INFO, "Cache variable set successfully.");
    } else {
        LOG_F(ERROR, "Failed to set cache variable.");
    }
    return success;
}

void BuildManager::addBuildTask(const BuildTask& task) {
    taskChain_.emplace_back(task);
    LOG_F(INFO, "Build task added to the task chain.");
}

BuildResult BuildManager::executeTaskChain() {
    LOG_F(INFO, "Executing build task chain...");
    for (size_t i = 0; i < taskChain_.size(); ++i) {
        LOG_F(INFO, "Executing task %zu/%zu", i + 1, taskChain_.size());
        BuildResult result = taskChain_[i]();
        if (!result.isSuccess()) {
            LOG_F(ERROR, "Task %zu failed: {}", i + 1,
                  result.getMessage());
            return result;
        }
        LOG_F(INFO, "Task %zu completed successfully.", i + 1);
    }
    LOG_F(INFO, "All tasks in the task chain executed successfully.");
    return BuildResult(true, "All tasks executed successfully.", 0);
}

void BuildManager::clearTaskChain() {
    taskChain_.clear();
    LOG_F(INFO, "Build task chain cleared.");
}

}  // namespace lithium
