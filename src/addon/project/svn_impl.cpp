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

}  // namespace lithium
