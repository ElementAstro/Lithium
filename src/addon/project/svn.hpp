#ifndef LITHIUM_ADDON_SVNMANAGER_HPP
#define LITHIUM_ADDON_SVNMANAGER_HPP

#include <memory>
#include <string>
#include <vector>
#include "base.hpp"

namespace lithium {

constexpr int DEFAULT_LOG_LIMIT = 10;

class SvnManager : public VcsManager {
public:
    explicit SvnManager(const std::string& repoPath);
    ~SvnManager() override;

    auto initRepository() -> bool override;
    auto cloneRepository(const std::string& url) -> bool override;
    auto createBranch(const std::string& branchName) -> bool override;
    auto checkoutBranch(const std::string& branchName) -> bool override;
    auto mergeBranch(const std::string& branchName) -> bool override;
    auto addFile(const std::string& filePath) -> bool override;
    auto commitChanges(const std::string& message) -> bool override;
    auto pull(const std::string& remoteName,
              const std::string& branchName) -> bool override;
    auto push(const std::string& remoteName,
              const std::string& branchName) -> bool override;
    auto getLog(int limit = DEFAULT_LOG_LIMIT)
        -> std::vector<CommitInfo> override;
    auto getCurrentBranch() -> std::optional<std::string> override;
    auto getBranches() -> std::vector<std::string> override;
    auto getStatus()
        -> std::vector<std::pair<std::string, std::string>> override;
    auto revertCommit(const std::string& commitId) -> bool override;
    auto createTag(const std::string& tagName,
                   const std::string& message) -> bool override;

    SvnManager(const SvnManager&) = delete;
    SvnManager& operator=(const SvnManager&) = delete;
    SvnManager(SvnManager&&) = default;
    SvnManager& operator=(SvnManager&&) = default;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace lithium

#endif  // LITHIUM_ADDON_SVNMANAGER_HPP