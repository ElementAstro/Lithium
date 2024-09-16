#include "token.hpp"

#include "atom/log/loguru.hpp"

namespace lithium {
void DefaultParallelStrategy::process(std::vector<std::future<void>>& futures,
                                      std::function<void()> task) {
    LOG_F(INFO, "DefaultParallelStrategy: Adding task to futures");
    futures.push_back(std::async(std::launch::async, task));
}

void ThreadPoolParallelStrategy::process(
    std::vector<std::future<void>>& futures, std::function<void()> task) {
    LOG_F(INFO, "ThreadPoolParallelStrategy: Adding task to futures");
    futures.push_back(pool.lock()->enqueue(task));
}

StringSplitter::StringSplitter()
    : parallelStrategy(std::make_shared<DefaultParallelStrategy>()),
      segmentsProcessed(0),
      validationFailures(0) {
    LOG_F(INFO, "StringSplitter initialized with DefaultParallelStrategy");
}

void StringSplitter::setParallelStrategy(
    std::shared_ptr<ParallelStrategy> strategy) {
    LOG_F(INFO, "Setting new parallel strategy");
    parallelStrategy = std::move(strategy);
}

void StringSplitter::registerValidator(Validator validator, std::string group) {
    std::lock_guard lock(mutex_);
    LOG_F(INFO, "Registering validator for group: {}", group);
    validators[group].push_back(std::move(validator));
}

void StringSplitter::registerErrorHandler(ErrorHandler handler) {
    std::lock_guard lock(mutex_);
    LOG_F(INFO, "Registering error handler");
    errorHandlers.push_back(std::move(handler));
}

auto StringSplitter::splitAndValidate(const std::string& str,
                                      const std::set<char>& delimiters)
    -> std::vector<std::string> {
    LOG_F(INFO, "Starting split and validate");
    std::vector<std::string> result;
    std::vector<std::future<void>> futures;

    size_t index = 0;
    auto it = str.begin();
    auto end = str.end();

    while (it != end) {
        auto nextIt =
            std::find_if(it, end, [&](char c) { return delimiters.count(c); });
        std::string_view segment(&*it, std::distance(it, nextIt));

        LOG_F(INFO, "Processing segment: {}", segment);

        auto task = [this, segment, index, &result]() {
            if (validateSegment(segment, index)) {
                std::lock_guard lock(resultMutex_);
                result.emplace_back(segment);
                segmentsProcessed++;
                LOG_F(INFO, "Segment {} validated and added to result", index);
            } else {
                validationFailures++;
                LOG_F(WARNING, "Segment {} validation failed", index);
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

auto StringSplitter::getSegmentsProcessed() const -> size_t {
    LOG_F(INFO, "Getting segments processed count");
    return segmentsProcessed.load();
}

auto StringSplitter::getValidationFailures() const -> size_t {
    LOG_F(INFO, "Getting validation failures count");
    return validationFailures.load();
}

auto StringSplitter::validateSegment(const std::string_view& segment,
                                     size_t index) -> bool {
    LOG_F(INFO, "Validating segment {}: {}", index, segment);
    {
        std::lock_guard lock(mutex_);
        auto cacheIt = validationCache.find(segment);
        if (cacheIt != validationCache.end()) {
            LOG_F(INFO, "Segment {} found in cache: {}", index,
                  cacheIt->second);
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
                    LOG_F(WARNING, "Segment {} failed validation in group {}",
                          index, group);
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
    LOG_F(ERROR, "Handling validation error for segment {}", index);
    std::lock_guard lock(mutex_);
    for (const auto& handler : errorHandlers) {
        handler(segment, index);
    }
    LOG_F(ERROR, "Validation failed at segment {}", index);
}
}  // namespace lithium