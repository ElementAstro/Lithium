/*
 * memory.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: A simple implementation of memory pool

**************************************************/

#ifndef ATOM_EXPERIMENT_MEMORY_POOL_HPP
#define ATOM_EXPERIMENT_MEMORY_POOL_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <memory_resource>
#include <mutex>
#include <vector>

#include "noncopyable.hpp"

template <typename T, size_t BlockSize = 4096>
class MemoryPool : public std::pmr::memory_resource, NonCopyable {
public:
  T *allocate(size_t n) {
    if (n > max_size()) {
      throw std::bad_alloc();
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (auto p = allocate_from_pool(n)) {
      return p;
    }
    size_t chunk_size = (n + chunk_space_ - 1) / chunk_space_ * chunk_space_;
    return allocate_from_chunk(chunk_size);
  }

  void deallocate(T *p, size_t n) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (is_from_pool(p)) {
      deallocate_to_pool(p, n);
    } else {
      deallocate_to_chunk(p, n);
    }
  }

  bool is_equal(const memory_resource &other) const noexcept {
    return this == &other;
  }

  size_t block_size() const { return BlockSize; }

private:
  struct Chunk {
    size_t size;
    size_t used;
    std::unique_ptr<uint8_t, std::function<void(uint8_t *)>> memory;
    Chunk(size_t s)
        : size(s), used(0), memory(static_cast<uint8_t *>(::operator new(s)),
                                   [](uint8_t *p) { ::operator delete(p); }) {}
  };

  size_t max_size() const { return BlockSize - sizeof(T); }
  size_t chunk_space() const { return BlockSize - sizeof(Chunk); }

  T *allocate_from_pool(size_t n) {
    if (pool_.empty() || pool_.back().used + n > pool_.back().size) {
      return nullptr;
    }
    auto &chunk = pool_.back();
    auto p = reinterpret_cast<T *>(chunk.memory.get() + chunk.used);
    chunk.used += n;
    return p;
  }

  void deallocate_to_pool(T *p, size_t n) {
    auto it = std::find_if(pool_.begin(), pool_.end(), [p](const Chunk &chunk) {
      return chunk.memory.get() <= reinterpret_cast<uint8_t *>(p) &&
             reinterpret_cast<uint8_t *>(p) < chunk.memory.get() + chunk.size;
    });
    assert(it != pool_.end());
    if (reinterpret_cast<uint8_t *>(p) + n == it->memory.get() + it->used) {
      it->used -= n;
    }
  }

  T *allocate_from_chunk(size_t chunk_size) {
    pool_.emplace_back(chunk_size);
    auto &chunk = pool_.back();
    auto p = reinterpret_cast<T *>(chunk.memory.get());
    chunk.used += chunk_size;
    return p;
  }

  void deallocate_to_chunk(T *p, [[maybe_unused]] size_t n) {
    auto it = std::find_if(pool_.begin(), pool_.end(), [p](const Chunk &chunk) {
      return chunk.memory.get() <= reinterpret_cast<uint8_t *>(p) &&
             reinterpret_cast<uint8_t *>(p) < chunk.memory.get() + chunk.size;
    });
    assert(it != pool_.end());
    if (it->used == it->size) {
      pool_.erase(it);
    }
  }

  bool is_from_pool(T *p) {
    for (const auto &chunk : pool_) {
      if (chunk.memory.get() <= reinterpret_cast<uint8_t *>(p) &&
          reinterpret_cast<uint8_t *>(p) < chunk.memory.get() + chunk.size) {
        return true;
      }
    }
    return false;
  }

  void *do_allocate(size_t bytes, size_t alignment) override {
    size_t space = (bytes + alignment - 1) / alignment * alignment;
    return allocate(space);
  }

  void do_deallocate(void *p, size_t bytes, size_t alignment) override {
    size_t space = (bytes + alignment - 1) / alignment * alignment;
    deallocate(static_cast<T *>(p), space);
  }

  bool do_is_equal(const memory_resource &other) const noexcept override {
    return this == &other;
  }

  size_t chunk_space_ = chunk_space();
  std::vector<Chunk> pool_;
  std::mutex mutex_;
};

#endif