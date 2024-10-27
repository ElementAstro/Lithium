/*!
 * \file stepper.hpp
 * \brief Proxy Function Sequence
 * \author Max Qian <lightapt.com>
 * \date 2024-03-01, Updated 2024-10-14
 */

#ifndef ATOM_META_SEQUENCE_HPP
#define ATOM_META_SEQUENCE_HPP

#include <any>
#include <chrono>
#include <functional>
#include <future>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

#include "atom/algorithm/hash.hpp"
#include "atom/error/exception.hpp"

namespace atom::meta {

class FunctionSequence {
public:
    using FunctionType = std::function<std::any(const std::vector<std::any>&)>;

    // Register a function to be part of the sequence
    void registerFunction(FunctionType func) {
        std::unique_lock lock(mutex_);
        functions_.emplace_back(std::move(func));
    }

    // Run the last function with each set of arguments provided
    auto run(const std::vector<std::vector<std::any>>& argsBatch)
        -> std::vector<std::any> {
        std::vector<std::any> results;
        std::shared_lock lock(mutex_);
        if (functions_.empty()) {
            THROW_EXCEPTION("No functions registered in the sequence");
            return results;
        }

        try {
            auto& func = functions_.back();
            results.reserve(argsBatch.size());
            for (const auto& args : argsBatch) {
                results.emplace_back(func(args));
            }
        } catch (const std::exception& e) {
            THROW_EXCEPTION(std::string("Exception caught: ") + e.what());
        }
        return results;
    }

    // Run all functions with each set of arguments provided and return the
    // results of each function
    auto runAll(const std::vector<std::vector<std::any>>& argsBatch)
        -> std::vector<std::vector<std::any>> {
        std::vector<std::vector<std::any>> resultsBatch;
        std::shared_lock lock(mutex_);
        if (functions_.empty()) {
            THROW_EXCEPTION("No functions registered in the sequence");
            return resultsBatch;
        }

        try {
            resultsBatch.reserve(argsBatch.size());
            for (const auto& args : argsBatch) {
                std::vector<std::any> results;
                results.reserve(functions_.size());
                for (const auto& func : functions_) {
                    results.emplace_back(func(args));
                }
                resultsBatch.emplace_back(std::move(results));
            }
        } catch (const std::exception& e) {
            THROW_EXCEPTION(std::string("Exception caught: ") + e.what());
        }
        return resultsBatch;
    }

    // Run the last function asynchronously with each set of arguments provided
    auto runAsync(const std::vector<std::vector<std::any>>& argsBatch)
        -> std::future<std::vector<std::any>> {
        return std::async(std::launch::async,
                          [this, argsBatch] { return this->run(argsBatch); });
    }

    // Run all functions asynchronously with each set of arguments provided
    auto runAllAsync(const std::vector<std::vector<std::any>>& argsBatch)
        -> std::future<std::vector<std::vector<std::any>>> {
        return std::async(std::launch::async, [this, argsBatch] {
            return this->runAll(argsBatch);
        });
    }

    // Run the last function with each set of arguments provided, with a timeout
    auto runWithTimeout(const std::vector<std::vector<std::any>>& argsBatch,
                        std::chrono::milliseconds timeout)
        -> std::vector<std::any> {
        auto future = runAsync(argsBatch);
        if (future.wait_for(timeout) == std::future_status::timeout) {
            THROW_EXCEPTION("Function execution timed out");
        }
        return future.get();
    }

    // Run all functions with each set of arguments provided, with a timeout
    auto runAllWithTimeout(const std::vector<std::vector<std::any>>& argsBatch,
                           std::chrono::milliseconds timeout)
        -> std::vector<std::vector<std::any>> {
        auto future = runAllAsync(argsBatch);
        if (future.wait_for(timeout) == std::future_status::timeout) {
            THROW_EXCEPTION("Function execution timed out");
        }
        return future.get();
    }

    // Run the last function with each set of arguments provided, with retries
    auto runWithRetries(const std::vector<std::vector<std::any>>& argsBatch,
                        int retries) -> std::vector<std::any> {
        while (retries-- > 0) {
            try {
                return run(argsBatch);
            } catch (...) {
                if (retries == 0) {
                    throw;
                }
            }
        }
        return {};
    }

