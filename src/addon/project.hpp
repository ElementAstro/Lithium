#ifndef LITHIUM_ADDON_PROJECT_HPP
#define LITHIUM_ADDON_PROJECT_HPP

#include <memory>
#include <string>
#include <vector>

#include "project/base.hpp"

namespace lithium {
class ProjectManager {
public:
    enum class VcsType { Git, Svn };

    ProjectManager(VcsType type, const std::string& repoPath);
    ~ProjectManager();

    auto initRepository() -> bool;
    auto cloneRepository(const std::string& url) -> bool;
    auto createBranch(const std::string& branchName) -> bool;
    auto checkoutBranch(const std::string& branchName) -> bool;
    auto mergeBranch(const std::string& branchName) -> bool;
    auto addFile(const std::string& filePath) -> bool;
    auto commitChanges(const std::string& message) -> bool;
    auto pull(const std::string& remoteName, const std::string& branchName) -> bool;
    auto push(const std::string& remoteName, const std::string& branchName) -> bool;
    auto getLog(int limit = 10) -> std::vector<std::string>;
    auto update() -> bool;

private:
    std::unique_ptr<VcsManager> vcsManager;
};

}  // namespace lithium

#endif