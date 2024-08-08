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
    const std::string &sourceDir, const std::string &buildDir,
    const std::string &buildType,
    const std::vector<std::string> &options) -> bool {
    return builder_->configureProject(sourceDir, buildDir, buildType, options);
}

auto BuildManager::buildProject(const std::string &buildDir, int jobs) -> bool {
    return builder_->buildProject(buildDir, jobs);
}

auto BuildManager::cleanProject(const std::string &buildDir) -> bool {
    return builder_->cleanProject(buildDir);
}

auto BuildManager::installProject(const std::string &buildDir,
                                  const std::string &installDir) -> bool {
    return builder_->installProject(buildDir, installDir);
}

auto BuildManager::runTests(const std::string &buildDir) -> bool {
    return builder_->runTests(buildDir);
}

auto BuildManager::generateDocs(const std::string &buildDir) -> bool {
    return builder_->generateDocs(buildDir);
}

auto BuildManager::loadConfig(const std::string &configPath) -> bool {
    return builder_->loadConfig(configPath);
}

}  // namespace lithium
