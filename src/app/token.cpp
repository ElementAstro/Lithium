#include "token.hpp"

#include "atom/log/loguru.hpp"

namespace lithium {
void DefaultParallelStrategy::process(std::vector<std::future<void>>& futures,
                                      std::function<void()> task) {
    futures.push_back(std::async(std::launch::async, task));
}

void ThreadPoolParallelStrategy::process(
    std::vector<std::future<void>>& futures, std::function<void()> task) {
    futures.push_back(pool.lock()->enqueue(task));
}

StringSplitter::StringSplitter()
    : parallelStrategy(std::make_shared<DefaultParallelStrategy>()),
      segmentsProcessed(0),
      validationFailures(0) {}

void StringSplitter::setParallelStrategy(
    std::shared_ptr<ParallelStrategy> strategy) {
    parallelStrategy = std::move(strategy);
}

// 设置日志功能
void StringSplitter::registerValidator(Validator validator, std::string group) {
    std::lock_guard lock(mutex_);
    validators[group].push_back(std::move(validator));
}

void StringSplitter::registerErrorHandler(ErrorHandler handler) {
    std::lock_guard lock(mutex_);
    errorHandlers.push_back(std::move(handler));
}

auto StringSplitter::splitAndValidate(const std::string& str,
                                      const std::set<char>& delimiters)
    -> std::vector<std::string> {
    std::vector<std::string> result;
    std::vector<std::future<void>> futures;

    size_t index = 0;
    auto it = str.begin();
    auto end = str.end();

    while (it != end) {
        auto nextIt =
            std::find_if(it, end, [&](char c) { return delimiters.count(c); });
        std::string_view segment(&*it, std::distance(it, nextIt));

        auto task = [this, segment, index, &result]() {
            if (validateSegment(segment, index)) {
                std::lock_guard lock(resultMutex_);
                result.emplace_back(segment);
                segmentsProcessed++;
            } else {
                validationFailures++;
            }
        };

        parallelStrategy->process(futures, task);

        ++index;
        if (nextIt != end) {
            ++nextIt;  // 跳过分隔符
        }
        it = nextIt;
    }

    for (auto& future : futures) {
        future.get();
    }

    LOG_F(INFO, "Segments processed: {}", segmentsProcessed);
    LOG_F(INFO, "Validation failures: {}", validationFailures);

    return result;
}

// 获取统计信息
auto StringSplitter::getSegmentsProcessed() const -> size_t {
    return segmentsProcessed.load();
}

auto StringSplitter::getValidationFailures() const -> size_t {
    return validationFailures.load();
}

auto StringSplitter::validateSegment(const std::string_view& segment,
                                     size_t index) -> bool {
    {
        std::lock_guard lock(mutex_);
        auto cacheIt = validationCache.find(segment);
        if (cacheIt != validationCache.end()) {
            return cacheIt->second;
        }
    }

    bool isValid = true;
    {
        std::lock_guard lock(mutex_);
        for (const auto& [group, groupValidators] : validators) {
            for (const auto& validator : groupValidators) {
                if (!validator(segment, index)) {
                    isValid = false;
                    break;
                }
            }
            if (!isValid) {
                break;
            }
        }
        validationCache[segment] = isValid;
    }

    if (!isValid) {
        handleValidationError(segment, index);
    }

    return isValid;
}

void StringSplitter::handleValidationError(const std::string_view& segment,
                                           size_t index) {
    std::lock_guard lock(mutex_);
    for (const auto& handler : errorHandlers) {
        handler(segment, index);
    }
    LOG_F(ERROR, "Validation failed at segment {}", index);
}
}  // namespace lithium
