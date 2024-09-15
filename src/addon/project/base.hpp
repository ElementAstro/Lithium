#ifndef LITHIUM_ADDON_VCSMANAGER_HPP
#define LITHIUM_ADDON_VCSMANAGER_HPP

#include <chrono>
#include <optional>
#include <string>
#include <vector>

namespace lithium {

struct CommitInfo {
    std::string id;
    std::string author;
    std::string message;
    std::chrono::system_clock::time_point timestamp;
};

class VcsManager {
public:
    virtual ~VcsManager() = default;

    virtual auto initRepository() -> bool = 0;
    virtual auto cloneRepository(const std::string& url) -> bool = 0;
    virtual auto createBranch(const std::string& branchName) -> bool = 0;
    virtual auto checkoutBranch(const std::string& branchName) -> bool = 0;
    virtual auto mergeBranch(const std::string& branchName) -> bool = 0;
    virtual auto addFile(const std::string& filePath) -> bool = 0;
    virtual auto commitChanges(const std::string& message) -> bool = 0;
    virtual auto pull(const std::string& remoteName,
                      const std::string& branchName) -> bool = 0;
    virtual auto push(const std::string& remoteName,
                      const std::string& branchName) -> bool = 0;
    virtual auto getLog(int limit = 10) -> std::vector<CommitInfo> = 0;
    virtual auto getCurrentBranch() -> std::optional<std::string> = 0;
    virtual auto getBranches() -> std::vector<std::string> = 0;
    virtual auto getStatus()
        -> std::vector<std::pair<std::string, std::string>> = 0;
    virtual auto revertCommit(const std::string& commitId) -> bool = 0;
    virtual auto createTag(const std::string& tagName,
                           const std::string& message) -> bool = 0;
};

}  // namespace lithium

#endif  // LITHIUM_ADDON_VCSMANAGER_HPP