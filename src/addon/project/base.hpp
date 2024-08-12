#ifndef LITHIUM_ADDON_VCSMANAGER_HPP
#define LITHIUM_ADDON_VCSMANAGER_HPP

#include <string>
#include <vector>

namespace lithium {
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
    virtual auto getLog(int limit = 10) -> std::vector<std::string> = 0;
};

}  // namespace lithium

#endif  // LITHIUM_ADDON_VCSMANAGER_HPP
