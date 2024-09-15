#include "svn_impl.hpp"

#include <svn_cmdline.h>
#include <svn_path.h>
#include <svn_pools.h>
#include <iostream>

namespace lithium {
SvnManager::Impl::Impl(const std::string& repoPath)
    : repoPath(repoPath), ctx(nullptr), pool(nullptr) {
    apr_initialize();
    pool = svn_pool_create(nullptr);

    svn_client_create_context2(&ctx, nullptr, pool);
}

SvnManager::Impl::~Impl() {
    if (pool) {
        svn_pool_destroy(pool);
    }
    apr_terminate();
}

bool SvnManager::Impl::initRepository() {
    svn_error_t* err = svn_client_create_context2(&ctx, nullptr, pool);
    if (err) {
        printError(err);
        return false;
    }
    return true;
}

bool SvnManager::Impl::cloneRepository(const std::string& url) {
    return checkout(url, "HEAD");
}

void SvnManager::Impl::printError(svn_error_t* err) {
    if (err) {
        char buf[1024];
        svn_error_clear(svn_cmdline_cstring_from_utf8_fuzzy(buf, sizeof(buf),
                                                            err->message));
        std::cerr << "SVN Error: " << buf << std::endl;
        svn_error_clear(err);
    }
}

bool SvnManager::Impl::checkout(const std::string& url,
                                const std::string& revision) {
    svn_revnum_t rev;
    svn_opt_revision_t peg_revision = {svn_opt_revision_unspecified};
    svn_opt_revision_t opt_revision = {svn_opt_revision_unspecified};

    opt_revision.kind = svn_opt_revision_head;
    svn_error_t* err = svn_client_checkout3(
        &rev, url.c_str(), repoPath.c_str(), &peg_revision, &opt_revision,
        svn_depth_infinity, FALSE, FALSE, ctx, pool);
    if (err) {
        printError(err);
        return false;
    }
    return true;
}

bool SvnManager::Impl::addFile(const std::string& filePath) {
    svn_error_t* err = svn_client_add4(filePath.c_str(), svn_depth_infinity,
                                       FALSE, FALSE, FALSE, ctx, pool);
    if (err) {
        printError(err);
        return false;
    }
    return true;
}

bool SvnManager::Impl::commitChanges(const std::string& message) {
    apr_array_header_t* targets = apr_array_make(pool, 1, sizeof(const char*));
    APR_ARRAY_PUSH(targets, const char*) = repoPath.c_str();

    svn_commit_info_t* commit_info = nullptr;
    svn_error_t* err =
        svn_client_commit6(targets, svn_depth_infinity, FALSE, FALSE, FALSE,
                           nullptr, message.c_str(), nullptr, ctx, pool);
    if (err) {
        printError(err);
        return false;
    }
    return true;
}

bool SvnManager::Impl::update() {
    apr_array_header_t* targets = apr_array_make(pool, 1, sizeof(const char*));
    APR_ARRAY_PUSH(targets, const char*) = repoPath.c_str();

    svn_error_t* err =
        svn_client_update3(nullptr, targets, svn_depth_infinity, FALSE, FALSE,
                           FALSE, FALSE, ctx, pool);
    if (err) {
        printError(err);
        return false;
    }
    return true;
}

bool SvnManager::Impl::createBranch(const std::string& branchName) {
    std::string branchUrl = repoPath + "/branches/" + branchName;
    svn_opt_revision_t revision = {svn_opt_revision_head};

    svn_error_t* err = svn_client_copy4(nullptr, repoPath.c_str(), &revision,
                                        branchUrl.c_str(), svn_depth_infinity,
                                        FALSE, FALSE, nullptr, ctx, pool);
    if (err) {
        printError(err);
        return false;
    }
    return true;
}

bool SvnManager::Impl::mergeBranch(const std::string& branchName) {
    std::string branchUrl = repoPath + "/branches/" + branchName;
    svn_opt_revision_t peg_revision = {svn_opt_revision_head};
    svn_opt_revision_t revision = {svn_opt_revision_head};

    svn_error_t* err = svn_client_merge_peg5(
        branchUrl.c_str(), nullptr, &peg_revision, &revision, repoPath.c_str(),
        svn_depth_infinity, FALSE, FALSE, FALSE, FALSE, FALSE, nullptr, ctx,
        pool);
    if (err) {
        printError(err);
        return false;
    }
    return true;
}

std::vector<std::string> SvnManager::Impl::getLog(int limit) {
    std::vector<std::string> logMessages;

    struct LogReceiverBaton {
        std::vector<std::string>* logMessages;
        int remaining;
    };

    LogReceiverBaton baton = {&logMessages, limit};

    auto log_receiver = [](void* baton, svn_log_entry_t* log_entry,
                           apr_pool_t* pool) -> svn_error_t* {
        auto* b = static_cast<LogReceiverBaton*>(baton);
        if (b->remaining-- <= 0)
            return SVN_NO_ERROR;
        b->logMessages->emplace_back(
            log_entry->revprops->nelts > 0
                ? svn_string_value(log_entry->revprops->nelts)
                : "No message");
        return SVN_NO_ERROR;
    };

    svn_opt_revision_t start = {svn_opt_revision_head};
    svn_opt_revision_t end = {svn_opt_revision_number, 0};

    svn_error_t* err =
        svn_client_log5(repoPath.c_str(), nullptr, &start, &end, 0, FALSE, TRUE,
                        FALSE, apr_array_make(pool, 1, sizeof(const char*)),
                        log_receiver, &baton, ctx, pool);
    if (err) {
        printError(err);
        return {};
    }

    return logMessages;
}

bool SvnManager::Impl::checkoutBranch(const std::string& branchName) {
    std::string branchUrl = repoPath + "/branches/" + branchName;
    return checkout(branchUrl, "HEAD");
}

bool SvnManager::Impl::pull(const std::string& remoteName,
                            const std::string& branchName) {
    // SVN doesn't have a direct equivalent to Git's pull, so we'll just update
    return update();
}

bool SvnManager::Impl::push(const std::string& remoteName,
                            const std::string& branchName) {
    // SVN doesn't have a direct equivalent to Git's push, as changes are
    // immediately visible after commit
    return true;
}

std::vector<CommitInfo> SvnManager::Impl::getLog(int limit) {
    std::vector<CommitInfo> logEntries;

    struct LogReceiverBaton {
        std::vector<CommitInfo>* logEntries;
        int remaining;
    };

    LogReceiverBaton baton = {&logEntries, limit};

    auto log_receiver = [](void* baton, svn_log_entry_t* log_entry,
                           apr_pool_t* pool) -> svn_error_t* {
        auto* b = static_cast<LogReceiverBaton*>(baton);
        if (b->remaining-- <= 0)
            return SVN_NO_ERROR;

        CommitInfo info;
        info.id = std::to_string(log_entry->revision);
        info.author = svn_prop_get_value(log_entry->revprops, "svn:author")
                          ? svn_string_value(svn_prop_get_value(
                                log_entry->revprops, "svn:author"))
                          : "";
        info.message = svn_prop_get_value(log_entry->revprops, "svn:log")
                           ? svn_string_value(svn_prop_get_value(
                                 log_entry->revprops, "svn:log"))
                           : "";

        apr_time_t commit_time = 0;
        if (svn_prop_get_value(log_entry->revprops, "svn:date")) {
            svn_time_from_cstring(&commit_time,
                                  svn_string_value(svn_prop_get_value(
                                      log_entry->revprops, "svn:date")),
                                  pool);
        }
        info.timestamp =
            std::chrono::system_clock::from_time_t(apr_time_sec(commit_time));

        b->logEntries->push_back(info);
        return SVN_NO_ERROR;
    };

    svn_opt_revision_t start = {svn_opt_revision_head};
    svn_opt_revision_t end = {svn_opt_revision_number, 0};

    svn_error_t* err = svn_client_log5(
        repoPath.c_str(), nullptr, &start, &end, limit, TRUE, TRUE, FALSE,
        apr_array_make(pool, 1, sizeof(const char*)), log_receiver, &baton, ctx,
        pool);

    if (err) {
        printError(err);
        return {};
    }

    return logEntries;
}

std::optional<std::string> SvnManager::Impl::getCurrentBranch() {
    // SVN doesn't have a direct equivalent to Git's current branch concept
    // We can return the current working copy root URL instead
    const char* url;
    svn_error_t* err =
        svn_client_get_repos_root(&url, nullptr, repoPath.c_str(), ctx, pool);
    if (err) {
        printError(err);
        return std::nullopt;
    }
    return std::string(url);
}

std::vector<std::string> SvnManager::Impl::getBranches() {
    std::vector<std::string> branches;
    const char* url;
    svn_error_t* err =
        svn_client_get_repos_root(&url, nullptr, repoPath.c_str(), ctx, pool);
    if (err) {
        printError(err);
        return branches;
    }

    std::string branchesUrl = std::string(url) + "/branches";
    svn_opt_revision_t revision = {svn_opt_revision_head};

    auto list_receiver = [](void* baton, const char* path,
                            const svn_dirent_t* dirent, const svn_lock_t* lock,
                            const char* abs_path,
                            apr_pool_t* pool) -> svn_error_t* {
        auto* branches = static_cast<std::vector<std::string>*>(baton);
        branches->push_back(svn_path_basename(path, pool));
        return SVN_NO_ERROR;
    };

    err = svn_client_list3(branchesUrl.c_str(), &revision, &revision,
                           svn_depth_immediates, SVN_DIRENT_ALL, FALSE,
                           list_receiver, &branches, ctx, pool);
    if (err) {
        printError(err);
    }

    return branches;
}

std::vector<std::pair<std::string, std::string>> SvnManager::Impl::getStatus() {
    std::vector<std::pair<std::string, std::string>> status;

    auto status_receiver = [](void* baton, const char* path,
                              const svn_client_status_t* status,
                              apr_pool_t* pool) -> svn_error_t* {
        auto* statusVec =
            static_cast<std::vector<std::pair<std::string, std::string>>*>(
                baton);
        std::string statusStr;
        switch (status->node_status) {
            case svn_wc_status_added:
                statusStr = "Added";
                break;
            case svn_wc_status_deleted:
                statusStr = "Deleted";
                break;
            case svn_wc_status_modified:
                statusStr = "Modified";
                break;
            case svn_wc_status_replaced:
                statusStr = "Replaced";
                break;
            case svn_wc_status_unversioned:
                statusStr = "Unversioned";
                break;
            default:
                statusStr = "Unknown";
        }
        statusVec->emplace_back(path, statusStr);
        return SVN_NO_ERROR;
    };

    svn_opt_revision_t revision = {svn_opt_revision_working};
    svn_error_t* err = svn_client_status6(
        nullptr, ctx, repoPath.c_str(), &revision, svn_depth_infinity, TRUE,
        TRUE, TRUE, TRUE, TRUE, TRUE, nullptr, status_receiver, &status, pool);
    if (err) {
        printError(err);
    }

    return status;
}

bool SvnManager::Impl::revertCommit(const std::string& commitId) {
    svn_revnum_t revision = std::stol(commitId);
    svn_opt_revision_t rev = {svn_opt_revision_number, {revision}};
    svn_error_t* err =
        svn_client_merge_peg5(repoPath.c_str(), nullptr, &rev, &rev,
                              repoPath.c_str(), svn_depth_infinity, TRUE, FALSE,
                              FALSE, FALSE, FALSE, nullptr, ctx, pool);
    if (err) {
        printError(err);
        return false;
    }
    return true;
}

bool SvnManager::Impl::createTag(const std::string& tagName,
                                 const std::string& message) {
    std::string tagUrl = repoPath + "/tags/" + tagName;
    svn_opt_revision_t revision = {svn_opt_revision_head};

    svn_error_t* err = svn_client_copy6(
        nullptr, repoPath.c_str(), &revision, tagUrl.c_str(), FALSE, TRUE,
        FALSE, nullptr, nullptr, nullptr, message.c_str(), ctx, pool);
    if (err) {
        printError(err);
        return false;
    }
    return true;
}

}  // namespace lithium
