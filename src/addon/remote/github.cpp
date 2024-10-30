
// GitHubAPI.cpp
#include "github.hpp"
#include "github_impl.hpp"

#include "atom/type/json.hpp"

namespace lithium::addon::remote {

GitHubAPI::GitHubAPI(const std::string& token)
    : impl(std::make_unique<GitHubAPIImpl>(token)) {}

GitHubAPI::~GitHubAPI() = default;

std::string GitHubAPI::getGitHubStatus() { return impl->getGitHubStatus(); }

json GitHubAPI::getRepoInfo(const std::string& owner, const std::string& repo) {
    return impl->getRepoInfo(owner, repo);
}

json GitHubAPI::getLatestRelease(const std::string& owner,
                                 const std::string& repo) {
    return impl->getLatestRelease(owner, repo);
}

json GitHubAPI::listBranches(const std::string& owner,
                             const std::string& repo) {
    return impl->listBranches(owner, repo);
}

json GitHubAPI::listContributors(const std::string& owner,
                                 const std::string& repo) {
    return impl->listContributors(owner, repo);
}

json GitHubAPI::listTags(const std::string& owner, const std::string& repo) {
    return impl->listTags(owner, repo);
}

json GitHubAPI::getBranchCommits(const std::string& owner,
                                 const std::string& repo,
                                 const std::string& branch) {
    return impl->getBranchCommits(owner, repo, branch);
}

// 新增功能实现
json GitHubAPI::createRepo(const std::string& name,
                           const std::string& description, bool isPrivate) {
    return impl->createRepo(name, description, isPrivate);
}

json GitHubAPI::deleteRepo(const std::string& owner, const std::string& repo) {
    return impl->deleteRepo(owner, repo);
}

json GitHubAPI::updateRepo(const std::string& owner, const std::string& repo,
                           const json& updates) {
    return impl->updateRepo(owner, repo, updates);
}

}  // namespace lithium::addon::remote