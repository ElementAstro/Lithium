#include "builder.hpp"

#include "platform/cmake.hpp"
#include "platform/meson.hpp"

#include "atom/error/exception.hpp"

namespace lithium {

BuildManager::BuildManager(BuildSystemType type) {
    switch (type) {
        case BuildSystemType::CMake:
            builder_ = std::make_unique<CMakeBuilder>();
            break;
        case BuildSystemType::Meson:
            builder_ = std::make_unique<MesonBuilder>();
            break;
        default:
            THROW_INVALID_ARGUMENT("Unsupported build system type");
    }
}

auto BuildManager::configureProject(
    const std::filesystem::path& sourceDir,
    const std::filesystem::path& buildDir, BuildType buildType,
    const std::vector<std::string>& options) -> BuildResult {
    return builder_->configureProject(sourceDir, buildDir, buildType, options);
}

auto BuildManager::buildProject(const std::filesystem::path& buildDir,
                                std::optional<int> jobs) -> BuildResult {
    return builder_->buildProject(buildDir, jobs);
}

auto BuildManager::cleanProject(const std::filesystem::path& buildDir)
    -> BuildResult {
    return builder_->cleanProject(buildDir);
}

auto BuildManager::installProject(const std::filesystem::path& buildDir,
                                  const std::filesystem::path& installDir)
    -> BuildResult {
    return builder_->installProject(buildDir, installDir);
}

auto BuildManager::runTests(const std::filesystem::path& buildDir,
                            const std::vector<std::string>& testNames)
    -> BuildResult {
    return builder_->runTests(buildDir, testNames);
}

auto BuildManager::generateDocs(const std::filesystem::path& buildDir,
                                const std::filesystem::path& outputDir)
    -> BuildResult {
    return builder_->generateDocs(buildDir, outputDir);
}

auto BuildManager::loadConfig(const std::filesystem::path& configPath) -> bool {
    return builder_->loadConfig(configPath);
}

auto BuildManager::setLogCallback(
    std::function<void(const std::string&)> callback) -> void {
    builder_->setLogCallback(std::move(callback));
}

auto BuildManager::getAvailableTargets(const std::filesystem::path& buildDir)
    -> std::vector<std::string> {
    return builder_->getAvailableTargets(buildDir);
}

auto BuildManager::buildTarget(const std::filesystem::path& buildDir,
                               const std::string& target,
                               std::optional<int> jobs) -> BuildResult {
    return builder_->buildTarget(buildDir, target, jobs);
}

auto BuildManager::getCacheVariables(const std::filesystem::path& buildDir)
    -> std::vector<std::pair<std::string, std::string>> {
    return builder_->getCacheVariables(buildDir);
}

auto BuildManager::setCacheVariable(const std::filesystem::path& buildDir,
                                    const std::string& name,
                                    const std::string& value) -> bool {
    return builder_->setCacheVariable(buildDir, name, value);
}

}  // namespace lithium
