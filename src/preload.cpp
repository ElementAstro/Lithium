#include "preload.hpp"

#include <fstream>
#include <future>
#include <mutex>
#include <thread>

#include "atom/async/pool.hpp"
#include "atom/error/exception.hpp"
#include "atom/function/global_ptr.hpp"
#include "atom/io/io.hpp"
#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"
#include "atom/utils/aes.hpp"
#include "atom/utils/string.hpp"
#include "atom/web/curl.hpp"

#include "script/pycaller.hpp"

#include "utils/constant.hpp"
#include "utils/resource.hpp"

using json = nlohmann::json;

namespace lithium {
class Preloader::Impl {
public:
    Impl()
        : download_progress_(0.0),
          resource_server_(resource::LITHIUM_RESOURCE_SERVER) {
        std::shared_ptr<lithium::PythonWrapper> pythonWrapperPtr;
        GET_OR_CREATE_PTR(pythonWrapperPtr, lithium::PythonWrapper,
                          Constants::PYTHON_WRAPPER);
    }

    auto checkResources() -> bool {
        LOG_F(INFO, "Checking resources...");
        std::lock_guard<std::mutex> lock(mutex_);
        bool allResourcesValid = true;

        for (auto &[key, value] : resource::LITHIUM_RESOURCES) {
            if (!atom::io::isFileExists(key.data())) {
                LOG_F(ERROR, "Resource file '{}' is missing.", key);
                allResourcesValid = false;
                continue;
            }
            auto sha256_val = atom::utils::calculateSha256(key);
            if (sha256_val.empty()) {
                LOG_F(ERROR, "Failed to calculate SHA256 value of '{}'.", key);
                allResourcesValid = false;
                continue;
            }
            auto expected_sha256 = value.first;
            if (sha256_val != expected_sha256) {
                LOG_F(ERROR, "SHA256 check failed for '{}'.", key);
                allResourcesValid = false;
            } else {
                LOG_F(INFO, "Resource '{}' is valid.", key);
                value.second = true;
            }
        }

        if (allResourcesValid) {
            LOG_F(INFO, "All resource files are valid.");
        } else {
            LOG_F(WARNING, "Some resource files are missing or invalid.");
        }

        return allResourcesValid;
    }

    void downloadResources() {
        LOG_F(INFO, "Starting download of missing resources...");

        std::lock_guard<std::mutex> lock(mutex_);
        // 创建线程池
        atom::async::ThreadPool pool(std::thread::hardware_concurrency());

        // 创建任务列表
        std::vector<std::future<bool>> tasks;
        size_t totalTasks = 0;
        size_t completedTasks = 0;

        for (auto &[key, value] : resource::LITHIUM_RESOURCES) {
            if (value.second) {
                continue;  // 跳过已存在且有效的资源
            }

            const auto url =
                atom::utils::joinStrings({resource_server_, key}, "/");
            totalTasks++;

            // 添加下载任务到线程池
            tasks.emplace_back(pool.enqueue([this, url, key, &completedTasks,
                                             &totalTasks]() -> bool {
                try {
                    atom::web::CurlWrapper curl;
                    std::string response;
                    curl.setUrl(url)
                        .setRequestMethod("GET")
                        .onResponse([&response](const std::string &data) {
                            response = data;
                        })
                        .onError([](CURLcode code) {
                            LOG_F(ERROR, "Curl error: %d", code);
                        })
                        .perform();

                    if (response.empty()) {
                        LOG_F(ERROR, "Failed to download resource: {}", url);
                        return false;
                    }

                    // 将下载的数据写入文件
                    std::ofstream outfile(std::string(key), std::ios::binary);
                    if (!outfile) {
                        LOG_F(ERROR, "Failed to open file '{}' for writing.",
                              key);
                        return false;
                    }
                    outfile.write(
                        response.c_str(),
                        static_cast<std::streamsize>(response.size()));
                    outfile.close();

                    // 验证下载的文件
                    auto sha256_val = atom::utils::calculateSha256(key);
                    if (sha256_val.empty()) {
                        LOG_F(ERROR,
                              "Failed to calculate SHA256 for downloaded file "
                              "'{}'.",
                              key);
                        return false;
                    }

                    auto expected_sha256 =
                        resource::LITHIUM_RESOURCES[key].first;
                    if (sha256_val != expected_sha256) {
                        LOG_F(ERROR, "SHA256 mismatch for '{}'.", key);
                        return false;
                    }

                    LOG_F(INFO,
                          "Resource '{}' downloaded and verified successfully.",
                          key);
                    {
                        std::lock_guard<std::mutex> progressLock(
                            progress_mutex_);
                        completedTasks++;
                        download_progress_ =
                            static_cast<double>(completedTasks) / totalTasks *
                            100.0;
                    }
                    return true;
                } catch (const std::exception &e) {
                    LOG_F(ERROR, "Exception while downloading '{}': {}", url,
                          e.what());
                    return false;
                }
            }));
        }

        if (totalTasks == 0) {
            LOG_F(INFO, "No resources need to be downloaded.");
            return;
        }

        // 等待所有任务完成
        for (auto &&task : tasks) {
            task.wait();
        }

        bool allDownloadsSuccessful = true;
        for (auto &&task : tasks) {
            if (!task.get()) {
                allDownloadsSuccessful = false;
            }
        }

        if (allDownloadsSuccessful) {
            LOG_F(INFO, "All resources downloaded and verified successfully.");
        } else {
            LOG_F(ERROR, "Some resources failed to download or verify.");
        }
    }

    auto getDownloadProgress() const -> double {
        std::lock_guard<std::mutex> lock(progress_mutex_);
        return download_progress_;
    }

    void setResourceServer(const std::string &server) {
        std::lock_guard<std::mutex> lock(mutex_);
        resource_server_ = server;
        LOG_F(INFO, "Resource server set to '{}'.", server);
    }

private:
    std::unordered_map<std::string, std::pair<std::string, bool>> scripts_;
    mutable std::mutex mutex_;

    // 新增成员用于下载进度
    double download_progress_;
    mutable std::mutex progress_mutex_;

    std::string resource_server_;
};

// Preloader 实现

Preloader::Preloader() : pImpl(std::make_unique<Impl>()) {}

Preloader::~Preloader() = default;

Preloader::Preloader(Preloader &&) noexcept = default;

auto Preloader::operator=(Preloader &&) noexcept -> Preloader & = default;

auto Preloader::checkResources() -> bool { return pImpl->checkResources(); }

void Preloader::downloadResources() { pImpl->downloadResources(); }

auto Preloader::getDownloadProgress() const -> double {
    return pImpl->getDownloadProgress();
}

void Preloader::setResourceServer(const std::string &server) {
    pImpl->setResourceServer(server);
}

}  // namespace lithium
