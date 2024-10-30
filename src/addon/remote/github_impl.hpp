// GitHubAPIImpl.hpp
#ifndef GITHUB_API_IMPL_HPP
#define GITHUB_API_IMPL_HPP

#include <curl/curl.h>
#include <mutex>
#include <string>

#include "atom/type/json_fwd.hpp"
using json = nlohmann::json;

namespace lithium::addon::remote {

class GitHubAPIImpl {
public:
    explicit GitHubAPIImpl(std::string token);
    ~GitHubAPIImpl();

    GitHubAPIImpl(const GitHubAPIImpl& other) = delete;
    auto operator=(const GitHubAPIImpl& other) -> GitHubAPIImpl& = delete;
    GitHubAPIImpl(GitHubAPIImpl&& other) noexcept = delete;
    auto operator=(GitHubAPIImpl&& other) noexcept -> GitHubAPIImpl& = delete;

    auto getGitHubStatus() -> std::string;
    auto getRepoInfo(const std::string& owner, const std::string& repo) -> json;
    auto getLatestRelease(const std::string& owner,
                          const std::string& repo) -> json;
    auto listBranches(const std::string& owner,
                      const std::string& repo) -> json;
    auto listContributors(const std::string& owner,
                          const std::string& repo) -> json;
    auto listTags(const std::string& owner, const std::string& repo) -> json;
    auto getBranchCommits(const std::string& owner, const std::string& repo,
                          const std::string& branch) -> json;

    // 新增功能
    auto createRepo(const std::string& name, const std::string& description,
                    bool isPrivate) -> json;
    auto deleteRepo(const std::string& owner, const std::string& repo) -> json;
    auto updateRepo(const std::string& owner, const std::string& repo,
                    const json& updates) -> json;

private:
    void initCurl();
    void cleanupCurl();
    auto httpGet(const std::string& url) -> std::string;
    auto httpPost(const std::string& url,
                  const std::string& data) -> std::string;
    auto httpDelete(const std::string& url) -> std::string;
    auto httpPatch(const std::string& url,
                   const std::string& data) -> std::string;

    static auto writeCallback(void* contents, size_t size, size_t nmemb,
                              void* userp) -> size_t;

    CURL* curl_;
    std::mutex curlMutex_;
    std::string authToken_;
};

}  // namespace lithium::addon::remote

#endif  // GITHUB_API_IMPL_HPP