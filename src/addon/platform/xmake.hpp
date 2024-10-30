#ifndef LITHIUM_ADDON_XMAKE_BUILDER_HPP
#define LITHIUM_ADDON_XMAKE_BUILDER_HPP

#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "base.hpp"

namespace lithium {

/**
 * @struct XMakeBuilderConfig
 * @brief Stores configuration for XMakeBuilder.
 */
struct alignas(128) XMakeBuilderConfig {
    BuildType buildType;
    std::vector<std::string> options;
    std::map<std::string, std::string> envVars;
};

/**
 * @class XMakeBuilder
 * @brief Implementation of the BuildSystem interface for XMake.
 */
class XMakeBuilder : public BuildSystem {
public:
    XMakeBuilder();
    ~XMakeBuilder() override;

    auto configureProject(const std::filesystem::path& sourceDir,
                          const std::filesystem::path& buildDir,
                          BuildType buildType,
                          const std::vector<std::string>& options,
                          const std::map<std::string, std::string>& envVars)
        -> BuildResult override;

    auto buildProject(const std::filesystem::path& buildDir,
                      std::optional<int> jobs) -> BuildResult override;

    auto cleanProject(const std::filesystem::path& buildDir)
        -> BuildResult override;

    auto installProject(const std::filesystem::path& buildDir,
                        const std::filesystem::path& installDir)
        -> BuildResult override;

    auto runTests(const std::filesystem::path& buildDir,
                  const std::vector<std::string>& testNames)
        -> BuildResult override;

    auto generateDocs(const std::filesystem::path& buildDir,
                      const std::filesystem::path& outputDir)
        -> BuildResult override;

    auto loadConfig(const std::filesystem::path& configPath) -> bool override;

    auto getAvailableTargets(const std::filesystem::path& buildDir)
        -> std::vector<std::string> override;

    auto buildTarget(const std::filesystem::path& buildDir,
                     const std::string& target,
                     std::optional<int> jobs) -> BuildResult override;

    auto getCacheVariables(const std::filesystem::path& buildDir)
        -> std::vector<std::pair<std::string, std::string>> override;

    auto setCacheVariable(const std::filesystem::path& buildDir,
                          const std::string& name,
                          const std::string& value) -> bool override;

    // Deleted copy constructor and copy assignment operator
    XMakeBuilder(const XMakeBuilder&) = delete;
    XMakeBuilder& operator=(const XMakeBuilder&) = delete;

    // Defaulted move constructor and move assignment operator
    XMakeBuilder(XMakeBuilder&&) noexcept = default;
    XMakeBuilder& operator=(XMakeBuilder&&) noexcept = default;

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl_;
};

}  // namespace lithium

#endif  // LITHIUM_ADDON_XMAKE_BUILDER_HPP