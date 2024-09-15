#ifndef LITHIUM_ADDON_SVNMANAGER_HPP
#define LITHIUM_ADDON_SVNMANAGER_HPP

#include <memory>
#include <string>
#include <vector>
#include "base.hpp"

namespace lithium {
class SvnManager : public VcsManager {
public:
    SvnManager(const std::string& repoPath);
    ~SvnManager() override;

    bool initRepository() override;
    bool cloneRepository(const std::string& url) override;
    bool createBranch(const std::string& branchName) override;
    bool checkoutBranch(const std::string& branchName) override;
    bool mergeBranch(const std::string& branchName) override;
    bool addFile(const std::string& filePath) override;
    bool commitChanges(const std::string& message) override;
    bool pull(const std::string& remoteName,
              const std::string& branchName) override;
    bool push(const std::string& remoteName,
              const std::string& branchName) override;
    std::vector<CommitInfo> getLog(int limit = 10) override;
    std::optional<std::string> getCurrentBranch() override;
    std::vector<std::string> getBranches() override;
    std::vector<std::pair<std::string, std::string>> getStatus() override;
    bool revertCommit(const std::string& commitId) override;
    bool createTag(const std::string& tagName,
                   const std::string& message) override;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};
}  // namespace lithium

#endif  // LITHIUM_ADDON_SVNMANAGER_HPP
