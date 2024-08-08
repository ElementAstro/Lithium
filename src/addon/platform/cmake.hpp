#ifndef LITHIUM_ADDON_CMAKEBUILDER_HPP
#define LITHIUM_ADDON_CMAKEBUILDER_HPP

#include <memory>
#include <string>
#include <vector>

#include "base.hpp"

namespace lithium {
class CMakeBuilderImpl;
class CMakeBuilder : public BuildSystem {
public:
    CMakeBuilder();
    ~CMakeBuilder() override;

    auto configureProject(
        const std::string &sourceDir, const std::string &buildDir,
        const std::string &buildType,
        const std::vector<std::string> &options) -> bool override;
    auto buildProject(const std::string &buildDir, int jobs) -> bool override;
    auto cleanProject(const std::string &buildDir) -> bool override;
    auto installProject(const std::string &buildDir,
                        const std::string &installDir) -> bool override;
    auto runTests(const std::string &buildDir) -> bool override;
    auto generateDocs(const std::string &buildDir) -> bool override;
    auto loadConfig(const std::string &configPath) -> bool override;

private:
    std::unique_ptr<CMakeBuilderImpl> pImpl_;

    auto checkAndInstallDependencies() -> bool;
};
}  // namespace lithium

#endif  // LITHIUM_ADDON_CMAKEBUILDER_HPP