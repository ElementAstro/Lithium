#include "preload.hpp"

#include <fstream>

#include "atom/async/pool.hpp"
#include "atom/io/io.hpp"
#include "atom/log/loguru.hpp"
#include "atom/utils/aes.hpp"
#include "atom/utils/string.hpp"
#include "atom/web/httpclient.hpp"

#include "utils/resource.hpp"

bool checkResources() {
    for (auto &[key, value] : resource::LITHIUM_RESOURCES) {
        if (!Atom::IO::isFileExists(key.data())) {
            LOG_F(ERROR, "Resource file '{}' is missing.", key);
            return false;
        }
        auto sha256_val = atom::utils::calculateSha256(key);
        if (!sha256_val.empty()) {
            LOG_F(ERROR, "Failed to calculate SHA256 value of '{}'.", key);
            value.second = true;
            continue;
        }
        auto expected_sha256 = value.first;
        if (sha256_val != expected_sha256) {
            LOG_F(ERROR, "SHA256 check failed for '{}'", key);
            return false;
        }
        value.second = true;
    }

    DLOG_F(INFO, "All resource files are found.");
    return true;
}

void downloadResources() {
    DLOG_F(INFO, "Downloading missing resources...");

    // 创建线程池
    Atom::Async::ThreadPool pool(std::thread::hardware_concurrency());

    // 创建任务列表
    std::vector<std::future<bool>> tasks;

    for (auto &[key, value] : resource::LITHIUM_RESOURCES) {
        // 发送 HTTP GET 请求下载文件
        const auto url = atom::utils::joinStrings(
            {resource::LITHIUM_RESOURCE_SERVER, key}, "/");

        // 添加下载任务到线程池
        tasks.emplace_back(pool.enqueue([url] {
            try {
                auto client = Atom::Web::HttpClient(
                    resource::LITHIUM_RESOURCE_SERVER, 443, true);
                json res_body;
                std::string err;
                auto res = client.sendGetRequest(url, {}, res_body, err);

                if (!res) {
                    LOG_F(ERROR, "Failed to download resource: {}", url);
                    return false;
                }

                // 将下载的数据写入文件
                std::ofstream outfile(
                    std::string(atom::utils::splitString(url, '/').back()));
                outfile.write(res_body.dump().c_str(), res_body.dump().size());

                DLOG_F(INFO, "Resource file '{}' downloaded.", url);
                return true;
            } catch (const std::exception &e) {
                LOG_F(ERROR, "Error occurred when downloading resource '{}: {}",
                      url, e.what());
                return false;
            }
        }));
    }
    for (auto &&task : tasks) {
        task.wait();
    }
    for (auto &&task : tasks) {
        if (!task.get()) {
            LOG_F(ERROR, "Failed to download some resources.");
        }
    }
    DLOG_F(INFO, "Downloading finished.");
}