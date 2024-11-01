#ifndef ATOM_TYPE_WEAK_PTR_HPP
#define ATOM_TYPE_WEAK_PTR_HPP

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <future>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>

namespace atom::type {

/**
 * @class EnhancedWeakPtr
 * @brief A class template that extends the functionality of std::weak_ptr.
 * @tparam T The type of the managed object.
 */
template <typename T>
class EnhancedWeakPtr {
private:
    std::weak_ptr<T> ptr_;      ///< The underlying weak pointer.
    mutable std::mutex mutex_;  ///< Mutex for thread-safe operations.
    mutable std::condition_variable
        cv_;  ///< Condition variable for synchronization.
    static inline std::atomic<size_t> totalInstances =
        0;  ///< Total number of EnhancedWeakPtr instances.
    mutable std::atomic<size_t> lockAttempts_ =
        0;  ///< Number of lock attempts.

public:
    /**
     * @brief Default constructor.
     */
    EnhancedWeakPtr() : ptr_() { ++totalInstances; }

    /**
     * @brief Constructs an EnhancedWeakPtr from a shared pointer.
     * @param shared The shared pointer to manage.
     */
    explicit EnhancedWeakPtr(const std::shared_ptr<T>& shared) : ptr_(shared) {
        ++totalInstances;
    }

    /**
     * @brief Destructor.
     */
    ~EnhancedWeakPtr() { --totalInstances; }

    /**
     * @brief Copy constructor.
     * @param other The other EnhancedWeakPtr to copy from.
     */
    EnhancedWeakPtr(const EnhancedWeakPtr& other) : ptr_(other.ptr_) {
        ++totalInstances;
    }

    /**
     * @brief Move constructor.
     * @param other The other EnhancedWeakPtr to move from.
     */
    EnhancedWeakPtr(EnhancedWeakPtr&& other) noexcept
        : ptr_(std::move(other.ptr_)) {
        ++totalInstances;
    }

    /**
     * @brief Copy assignment operator.
     * @param other The other EnhancedWeakPtr to copy from.
     * @return A reference to this EnhancedWeakPtr.
     */
    auto operator=(const EnhancedWeakPtr& other) -> EnhancedWeakPtr& {
        if (this != &other) {
            ptr_ = other.ptr_;
        }
        return *this;
    }

    /**
     * @brief Move assignment operator.
     * @param other The other EnhancedWeakPtr to move from.
     * @return A reference to this EnhancedWeakPtr.
     */
    auto operator=(EnhancedWeakPtr&& other) noexcept -> EnhancedWeakPtr& {
        if (this != &other) {
            ptr_ = std::move(other.ptr_);
        }
        return *this;
    }

    /**
     * @brief Locks the weak pointer and returns a shared pointer.
     * @return A shared pointer to the managed object, or nullptr if the object
     * has expired.
     */
    auto lock() const -> std::shared_ptr<T> {
        ++lockAttempts_;
        return ptr_.lock();
    }

    /**
     * @brief Checks if the managed object has expired.
     * @return True if the object has expired, false otherwise.
     */
    auto expired() const -> bool { return ptr_.expired(); }

    /**
     * @brief Resets the weak pointer.
     */
    void reset() { ptr_.reset(); }

