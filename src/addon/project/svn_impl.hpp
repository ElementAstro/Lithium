#ifndef LITHIUM_ADDON_SVNMANAGERIMPL_HPP
#define LITHIUM_ADDON_SVNMANAGERIMPL_HPP

#include <string>
#include <vector>

#include "svn.hpp"

#include <svn_client.h>

namespace lithium {
class SvnManager::Impl {
public:
    Impl(const std::string& repoPath);
    ~Impl();

    bool checkout(const std::string& url, const std::string& revision);
    bool addFile(const std::string& filePath);
    bool commitChanges(const std::string& message);
    bool update();
    bool createBranch(const std::string& branchName);
    bool mergeBranch(const std::string& branchName);
    std::vector<std::string> getLog(int limit);

private:
    std::string repoPath;
    svn_client_ctx_t* ctx;
    apr_pool_t* pool;

    void printError(svn_error_t* err);
};

}  // namespace lithium

#endif  // LITHIUM_ADDON_SVNMANAGERIMPL_HPP
