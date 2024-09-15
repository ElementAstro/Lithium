#ifndef LITHIUM_ADDON_SVNMANAGERIMPL_HPP
#define LITHIUM_ADDON_SVNMANAGERIMPL_HPP

#include "svn.hpp"
#include <svn_client.h>

namespace lithium {
class SvnManager::Impl {
public:
    Impl(const std::string& repoPath);
    ~Impl();

    bool initRepository();
    bool cloneRepository(const std::string& url);
    bool createBranch(const std::string& branchName);
    bool checkoutBranch(const std::string& branchName);
    bool mergeBranch(const std::string& branchName);
    bool addFile(const std::string& filePath);
    bool commitChanges(const std::string& message);
    bool pull(const std::string& remoteName, const std::string& branchName);
    bool push(const std::string& remoteName, const std::string& branchName);
    std::vector<CommitInfo> getLog(int limit);
    std::optional<std::string> getCurrentBranch();
    std::vector<std::string> getBranches();
    std::vector<std::pair<std::string, std::string>> getStatus();
    bool revertCommit(const std::string& commitId);
    bool createTag(const std::string& tagName, const std::string& message);

private:
    std::string repoPath;
    svn_client_ctx_t* ctx;
    apr_pool_t* pool;

    void printError(svn_error_t* err);
};

}  // namespace lithium

#endif  // LITHIUM_ADDON_SVNMANAGERIMPL_HPP
