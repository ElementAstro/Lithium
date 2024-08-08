#ifndef LITHIUM_ADDON_BUILDBASE_HPP
#define LITHIUM_ADDON_BUILDBASE_HPP

#include <string>
#include <vector>

namespace lithium {
class BuildSystem {
public:
    virtual ~BuildSystem() = default;

    virtual auto configureProject(
        const std::string &sourceDir, const std::string &buildDir,
        const std::string &buildType,
        const std::vector<std::string> &options) -> bool = 0;
    virtual auto buildProject(const std::string &buildDir,
                              int jobs) -> bool = 0;
    virtual auto cleanProject(const std::string &buildDir) -> bool = 0;
    virtual auto installProject(const std::string &buildDir,
                                const std::string &installDir) -> bool = 0;
    virtual auto runTests(const std::string &buildDir) -> bool = 0;
    virtual auto generateDocs(const std::string &buildDir) -> bool = 0;
    virtual auto loadConfig(const std::string &configPath) -> bool = 0;
};
}  // namespace lithium

#endif  // LITHIUM_ADDON_BUILDBASE_HPP
