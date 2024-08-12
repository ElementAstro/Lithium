#ifndef LITHIUM_ADDON_XMAKEBUILDER_HPP
#define LITHIUM_ADDON_XMAKEBUILDER_HPP

#include <memory>
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
    std::unique_ptr<XMakeBuilderImpl> pImpl_;

    auto checkAndInstallDependencies() -> bool;
};
}  // namespace lithium

#endif  // LITHIUM_ADDON_XMAKEBUILDER_HPP
