#ifndef LITHIUM_APP_TOKEN_HPP
#define LITHIUM_APP_TOKEN_HPP

#include <atomic>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "atom/async/pool.hpp"

namespace lithium {
class ParallelStrategy {
public:
    virtual void process(std::vector<std::future<void>>& futures,
                         std::function<void()> task) = 0;
    virtual ~ParallelStrategy() = default;
};

class DefaultParallelStrategy : public ParallelStrategy {
public:
    void process(std::vector<std::future<void>>& futures,
                 std::function<void()> task) override;
};

class ThreadPoolParallelStrategy : public ParallelStrategy {
public:
    void process(std::vector<std::future<void>>& futures,
                 std::function<void()> task) override;

private:
    std::weak_ptr<atom::async::ThreadPool<>> pool;
};

class StringSplitter {
public:
    using Validator = std::function<bool(const std::string_view&, size_t)>;
    using ErrorHandler = std::function<void(const std::string_view&, size_t)>;

    StringSplitter();

    void setParallelStrategy(std::shared_ptr<ParallelStrategy> strategy);

    void registerValidator(Validator validator, std::string group = "default");

    void registerErrorHandler(ErrorHandler handler);

    auto splitAndValidate(const std::string& str,
                          const std::set<char>& delimiters)
        -> std::vector<std::string>;

    auto getSegmentsProcessed() const -> size_t;

    auto getValidationFailures() const -> size_t;

private:
    std::unordered_map<std::string, std::vector<Validator>> validators;
    std::vector<ErrorHandler> errorHandlers;

    std::mutex mutex_;
    std::mutex resultMutex_;
    std::unordered_map<std::string_view, bool> validationCache;

    std::shared_ptr<ParallelStrategy> parallelStrategy;

    std::atomic<size_t> segmentsProcessed;
    std::atomic<size_t> validationFailures;

    auto validateSegment(const std::string_view& segment, size_t index) -> bool;

    void handleValidationError(const std::string_view& segment, size_t index);
};
}  // namespace lithium

#endif
