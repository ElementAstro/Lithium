#ifndef LITHIUM_ADDON_BUILDER_HPP
#define LITHIUM_ADDON_BUILDER_HPP

#include <memory>

#include "platform/base.hpp"

namespace lithium {
class BuildManager {
public:
    enum class BuildSystemType { CMake, Meson };

    BuildManager(BuildSystemType type);
    auto configureProject(const std::string &sourceDir,
                          const std::string &buildDir,
                          const std::string &buildType,
                          const std::vector<std::string> &options) -> bool;
    auto buildProject(const std::string &buildDir, int jobs) -> bool;
    auto cleanProject(const std::string &buildDir) -> bool;
    auto installProject(const std::string &buildDir,
                        const std::string &installDir) -> bool;
    auto runTests(const std::string &buildDir) -> bool;
    auto generateDocs(const std::string &buildDir) -> bool;
    auto loadConfig(const std::string &configPath) -> bool;

private:
    std::unique_ptr<BuildSystem> builder_;
};
}  // namespace lithium

#endif
