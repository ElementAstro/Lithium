/*
 * object.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-5

Description: A simple implementation of object pool

**************************************************/

#ifndef ATOM_EXPERIMENT_OBJECT_HPP
#define ATOM_EXPERIMENT_OBJECT_HPP

#include <cassert>
#include <concepts>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <vector>

template <typename T>
concept Resettable = requires(T &obj) { obj.reset(); };

template <Resettable T> class ObjectPool {
public:
  ObjectPool(size_t max_size) : max_size_(max_size), available_(max_size) {}

  std::shared_ptr<T> acquire() {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this] { return !pool_.empty() || available_ > 0; });
    if (!pool_.empty()) {
      auto obj = std::move(pool_.back());
      pool_.pop_back();
      return obj;
    }
    assert(available_ > 0);
    --available_;
    return std::make_shared<T>();
  }

  void release(std::shared_ptr<T> obj) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (pool_.size() < max_size_) {
      obj->reset();
      pool_.push_back(std::move(obj));
      cv_.notify_one();
    } else {
      ++available_;
      cv_.notify_one();
    }
  }

  size_t available() const {
    std::unique_lock<std::mutex> lock(mutex_);
    return available_ + pool_.size();
  }

  size_t size() const {
    std::unique_lock<std::mutex> lock(mutex_);
    return max_size_ - available_;
  }

private:
  size_t max_size_;
  size_t available_;
  std::vector<std::shared_ptr<T>> pool_;
  mutable std::mutex mutex_;
  std::condition_variable cv_;
};

#endif
