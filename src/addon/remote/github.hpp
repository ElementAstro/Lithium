// GitHubAPI.h
#ifndef LITHIUM_ADDON_REMOTE_GITHUBAPI_HPP
#define LITHIUM_ADDON_REMOTE_GITHUBAPI_HPP

#include <memory>
#include <string>

#include "atom/type/json_fwd.hpp"
using json = nlohmann::json;

namespace lithium::addon::remote {

class GitHubAPIImpl;

class GitHubAPI {
public:
    GitHubAPI(const std::string& token = "");
    ~GitHubAPI();

    std::string getGitHubStatus();
    json getRepoInfo(const std::string& owner, const std::string& repo);
    json getLatestRelease(const std::string& owner, const std::string& repo);
    json listBranches(const std::string& owner, const std::string& repo);
    json listContributors(const std::string& owner, const std::string& repo);
    json listTags(const std::string& owner, const std::string& repo);
    json getBranchCommits(const std::string& owner, const std::string& repo,
                          const std::string& branch);

    // 新增功能
    json createRepo(const std::string& name, const std::string& description,
                    bool isPrivate);
    json deleteRepo(const std::string& owner, const std::string& repo);
    json updateRepo(const std::string& owner, const std::string& repo,
                    const json& updates);

private:
    std::unique_ptr<GitHubAPIImpl> impl;
};

}  // namespace lithium::addon::remote

#endif  // LITHIUM_ADDON_REMOTE_GITHUBAPI_HPP
