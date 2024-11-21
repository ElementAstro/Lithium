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
    Impl() : resource_server_(resource::LITHIUM_RESOURCE_SERVER) {
        std::shared_ptr<lithium::PythonManager> pythonManagerPtr;
        GET_OR_CREATE_PTR(pythonManagerPtr, lithium::PythonManager,
                          Constants::PYTHON_MANAGER);
    }

    auto checkResources() -> bool {
        LOG_F(INFO, "Checking resources...");
        std::lock_guard lock(mutex_);
        bool allResourcesValid = true;

        for (auto &[key, value] : resource::LITHIUM_RESOURCES) {
            if (!validateResource(key, value.first)) {
                allResourcesValid = false;
            } else {
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

        std::lock_guard lock(mutex_);
        atom::async::ThreadPool pool(std::thread::hardware_concurrency());
        std::vector<std::future<bool>> tasks;
        size_t totalTasks = 0;
        size_t completedTasks = 0;

        for (auto &[key, value] : resource::LITHIUM_RESOURCES) {
            if (value.second) {
                continue;
            }

            const auto URL =
                atom::utils::joinStrings({resource_server_, key}, "/");
            totalTasks++;

            tasks.emplace_back(pool.enqueue(
                [this, URL, key, &completedTasks, &totalTasks]() -> bool {
                    return downloadAndValidateResource(
                        URL, std::string(key), completedTasks, totalTasks);
                }));
        }

        if (totalTasks == 0) {
            LOG_F(INFO, "No resources need to be downloaded.");
            return;
        }

        for (auto &&task : tasks) {
            task.wait();
        }

        bool allDownloadsSuccessful = std::all_of(
            tasks.begin(), tasks.end(), [](auto &task) { return task.get(); });

        if (allDownloadsSuccessful) {
            LOG_F(INFO, "All resources downloaded and verified successfully.");
        } else {
            LOG_F(ERROR, "Some resources failed to download or verify.");
        }
    }

    auto getDownloadProgress() const -> double {
        std::lock_guard lock(progress_mutex_);
        return download_progress_;
    }

    void setResourceServer(const std::string &server) {
        std::lock_guard lock(mutex_);
        resource_server_ = server;
        LOG_F(INFO, "Resource server set to '{}'.", server);
    }

private:
    static auto validateResource(std::string_view key,
                                 const std::string &expectedSha256) -> bool {
        if (!atom::io::isFileExists(key.data())) {
            LOG_F(ERROR, "Resource file '{}' is missing.", key);
            return false;
        }
        auto sha256Val = atom::utils::calculateSha256(key);
        if (sha256Val.empty()) {
            LOG_F(ERROR, "Failed to calculate SHA256 value of '{}'.", key);
            return false;
        }
        if (sha256Val != expectedSha256) {
            LOG_F(ERROR, "SHA256 check failed for '{}'.", key);
            return false;
        }
        LOG_F(INFO, "Resource '{}' is valid.", key);
        return true;
    }

    bool downloadAndValidateResource(const std::string &URL,
                                     const std::string &key,
                                     size_t &completedTasks,
                                     size_t totalTasks) {
        try {
            atom::web::CurlWrapper curl;
            std::string response;
            curl.setUrl(URL)
                .setRequestMethod("GET")
                .onResponse(
                    [&response](const std::string &data) { response = data; })
                .onError(
                    [](CURLcode code) { LOG_F(ERROR, "Curl error: %d", code); })
                .perform();

            if (response.empty()) {
                LOG_F(ERROR, "Failed to download resource: {}", URL);
                return false;
            }

            std::ofstream outfile(std::string(key), std::ios::binary);
            if (!outfile) {
                LOG_F(ERROR, "Failed to open file '{}' for writing.", key);
                return false;
            }
            outfile.write(response.c_str(),
                          static_cast<std::streamsize>(response.size()));
            outfile.close();

            if (!validateResource(key,
                                  resource::LITHIUM_RESOURCES[key].first)) {
                return false;
            }

            {
                std::lock_guard progressLock(progress_mutex_);
                completedTasks++;
                download_progress_ = static_cast<double>(completedTasks) /
                                     static_cast<double>(totalTasks) * 100.0;
            }
            return true;
        } catch (const std::exception &e) {
            LOG_F(ERROR, "Exception while downloading '{}': {}", URL, e.what());
            return false;
        }
    }

    std::unordered_map<std::string, std::pair<std::string, bool>> scripts_;
    mutable std::mutex mutex_;
    double download_progress_{};
    mutable std::mutex progress_mutex_;
    std::string resource_server_;
};

Preloader::Preloader() : impl_(std::make_unique<Impl>()) {}

Preloader::~Preloader() = default;

auto Preloader::checkResources() -> bool { return impl_->checkResources(); }

void Preloader::downloadResources() { impl_->downloadResources(); }

auto Preloader::getDownloadProgress() const -> double {
    return impl_->getDownloadProgress();
}

void Preloader::setResourceServer(const std::string &server) {
    impl_->setResourceServer(server);
}

}  // namespace lithium
