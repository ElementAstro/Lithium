#ifndef LITHIUM_ADDON_BUILDER_HPP
#define LITHIUM_ADDON_BUILDER_HPP

#include <filesystem>
#include <functional>
#include <memory>
#include <optional>

#include "platform/base.hpp"

namespace lithium {

class BuildManager {
public:
    enum class BuildSystemType { CMake, Meson };

    BuildManager(BuildSystemType type);

    auto configureProject(
        const std::filesystem::path& sourceDir,
        const std::filesystem::path& buildDir, BuildType buildType,
        const std::vector<std::string>& options) -> BuildResult;

    auto buildProject(const std::filesystem::path& buildDir,
                      std::optional<int> jobs = std::nullopt) -> BuildResult;

    auto cleanProject(const std::filesystem::path& buildDir) -> BuildResult;

    auto installProject(const std::filesystem::path& buildDir,
                        const std::filesystem::path& installDir) -> BuildResult;

    auto runTests(const std::filesystem::path& buildDir,
                  const std::vector<std::string>& testNames = {})
        -> BuildResult;

    auto generateDocs(const std::filesystem::path& buildDir,
                      const std::filesystem::path& outputDir) -> BuildResult;

    auto loadConfig(const std::filesystem::path& configPath) -> bool;

    auto setLogCallback(std::function<void(const std::string&)> callback)
        -> void;

    auto getAvailableTargets(const std::filesystem::path& buildDir)
        -> std::vector<std::string>;

    auto buildTarget(const std::filesystem::path& buildDir,
                     const std::string& target,
                     std::optional<int> jobs = std::nullopt) -> BuildResult;

    auto getCacheVariables(const std::filesystem::path& buildDir)
        -> std::vector<std::pair<std::string, std::string>>;

    auto setCacheVariable(const std::filesystem::path& buildDir,
                          const std::string& name,
                          const std::string& value) -> bool;

private:
    std::unique_ptr<BuildSystem> builder_;
};

}  // namespace lithium

#endif  // LITHIUM_ADDON_BUILDER_HPP