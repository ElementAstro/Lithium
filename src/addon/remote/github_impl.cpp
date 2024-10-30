// GitHubAPIImpl.cpp
#include "github_impl.hpp"

#include <utility>

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"
#include "atom/web/curl.hpp"

namespace lithium::addon::remote {

// 自定义异常类
class GitHubAPIException : public atom::error::Exception {
public:
    using atom::error::Exception::Exception;
};

#define THROW_GITHUB_EXCEPTION(...)                   \
    throw lithium::addon::remote::GitHubAPIException( \
        ATOM_FILE_NAME, ATOM_FILE_LINE, ATOM_FUNC_NAME, __VA_ARGS__)

#define THROW_NESTED_GITHUB_EXCEPTION(...)                     \
    lithium::addon::remote::GitHubAPIException::rethrowNested( \
        ATOM_FILE_NAME, ATOM_FILE_LINE, ATOM_FUNC_NAME, __VA_ARGS__)

GitHubAPIImpl::GitHubAPIImpl(std::string token)
    : curl_(), authToken_(std::move(token)) {
    LOG_F(INFO, "Initializing GitHubAPIImpl with token");
}

GitHubAPIImpl::~GitHubAPIImpl() { LOG_F(INFO, "Destroying GitHubAPIImpl"); }

auto GitHubAPIImpl::httpGet(const std::string& url) -> std::string {
    LOG_F(INFO, "Performing HTTP GET request to URL: {}", url);
    atom::web::CurlWrapper curl;
    std::string readBuffer;

    curl.setUrl(url)
        .setRequestMethod("GET")
        .addHeader("Authorization", "token " + authToken_)
        .addHeader("User-Agent", "YourAppName")
        .onError([](CURLcode code) {
            LOG_F(ERROR, "CURL error: {}", curl_easy_strerror(code));
            THROW_GITHUB_EXCEPTION("CURL perform failed: " +
                                   std::string(curl_easy_strerror(code)));
        })
        .onResponse(
            [&readBuffer](const std::string& data) { readBuffer = data; });

    return curl.perform();
}

auto GitHubAPIImpl::httpPost(const std::string& url,
                             const std::string& data) -> std::string {
    LOG_F(INFO, "Performing HTTP POST request to URL: {}", url);
    atom::web::CurlWrapper curl;
    std::string readBuffer;

    curl.setUrl(url)
        .setRequestMethod("POST")
        .addHeader("Authorization", "token " + authToken_)
        .addHeader("User-Agent", "YourAppName")
        .addHeader("Content-Type", "application/json")
        .setRequestBody(data)
        .onError([](CURLcode code) {
            LOG_F(ERROR, "CURL error: {}", curl_easy_strerror(code));
            THROW_GITHUB_EXCEPTION("CURL perform failed: " +
                                   std::string(curl_easy_strerror(code)));
        })
        .onResponse(
            [&readBuffer](const std::string& data) { readBuffer = data; });

    return curl.perform();
}

auto GitHubAPIImpl::httpDelete(const std::string& url) -> std::string {
    LOG_F(INFO, "Performing HTTP DELETE request to URL: {}", url);
    atom::web::CurlWrapper curl;
    std::string readBuffer;

    curl.setUrl(url)
        .setRequestMethod("DELETE")
        .addHeader("Authorization", "token " + authToken_)
        .addHeader("User-Agent", "YourAppName")
        .onError([](CURLcode code) {
            LOG_F(ERROR, "CURL error: {}", curl_easy_strerror(code));
            THROW_GITHUB_EXCEPTION("CURL perform failed: " +
                                   std::string(curl_easy_strerror(code)));
        })
        .onResponse(
            [&readBuffer](const std::string& data) { readBuffer = data; });

    return curl.perform();
}

auto GitHubAPIImpl::httpPatch(const std::string& url,
                              const std::string& data) -> std::string {
    LOG_F(INFO, "Performing HTTP PATCH request to URL: {}", url);
    atom::web::CurlWrapper curl;
    std::string readBuffer;

    curl.setUrl(url)
        .setRequestMethod("PATCH")
        .addHeader("Authorization", "token " + authToken_)
        .addHeader("User-Agent", "YourAppName")
        .addHeader("Content-Type", "application/json")
        .setRequestBody(data)
        .onError([](CURLcode code) {
            LOG_F(ERROR, "CURL error: {}", curl_easy_strerror(code));
            THROW_GITHUB_EXCEPTION("CURL perform failed: " +
                                   std::string(curl_easy_strerror(code)));
        })
        .onResponse(
            [&readBuffer](const std::string& data) { readBuffer = data; });

    return curl.perform();
}

auto GitHubAPIImpl::getGitHubStatus() -> std::string {
    const std::string URL = "https://www.githubstatus.com/api/v2/status.json";
    LOG_F(INFO, "Getting GitHub status from URL: {}", URL);
    std::string response = httpGet(URL);
    auto jsonData = json::parse(response);
    return jsonData["status"]["description"].get<std::string>();
}

auto GitHubAPIImpl::getRepoInfo(const std::string& owner,
                                const std::string& repo) -> json {
    const std::string URL =
        "https://api.github.com/repos/" + owner + "/" + repo;
    LOG_F(INFO, "Getting repository info for {}/{}", owner.c_str(),
          repo.c_str());
    std::string response = httpGet(URL);
    return json::parse(response);
}

auto GitHubAPIImpl::getLatestRelease(const std::string& owner,
                                     const std::string& repo) -> json {
    const std::string URL = "https://api.github.com/repos/" + owner + "/" +
                            repo + "/releases/latest";
    LOG_F(INFO, "Getting latest release for {}/{}", owner.c_str(),
          repo.c_str());
    std::string response = httpGet(URL);
    return json::parse(response);
}

auto GitHubAPIImpl::listBranches(const std::string& owner,
                                 const std::string& repo) -> json {
    const std::string URL =
        "https://api.github.com/repos/" + owner + "/" + repo + "/branches";
    LOG_F(INFO, "Listing branches for {}/{}", owner.c_str(), repo.c_str());
    std::string response = httpGet(URL);
    return json::parse(response);
}

auto GitHubAPIImpl::listContributors(const std::string& owner,
                                     const std::string& repo) -> json {
    const std::string URL =
        "https://api.github.com/repos/" + owner + "/" + repo + "/contributors";
    LOG_F(INFO, "Listing contributors for {}/{}", owner.c_str(), repo.c_str());
    std::string response = httpGet(URL);
    return json::parse(response);
}

auto GitHubAPIImpl::listTags(const std::string& owner,
                             const std::string& repo) -> json {
    const std::string URL =
        "https://api.github.com/repos/" + owner + "/" + repo + "/tags";
    LOG_F(INFO, "Listing tags for {}/{}", owner.c_str(), repo.c_str());
    std::string response = httpGet(URL);
    return json::parse(response);
}

auto GitHubAPIImpl::getBranchCommits(const std::string& owner,
                                     const std::string& repo,
                                     const std::string& branch) -> json {
    const std::string URL = "https://api.github.com/repos/" + owner + "/" +
                            repo + "/commits?sha=" + branch;
    LOG_F(INFO, "Getting commits for branch {} of {}/{}", branch.c_str(),
          owner.c_str(), repo.c_str());
    std::string response = httpGet(URL);
    return json::parse(response);
}

// 新增功能实现
auto GitHubAPIImpl::createRepo(const std::string& name,
                               const std::string& description,
                               bool isPrivate) -> json {
    const std::string URL = "https://api.github.com/user/repos";
    json payload = {
        {"name", name}, {"description", description}, {"private", isPrivate}};
    LOG_F(INFO, "Creating repository with name: {}", name.c_str());
    std::string response = httpPost(URL, payload.dump());
    return json::parse(response);
}

auto GitHubAPIImpl::deleteRepo(const std::string& owner,
                               const std::string& repo) -> json {
    const std::string URL =
        "https://api.github.com/repos/" + owner + "/" + repo;
    LOG_F(INFO, "Deleting repository {}/{}", owner.c_str(), repo.c_str());
    std::string response = httpDelete(URL);
    return json::parse(response);
}

auto GitHubAPIImpl::updateRepo(const std::string& owner,
                               const std::string& repo,
                               const json& updates) -> json {
    const std::string URL =
        "https://api.github.com/repos/" + owner + "/" + repo;
    LOG_F(INFO, "Updating repository {}/{}", owner.c_str(), repo.c_str());
    std::string response = httpPatch(URL, updates.dump());
    return json::parse(response);
}

}  // namespace lithium::addon::remote