    /**
     * @brief Executes a function with a locked shared pointer.
     * @tparam Func The type of the function.
     * @tparam R The return type of the function.
     * @param func The function to execute.
     * @return The result of the function, or std::nullopt if the object has
     * expired.
     */
    template <typename Func, typename R = std::invoke_result_t<Func, T&>>
    auto withLock(Func&& func) const
        -> std::conditional_t<std::is_void_v<R>, bool, std::optional<R>> {
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

    /**
     * @brief Waits for the managed object to become available or for a timeout.
     * @tparam Rep The representation of the duration.
     * @tparam Period The period of the duration.
     * @param timeout The timeout duration.
     * @return True if the object became available, false if the timeout was
     * reached.
     */
    template <typename Rep, typename Period>
    auto waitFor(const std::chrono::duration<Rep, Period>& timeout) const
        -> bool {
        std::unique_lock<std::mutex> lock(mutex_);
        return cv_.wait_for(lock, timeout, [this] { return !this->expired(); });
    }

    /**
     * @brief Equality operator.
     * @param other The other EnhancedWeakPtr to compare with.
     * @return True if the weak pointers are equal, false otherwise.
     */
    auto operator==(const EnhancedWeakPtr& other) const -> bool {
        return !ptr_.owner_before(other.ptr_) && !other.ptr_.owner_before(ptr_);
    }

    /**
     * @brief Gets the use count of the managed object.
     * @return The use count of the managed object.
     */
    auto useCount() const -> long { return ptr_.use_count(); }

    /**
     * @brief Gets the total number of EnhancedWeakPtr instances.
     * @return The total number of EnhancedWeakPtr instances.
     */
    static auto getTotalInstances() -> size_t { return totalInstances; }

    /**
     * @brief Tries to lock the weak pointer and executes one of two functions
     * based on success or failure.
     * @tparam SuccessFunc The type of the success function.
     * @tparam FailureFunc The type of the failure function.
     * @param success The function to execute on success.
     * @param failure The function to execute on failure.
     * @return The result of the success or failure function.
     */
    template <typename SuccessFunc, typename FailureFunc>
    auto tryLockOrElse(SuccessFunc&& success, FailureFunc&& failure) const {
        if (auto shared = lock()) {
            return std::forward<SuccessFunc>(success)(*shared);
        }
        return std::forward<FailureFunc>(failure)();
    }

    /**
     * @brief Tries to lock the weak pointer periodically until success or a
     * maximum number of attempts.
     * @tparam Rep The representation of the duration.
     * @tparam Period The period of the duration.
     * @param interval The interval between attempts.
     * @param maxAttempts The maximum number of attempts.
     * @return A shared pointer to the managed object, or nullptr if the maximum
     * number of attempts was reached.
     */
    template <typename Rep, typename Period>
    auto tryLockPeriodic(
        const std::chrono::duration<Rep, Period>& interval,
        size_t maxAttempts = std::numeric_limits<size_t>::max())
        -> std::shared_ptr<T> {
        for (size_t i = 0; i < maxAttempts; ++i) {
            if (auto shared = lock()) {
                return shared;
            }
            std::this_thread::sleep_for(interval);
        }
        return nullptr;
    }

    /**
     * @brief Gets the underlying weak pointer.
     * @return The underlying weak pointer.
     */
    auto getWeakPtr() const -> std::weak_ptr<T> { return ptr_; }

    /**
     * @brief Creates a shared pointer from the weak pointer.
     * @return A shared pointer to the managed object.
     */
    auto createShared() const -> std::shared_ptr<T> {
        return std::shared_ptr<T>(ptr_);
    }

    /**
     * @brief Notifies all waiting threads.
     */
    void notifyAll() const { cv_.notify_all(); }

    /**
     * @brief Gets the number of lock attempts.
     * @return The number of lock attempts.
     */
    auto getLockAttempts() const -> size_t { return lockAttempts_; }

    /**
     * @brief Asynchronously locks the weak pointer.
     * @return A future that resolves to a shared pointer to the managed object.
     */
    auto asyncLock() const -> std::future<std::shared_ptr<T>> {
        return std::async(std::launch::async, [this] { return this->lock(); });
    }

    /**
     * @brief Waits until a predicate is satisfied or the managed object
     * expires.
     * @tparam Predicate The type of the predicate.
     * @param pred The predicate to satisfy.
     * @return True if the predicate was satisfied, false if the object expired.
     */
    template <typename Predicate>
    auto waitUntil(Predicate pred) const -> bool {
        std::unique_lock<std::mutex> lock(mutex_);
        return cv_.wait(lock,
                        [this, &pred] { return !this->expired() && pred(); });
    }

    /**
     * @brief Casts the weak pointer to a different type.
     * @tparam U The type to cast to.
     * @return An EnhancedWeakPtr of the new type.
     */
    template <typename U>
    auto cast() const -> EnhancedWeakPtr<U> {
        return EnhancedWeakPtr<U>(std::static_pointer_cast<U>(ptr_.lock()));
    }
};

/**
 * @brief Creates a group of EnhancedWeakPtr from a vector of shared pointers.
 * @tparam T The type of the managed objects.
 * @param sharedPtrs The vector of shared pointers.
 * @return A vector of EnhancedWeakPtr.
 */
template <typename T>
auto createWeakPtrGroup(const std::vector<std::shared_ptr<T>>& sharedPtrs)
    -> std::vector<EnhancedWeakPtr<T>> {
    std::vector<EnhancedWeakPtr<T>> weakPtrs;
    weakPtrs.reserve(sharedPtrs.size());
    std::transform(
        sharedPtrs.begin(), sharedPtrs.end(), std::back_inserter(weakPtrs),
        [](const auto& sharedPtr) { return EnhancedWeakPtr<T>(sharedPtr); });
    return weakPtrs;
}

/**
 * @brief Performs a batch operation on a vector of EnhancedWeakPtr.
 * @tparam T The type of the managed objects.
 * @tparam Func The type of the function to execute.
 * @param weakPtrs The vector of EnhancedWeakPtr.
 * @param func The function to execute.
 */
template <typename T, typename Func>
void batchOperation(const std::vector<EnhancedWeakPtr<T>>& weakPtrs,
                    Func&& func) {
    for (const auto& weakPtr : weakPtrs) {
        weakPtr.withLock(std::forward<Func>(func));
    }
}

}  // namespace atom::type

#endif
