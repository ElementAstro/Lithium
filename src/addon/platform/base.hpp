#ifndef LITHIUM_ADDON_BUILD_BASE_HPP
#define LITHIUM_ADDON_BUILD_BASE_HPP

#include <filesystem>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace lithium {

/**
 * @enum BuildType
 * @brief Represents the type of build configuration.
 */
enum class BuildType {
    DEBUG,             /**< Debug build configuration. */
    RELEASE,           /**< Release build configuration. */
    REL_WITH_DEB_INFO, /**< Release with Debug Information. */
    MIN_SIZE_REL       /**< Minimum Size Release. */
};

/**
 * @struct BuildResult
 * @brief Represents the result of a build operation.
 */
struct BuildResult {
private:
    bool success_; /**< Indicates if the build operation was successful. */
    std::string
        message_; /**< Contains messages or errors from the build operation. */
    int exit_code_; /**< The exit code returned by the build system. */

public:
    explicit BuildResult(bool success = true, std::string msg = "",
                         int code = 0)
        : success_(success), message_(std::move(msg)), exit_code_(code) {}

    bool isSuccess() const { return success_; }
    const std::string& getMessage() const { return message_; }
    int getExitCode() const { return exit_code_; }
};

/**
 * @class BuildSystem
 * @brief Abstract base class for different build system implementations.
 */
class BuildSystem {
public:
    virtual ~BuildSystem() = default;

    virtual auto configureProject(
        const std::filesystem::path& sourceDir,
        const std::filesystem::path& buildDir, BuildType buildType,
        const std::vector<std::string>& options,
        const std::map<std::string, std::string>& envVars) -> BuildResult = 0;

    virtual auto buildProject(const std::filesystem::path& buildDir,
                              std::optional<int> jobs) -> BuildResult = 0;

    virtual auto cleanProject(const std::filesystem::path& buildDir)
        -> BuildResult = 0;

    virtual auto installProject(const std::filesystem::path& buildDir,
                                const std::filesystem::path& installDir)
        -> BuildResult = 0;

    virtual auto runTests(const std::filesystem::path& buildDir,
                          const std::vector<std::string>& testNames)
        -> BuildResult = 0;

    virtual auto generateDocs(const std::filesystem::path& buildDir,
                              const std::filesystem::path& outputDir)
        -> BuildResult = 0;

    virtual auto loadConfig(const std::filesystem::path& configPath)
        -> bool = 0;

    virtual auto getAvailableTargets(const std::filesystem::path& buildDir)
        -> std::vector<std::string> = 0;

    virtual auto buildTarget(const std::filesystem::path& buildDir,
                             const std::string& target,
                             std::optional<int> jobs) -> BuildResult = 0;

    virtual auto getCacheVariables(const std::filesystem::path& buildDir)
        -> std::vector<std::pair<std::string, std::string>> = 0;

    virtual auto setCacheVariable(const std::filesystem::path& buildDir,
                                  const std::string& name,
                                  const std::string& value) -> bool = 0;
};

}  // namespace lithium

#endif  // LITHIUM_ADDON_BUILD_BASE_HPP
