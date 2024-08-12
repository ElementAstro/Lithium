#ifndef LITHIUM_ADDON_GITMANAGER_HPP
#define LITHIUM_ADDON_GITMANAGER_HPP

#include <memory>
#include <string>

namespace lithium {
class GitManager {
public:
    explicit GitManager(const std::string& repoPath);
    ~GitManager();

    bool initRepository();
    bool cloneRepository(const std::string& url);
    bool createBranch(const std::string& branchName);
    bool checkoutBranch(const std::string& branchName);
    bool mergeBranch(const std::string& branchName);
    bool addFile(const std::string& filePath);
    bool commitChanges(const std::string& message);
    bool pull(const std::string& remoteName, const std::string& branchName);
    bool push(const std::string& remoteName, const std::string& branchName);

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};
}  // namespace lithium

#endif  // LITHIUM_ADDON_GITMANAGER_HPP
