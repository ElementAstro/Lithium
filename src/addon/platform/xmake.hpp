#ifndef LITHIUM_ADDON_XMAKEBUILDER_HPP
#define LITHIUM_ADDON_XMAKEBUILDER_HPP

#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "base.hpp"

namespace lithium {

class XMakeBuilderImpl;

class XMakeBuilder : public BuildSystem {
public:
    XMakeBuilder();
    ~XMakeBuilder() override;

    auto configureProject(
        const std::filesystem::path& sourceDir,
        const std::filesystem::path& buildDir, BuildType buildType,
        const std::vector<std::string>& options) -> BuildResult override;

    auto buildProject(const std::filesystem::path& buildDir,
                      std::optional<int> jobs = std::nullopt)
        -> BuildResult override;

    auto cleanProject(const std::filesystem::path& buildDir)
        -> BuildResult override;

    auto installProject(const std::filesystem::path& buildDir,
                        const std::filesystem::path& installDir)
        -> BuildResult override;

    auto runTests(const std::filesystem::path& buildDir,
                  const std::vector<std::string>& testNames = {})
        -> BuildResult override;

    auto generateDocs(const std::filesystem::path& buildDir,
                      const std::filesystem::path& outputDir)
        -> BuildResult override;

    auto loadConfig(const std::filesystem::path& configPath) -> bool override;

    auto setLogCallback(std::function<void(const std::string&)> callback)
        -> void override;

    auto getAvailableTargets(const std::filesystem::path& buildDir)
        -> std::vector<std::string> override;

    auto buildTarget(
        const std::filesystem::path& buildDir, const std::string& target,
        std::optional<int> jobs = std::nullopt) -> BuildResult override;

    auto getCacheVariables(const std::filesystem::path& buildDir)
        -> std::vector<std::pair<std::string, std::string>> override;

    auto setCacheVariable(const std::filesystem::path& buildDir,
                          const std::string& name,
                          const std::string& value) -> bool override;

private:
    std::unique_ptr<XMakeBuilderImpl> pImpl_;
    auto checkAndInstallDependencies() -> bool;
};

}  // namespace lithium

#endif  // LITHIUM_ADDON_XMAKEBUILDER_HPP