    // Run all functions with each set of arguments provided, with retries
    auto runAllWithRetries(const std::vector<std::vector<std::any>>& argsBatch,
                           int retries) -> std::vector<std::vector<std::any>> {
        while (retries-- > 0) {
            try {
                return runAll(argsBatch);
            } catch (...) {
                if (retries == 0) {
                    throw;
                }
            }
        }
        return {};
    }

#if ATOM_DEBUG_OUTPUT
    // Log the execution of the last function with each set of arguments
    // provided
    auto runWithLogging(const std::vector<std::vector<std::any>>& argsBatch)
        -> std::vector<std::any> {
        auto start = std::chrono::high_resolution_clock::now();
        auto results = run(argsBatch);
        auto end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> duration = end - start;

        std::cout << "Function execution took " << duration.count()
                  << " seconds\n";
        return results;
    }

    // Log the execution of all functions with each set of arguments provided
    auto runAllWithLogging(const std::vector<std::vector<std::any>>& argsBatch)
        -> std::vector<std::vector<std::any>> {
        auto start = std::chrono::high_resolution_clock::now();
        auto results = runAll(argsBatch);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        std::cout << "Function execution took " << duration.count()
                  << " seconds\n";
        return results;
    }
#endif

    // Cache the results of the last function with each set of arguments
    // provided
    auto runWithCaching(const std::vector<std::vector<std::any>>& argsBatch)
        -> std::vector<std::any> {
        std::vector<std::any> results;
        std::shared_lock lock(mutex_);
        if (functions_.empty()) {
            THROW_EXCEPTION("No functions registered in the sequence");
            return results;
        }

        try {
            auto& func = functions_.back();
            results.reserve(argsBatch.size());
            for (const auto& args : argsBatch) {
                auto key = generateCacheKey(args);
                {
                    std::shared_lock cacheLock(cacheMutex_);
                    if (auto it = cache_.find(key); it != cache_.end()) {
                        results.emplace_back(it->second);
                        continue;
                    }
                }
                auto result = func(args);
                {
                    std::unique_lock cacheLock(cacheMutex_);
                    cache_[key] = result;
                }
                results.emplace_back(result);
            }
        } catch (const std::exception& e) {
            THROW_EXCEPTION(std::string("Exception caught: ") + e.what());
        }
        return results;
    }

    // Cache the results of all functions with each set of arguments provided
    auto runAllWithCaching(const std::vector<std::vector<std::any>>& argsBatch)
        -> std::vector<std::vector<std::any>> {
        std::vector<std::vector<std::any>> resultsBatch;
        std::shared_lock lock(mutex_);
        if (functions_.empty()) {
            THROW_EXCEPTION("No functions registered in the sequence");
            return resultsBatch;
        }

        try {
            resultsBatch.reserve(argsBatch.size());
            for (const auto& args : argsBatch) {
                std::vector<std::any> results;
                results.reserve(functions_.size());
                for (const auto& func : functions_) {
                    auto key = generateCacheKey(args);
                    {
                        std::shared_lock cacheLock(cacheMutex_);
                        if (auto it = cache_.find(key); it != cache_.end()) {
                            results.emplace_back(it->second);
                            continue;
                        }
                    }
                    auto result = func(args);
                    {
                        std::unique_lock cacheLock(cacheMutex_);
                        cache_[key] = result;
                    }
                    results.emplace_back(result);
                }
                resultsBatch.emplace_back(std::move(results));
            }
        } catch (const std::exception& e) {
            THROW_EXCEPTION(std::string("Exception caught: ") + e.what());
        }
        return resultsBatch;
    }

    // Notify listeners of the results of the last function with each set of
    // arguments provided
    auto runWithNotification(
        const std::vector<std::vector<std::any>>& argsBatch,
        const std::function<void(const std::any&)>& callback)
        -> std::vector<std::any> {
        auto results = run(argsBatch);
        for (const auto& result : results) {
            callback(result);
        }
        return results;
    }

    // Notify listeners of the results of all functions with each set of
    // arguments provided
    auto runAllWithNotification(
        const std::vector<std::vector<std::any>>& argsBatch,
        const std::function<void(const std::vector<std::any>&)>& callback)
        -> std::vector<std::vector<std::any>> {
        auto resultsBatch = runAll(argsBatch);
        for (const auto& results : resultsBatch) {
            callback(results);
        }
        return resultsBatch;
    }

private:
    std::vector<FunctionType> functions_;
    mutable std::shared_mutex mutex_;
    mutable std::unordered_map<std::string, std::any> cache_;
    mutable std::shared_mutex cacheMutex_;

    // Generate a cache key based on the arguments
    static auto generateCacheKey(const std::vector<std::any>& args)
        -> std::string {
        std::string key;
        for (const auto& arg : args) {
            key += std::to_string(algorithm::computeHash(arg));
        }
        return key;
    }
};

}  // namespace atom::meta

#endif  // ATOM_META_SEQUENCE_HPP
