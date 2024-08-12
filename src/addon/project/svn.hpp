#ifndef LITHIUM_ADDON_SVNMANAGER_HPP
#define LITHIUM_ADDON_SVNMANAGER_HPP

#include <memory>
#include <string>
#include <vector>

namespace lithium {
class SvnManager {
public:
    SvnManager(const std::string& repoPath);
    ~SvnManager();

    auto checkout(const std::string& url, const std::string& revision) -> bool;
    auto addFile(const std::string& filePath) -> bool;
    auto commitChanges(const std::string& message) -> bool;
    auto update() -> bool;
    auto createBranch(const std::string& branchName) -> bool;
    auto mergeBranch(const std::string& branchName) -> bool;
    auto getLog(int limit = 10) -> std::vector<std::string>;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};
}  // namespace lithium

#endif  // LITHIUM_ADDON_SVNMANAGER_HPP
