#include "loader.hpp"

#include <atomic>
#include <condition_variable>
#include <fstream>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>

#include "atom/log/loguru.hpp"
#include "atom/type/json-schema.hpp"
#include "atom/type/json.hpp"

using json = nlohmann::json;

namespace lithium {

std::unordered_map<fs::path, nlohmann::json> TaskLoader::cache_;
std::shared_mutex TaskLoader::cache_mutex_;

namespace {
std::vector<std::thread> threadPool;
std::queue<std::function<void()>> tasks;
std::mutex queue_mutex;
std::condition_variable condition;
bool stop = false;

void worker() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            condition.wait(lock, [] { return stop || !tasks.empty(); });
            if (stop && tasks.empty())
                return;
            task = std::move(tasks.front());
            tasks.pop();
        }
        task();
    }
}
}  // namespace

void TaskLoader::initializeThreadPool() {
    static std::once_flag flag;
    std::call_once(flag, []() {
        unsigned int threadCount = std::thread::hardware_concurrency();
        if (threadCount == 0)
            threadCount = 4;
        for (unsigned int i = 0; i < threadCount; ++i)
            threadPool.emplace_back(worker);
        LOG_F(INFO, "Thread pool initialized with {} threads", threadCount);
    });
}

void TaskLoader::enqueueTask(std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        tasks.emplace(std::move(task));
    }
    condition.notify_one();
    LOG_F(INFO, "Task enqueued");
}

auto TaskLoader::createShared() -> std::shared_ptr<TaskLoader> {
    initializeThreadPool();
    return std::make_shared<TaskLoader>();
}

auto TaskLoader::readJsonFile(const fs::path& filePath) -> std::optional<json> {
    {
        std::shared_lock<std::shared_mutex> lock(cache_mutex_);
        auto it = cache_.find(filePath);
        if (it != cache_.end()) {
            LOG_F(INFO, "Cache hit for file: {}", filePath.string());
            return it->second;
        }
    }

    try {
        if (!fs::exists(filePath) || !fs::is_regular_file(filePath)) {
            LOG_F(ERROR, "File not found: {}", filePath.string());
            return std::nullopt;
        }

        std::ifstream inputFile(filePath);
        json jsonData = json::parse(inputFile, nullptr, false);

        if (jsonData.is_discarded()) {
            LOG_F(ERROR, "Parse error in file: {}", filePath.string());
            return std::nullopt;
        }

        {
            std::unique_lock<std::shared_mutex> lock(cache_mutex_);
            cache_[filePath] = jsonData;
        }

        LOG_F(INFO, "File read and cached: {}", filePath.string());
        return jsonData;
    } catch (const json::exception& e) {
        LOG_F(ERROR, "JSON exception in file {}: {}", filePath.string(),
              e.what());
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Standard exception in file {}: {}", filePath.string(),
              e.what());
    }
    return std::nullopt;
}

auto TaskLoader::writeJsonFile(const fs::path& filePath,
                               const json& jsonData) -> bool {
    try {
        std::ofstream outputFile(filePath);
        outputFile << jsonData.dump(4);
        {
            std::unique_lock<std::shared_mutex> lock(cache_mutex_);
            cache_[filePath] = jsonData;
        }
        LOG_F(INFO, "File written and cached: {}", filePath.string());
        return true;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Failed to write JSON to {}: {}", filePath.string(),
              e.what());
        return false;
    }
}

void TaskLoader::asyncReadJsonFile(
    const fs::path& filePath,
    std::function<void(std::optional<json>)> callback) {
    enqueueTask([filePath, callback]() {
        auto result = readJsonFile(filePath);
        callback(result);
        LOG_F(INFO, "Async read completed for file: {}", filePath.string());
    });
}

void TaskLoader::asyncWriteJsonFile(const fs::path& filePath,
                                    const json& jsonData,
                                    std::function<void(bool)> callback) {
    enqueueTask([filePath, jsonData, callback]() {
        bool success = writeJsonFile(filePath, jsonData);
        callback(success);
        LOG_F(INFO, "Async write completed for file: {}", filePath.string());
    });
}

void TaskLoader::mergeJsonObjects(json& base, const json& toMerge) {
    for (const auto& [key, value] : toMerge.items()) {
        base[key] = value;
    }
    LOG_F(INFO, "JSON objects merged (shallow)");
}

void TaskLoader::deepMergeJsonObjects(json& base, const json& toMerge) {
    for (const auto& [key, value] : toMerge.items()) {
        if (base.contains(key) && base[key].is_object() && value.is_object()) {
            deepMergeJsonObjects(base[key], value);
        } else {
            base[key] = value;
        }
    }
    LOG_F(INFO, "JSON objects merged (deep)");
}

void TaskLoader::batchAsyncProcess(
    const std::vector<fs::path>& filePaths,
    const std::function<void(const std::optional<json>&)>& process,
    const std::function<void()>& onComplete) {
    if (filePaths.empty()) {
        onComplete();
        return;
    }

    std::atomic<int> filesProcessed = 0;
    int totalFiles = static_cast<int>(filePaths.size());

    for (const auto& path : filePaths) {
        asyncReadJsonFile(path,
                          [&filesProcessed, totalFiles, &process,
                           &onComplete](const std::optional<json>& jsonData) {
                              process(jsonData);
                              if (++filesProcessed == totalFiles) {
                                  onComplete();
                                  LOG_F(INFO, "Batch async process completed");
                              }
                          });
    }
}

void TaskLoader::asyncDeleteJsonFile(const fs::path& filePath,
                                     std::function<void(bool)> callback) {
    enqueueTask([filePath, callback]() {
        bool success = fs::remove(filePath);
        if (success) {
            std::unique_lock<std::shared_mutex> lock(cache_mutex_);
            cache_.erase(filePath);
        }
        callback(success);
        LOG_F(INFO, "Async delete completed for file: {}", filePath.string());
    });
}

void TaskLoader::asyncQueryJsonValue(
    const fs::path& filePath, const std::string& key,
    std::function<void(std::optional<json>)> callback) {
    enqueueTask([filePath, key, callback]() {
        auto jsonOpt = readJsonFile(filePath);
        if (jsonOpt && jsonOpt->contains(key)) {
            callback((*jsonOpt)[key]);
        } else {
            callback(std::nullopt);
        }
        LOG_F(INFO, "Async query completed for file: {}", filePath.string());
    });
}

void TaskLoader::batchProcessDirectory(
    const fs::path& directoryPath,
    const std::function<void(const std::optional<json>&)>& process,
    const std::function<void()>& onComplete) {
    if (!fs::exists(directoryPath) || !fs::is_directory(directoryPath)) {
        LOG_F(ERROR, "Invalid directory path: {}", directoryPath.string());
        onComplete();
        return;
    }

    std::vector<fs::path> filePaths;
    for (const auto& entry : fs::directory_iterator(directoryPath)) {
        if (entry.path().extension() == ".json") {
            filePaths.emplace_back(entry.path());
        }
    }

    batchAsyncProcess(filePaths, process, onComplete);
}

auto TaskLoader::validateJson(const json& jsonData,
                              const json& schema) -> bool {
    try {
        atom::type::JsonValidator validator;
        validator.setRootSchema(schema);
        validator.validate(jsonData);
        LOG_F(INFO, "JSON validation succeeded");
        return true;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "JSON validation failed: {}", e.what());
        return false;
    }
}

}  // namespace lithium
