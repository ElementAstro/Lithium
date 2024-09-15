#ifndef LITHIUM_ADDON_BUILDBASE_HPP
#define LITHIUM_ADDON_BUILDBASE_HPP

#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace lithium {

enum class BuildType { Debug, Release, RelWithDebInfo, MinSizeRel };

struct BuildResult {
    bool success;
    std::string output;
    std::string error;
};

class BuildSystem {
public:
    virtual ~BuildSystem() = default;

    virtual auto configureProject(
        const std::filesystem::path& sourceDir,
        const std::filesystem::path& buildDir, BuildType buildType,
        const std::vector<std::string>& options) -> BuildResult = 0;

    virtual auto buildProject(const std::filesystem::path& buildDir,
                              std::optional<int> jobs = std::nullopt)
        -> BuildResult = 0;

    virtual auto cleanProject(const std::filesystem::path& buildDir)
        -> BuildResult = 0;

    virtual auto installProject(const std::filesystem::path& buildDir,
                                const std::filesystem::path& installDir)
        -> BuildResult = 0;

    virtual auto runTests(const std::filesystem::path& buildDir,
                          const std::vector<std::string>& testNames = {})
        -> BuildResult = 0;

    virtual auto generateDocs(const std::filesystem::path& buildDir,
                              const std::filesystem::path& outputDir)
        -> BuildResult = 0;

    virtual auto loadConfig(const std::filesystem::path& configPath)
        -> bool = 0;

    virtual auto setLogCallback(
        std::function<void(const std::string&)> callback) -> void = 0;

    virtual auto getAvailableTargets(const std::filesystem::path& buildDir)
        -> std::vector<std::string> = 0;

    virtual auto buildTarget(
        const std::filesystem::path& buildDir, const std::string& target,
        std::optional<int> jobs = std::nullopt) -> BuildResult = 0;

    virtual auto getCacheVariables(const std::filesystem::path& buildDir)
        -> std::vector<std::pair<std::string, std::string>> = 0;

    virtual auto setCacheVariable(const std::filesystem::path& buildDir,
                                  const std::string& name,
                                  const std::string& value) -> bool = 0;
};

}  // namespace lithium

#endif  // LITHIUM_ADDON_BUILDBASE_HPP
