#ifndef ATOM_TYPE_WEAK_PTR_HPP
#define ATOM_TYPE_WEAK_PTR_HPP

#include <memory>
#include <optional>
#include <chrono>
#include <vector>
#include <algorithm>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <future>
#include <thread>

namespace atom::type {
template<typename T>
class EnhancedWeakPtr {
private:
    std::weak_ptr<T> ptr_;
    mutable std::mutex mutex_;
    mutable std::condition_variable cv_;
    static inline std::atomic<size_t> totalInstances = 0;
    mutable std::atomic<size_t> lockAttempts_ = 0;

public:
    EnhancedWeakPtr() : ptr_() { ++totalInstances; }
    explicit EnhancedWeakPtr(const std::shared_ptr<T>& shared) : ptr_(shared) { ++totalInstances; }
    ~EnhancedWeakPtr() { --totalInstances; }

    EnhancedWeakPtr(const EnhancedWeakPtr& other) : ptr_(other.ptr_) { ++totalInstances; }
    EnhancedWeakPtr(EnhancedWeakPtr&& other) noexcept : ptr_(std::move(other.ptr_)) { ++totalInstances; }

    auto operator=(const EnhancedWeakPtr& other) -> EnhancedWeakPtr& {
        if (this != &other) {
            ptr_ = other.ptr_;
        }
        return *this;
    }

    auto operator=(EnhancedWeakPtr&& other) noexcept -> EnhancedWeakPtr& {
        if (this != &other) {
            ptr_ = std::move(other.ptr_);
        }
        return *this;
    }

    auto lock() const -> std::shared_ptr<T> {
        ++lockAttempts_;
        return ptr_.lock();
    }

    auto expired() const -> bool { return ptr_.expired(); }
    void reset() { ptr_.reset(); }

    template<typename Func, typename R = std::invoke_result_t<Func, T&>>
    auto withLock(Func&& func) const -> std::conditional_t<std::is_void_v<R>, bool, std::optional<R>> {
        if (auto shared = lock()) {
            if constexpr (std::is_void_v<R>) {
                std::forward<Func>(func)(*shared);
                return true;
            } else {
                return std::forward<Func>(func)(*shared);
            }
        }
        if constexpr (std::is_void_v<R>) {
            return false;
        } else {
            return std::nullopt;
        }
    }

    template<typename Rep, typename Period>
    auto waitFor(const std::chrono::duration<Rep, Period>& timeout) const -> bool {
        std::unique_lock<std::mutex> lock(mutex_);
        return cv_.wait_for(lock, timeout, [this] { return !this->expired(); });
    }

    auto operator==(const EnhancedWeakPtr& other) const -> bool {
        return !ptr_.owner_before(other.ptr_) && !other.ptr_.owner_before(ptr_);
    }

    auto useCount() const -> long { return ptr_.use_count(); }

    static auto getTotalInstances() -> size_t { return totalInstances; }

    template<typename SuccessFunc, typename FailureFunc>
    auto tryLockOrElse(SuccessFunc&& success, FailureFunc&& failure) const {
        if (auto shared = lock()) {
            return std::forward<SuccessFunc>(success)(*shared);
        }
        return std::forward<FailureFunc>(failure)();
    }

    template<typename Rep, typename Period>
    auto tryLockPeriodic(const std::chrono::duration<Rep, Period>& interval, size_t maxAttempts = std::numeric_limits<size_t>::max()) -> std::shared_ptr<T> {
        for (size_t i = 0; i < maxAttempts; ++i) {
            if (auto shared = lock()) {
                return shared;
            }
            std::this_thread::sleep_for(interval);
        }
        return nullptr;
    }

    auto getWeakPtr() const -> std::weak_ptr<T> { return ptr_; }
    auto createShared() const -> std::shared_ptr<T> { return std::shared_ptr<T>(ptr_); }
    void notifyAll() const { cv_.notify_all(); }

    auto getLockAttempts() const -> size_t { return lockAttempts_; }

    auto asyncLock() const -> std::future<std::shared_ptr<T>> {
        return std::async(std::launch::async, [this] { return this->lock(); });
    }

    template<typename Predicate>
    auto waitUntil(Predicate pred) const -> bool {
        std::unique_lock<std::mutex> lock(mutex_);
        return cv_.wait(lock, [this, &pred] { return !this->expired() && pred(); });
    }

    template<typename U>
    auto cast() const -> EnhancedWeakPtr<U> {
        return EnhancedWeakPtr<U>(std::static_pointer_cast<U>(ptr_.lock()));
    }
};

template<typename T>
auto createWeakPtrGroup(const std::vector<std::shared_ptr<T>>& sharedPtrs) -> std::vector<EnhancedWeakPtr<T>> {
    std::vector<EnhancedWeakPtr<T>> weakPtrs;
    weakPtrs.reserve(sharedPtrs.size());
    std::transform(sharedPtrs.begin(), sharedPtrs.end(), std::back_inserter(weakPtrs),
                   [](const auto& sharedPtr) { return EnhancedWeakPtr<T>(sharedPtr); });
    return weakPtrs;
}

template<typename T, typename Func>
void batchOperation(const std::vector<EnhancedWeakPtr<T>>& weakPtrs, Func&& func) {
    for (const auto& weakPtr : weakPtrs) {
        weakPtr.withLock(std::forward<Func>(func));
    }
}

}

#endif
