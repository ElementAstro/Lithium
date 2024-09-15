#ifndef ATOM_ASYNC_SIGNAL_HPP
#define ATOM_ASYNC_SIGNAL_HPP

#include <algorithm>
#include <functional>
#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

namespace atom::async {

/**
 * @brief A signal class that allows connecting, disconnecting, and emitting
 * slots.
 *
 * @tparam Args The argument types for the slots.
 */
template <typename... Args>
class Signal {
public:
    using SlotType = std::function<void(Args...)>;

    /**
     * @brief Connect a slot to the signal.
     *
     * @param slot The slot to connect.
     */
    void connect(SlotType slot) {
        std::lock_guard lock(mutex_);
        slots_.push_back(std::move(slot));
    }

    /**
     * @brief Disconnect a slot from the signal.
     *
     * @param slot The slot to disconnect.
     */
    void disconnect(const SlotType& slot) {
        std::lock_guard lock(mutex_);
        slots_.erase(std::remove_if(slots_.begin(), slots_.end(),
                                    [&](const SlotType& s) {
                                        return s.target_type() ==
                                               slot.target_type();
                                    }),
                     slots_.end());
    }

    /**
     * @brief Emit the signal, calling all connected slots.
     *
     * @param args The arguments to pass to the slots.
     */
    void emit(Args... args) {
        std::lock_guard lock(mutex_);
        for (const auto& slot : slots_) {
            slot(args...);
        }
    }

private:
    std::vector<SlotType> slots_;
    std::mutex mutex_;
};

/**
 * @brief A signal class that allows asynchronous slot execution.
 *
 * @tparam Args The argument types for the slots.
 */
template <typename... Args>
class AsyncSignal {
public:
    using SlotType = std::function<void(Args...)>;

    /**
     * @brief Connect a slot to the signal.
     *
     * @param slot The slot to connect.
     */
    void connect(SlotType slot) {
        std::lock_guard lock(mutex_);
        slots_.push_back(std::move(slot));
    }

    /**
     * @brief Disconnect a slot from the signal.
     *
     * @param slot The slot to disconnect.
     */
    void disconnect(const SlotType& slot) {
        std::lock_guard lock(mutex_);
        slots_.erase(std::remove_if(slots_.begin(), slots_.end(),
                                    [&](const SlotType& s) {
                                        return s.target_type() ==
                                               slot.target_type();
                                    }),
                     slots_.end());
    }

    /**
     * @brief Emit the signal asynchronously, calling all connected slots.
     *
     * @param args The arguments to pass to the slots.
     */
    void emit(Args... args) {
        std::vector<std::future<void>> futures;
        {
            std::lock_guard lock(mutex_);
            for (const auto& slot : slots_) {
                futures.push_back(
                    std::async(std::launch::async, slot, args...));
            }
        }
        for (auto& future : futures) {
            future.get();
        }
    }

private:
    std::vector<SlotType> slots_;
    std::mutex mutex_;
};

/**
 * @brief A signal class that allows automatic disconnection of slots.
 *
 * @tparam Args The argument types for the slots.
 */
template <typename... Args>
class AutoDisconnectSignal {
public:
    using SlotType = std::function<void(Args...)>;

    /**
     * @brief Connect a slot to the signal and return its unique ID.
     *
     * @param slot The slot to connect.
     * @return int The unique ID of the connected slot.
     */
    auto connect(SlotType slot) -> int {
        std::lock_guard lock(mutex_);
        auto id = nextId_++;
        slots_.emplace(id, std::move(slot));
        return id;
    }

    /**
     * @brief Disconnect a slot from the signal using its unique ID.
     *
     * @param id The unique ID of the slot to disconnect.
     */
    void disconnect(int id) {
        std::lock_guard lock(mutex_);
        slots_.erase(id);
    }

    /**
     * @brief Emit the signal, calling all connected slots.
     *
     * @param args The arguments to pass to the slots.
     */
    void emit(Args... args) {
        std::lock_guard lock(mutex_);
        for (const auto& [id, slot] : slots_) {
            slot(args...);
        }
    }

private:
    std::map<int, SlotType> slots_;
    std::mutex mutex_;
    int nextId_ = 0;
};

/**
 * @brief A signal class that allows chaining of signals.
 *
 * @tparam Args The argument types for the slots.
 */
template <typename... Args>
class ChainedSignal {
public:
    using SlotType = std::function<void(Args...)>;

    /**
     * @brief Connect a slot to the signal.
     *
     * @param slot The slot to connect.
     */
    void connect(SlotType slot) {
        std::lock_guard lock(mutex_);
        slots_.push_back(std::move(slot));
    }

    /**
     * @brief Add a chained signal to be emitted after this signal.
     *
     * @param nextSignal The next signal to chain.
     */
    void addChain(ChainedSignal<Args...>& nextSignal) {
        std::lock_guard lock(mutex_);
        chains_.push_back(&nextSignal);
    }

    /**
     * @brief Emit the signal, calling all connected slots and chained signals.
     *
     * @param args The arguments to pass to the slots.
     */
    void emit(Args... args) {
        std::lock_guard lock(mutex_);
        for (const auto& slot : slots_) {
            slot(args...);
        }
        for (auto& chain : chains_) {
            chain->emit(args...);
        }
    }

private:
    std::vector<SlotType> slots_;
    std::vector<ChainedSignal<Args...>*> chains_;
    std::mutex mutex_;
};

/**
 * @brief A signal class that allows connecting, disconnecting, and emitting
 * slots.
 *
 * @tparam Args The argument types for the slots.
 */
template <typename... Args>
class TemplateSignal {
public:
    using SlotType = std::function<void(Args...)>;

    /**
     * @brief Connect a slot to the signal.
     *
     * @param slot The slot to connect.
     */
    void connect(SlotType slot) {
        std::lock_guard lock(mutex_);
        slots_.push_back(std::move(slot));
    }

    /**
     * @brief Disconnect a slot from the signal.
     *
     * @param slot The slot to disconnect.
     */
    void disconnect(const SlotType& slot) {
        std::lock_guard lock(mutex_);
        slots_.erase(std::remove_if(slots_.begin(), slots_.end(),
                                    [&](const SlotType& s) {
                                        return s.target_type() ==
                                               slot.target_type();
                                    }),
                     slots_.end());
    }

    /**
     * @brief Emit the signal, calling all connected slots.
     *
     * @param args The arguments to pass to the slots.
     */
    void emit(Args... args) {
        std::lock_guard lock(mutex_);
        for (const auto& slot : slots_) {
            slot(args...);
        }
    }

private:
    std::vector<SlotType> slots_;
    std::mutex mutex_;
};

/**
 * @brief A signal class that ensures thread-safe slot execution.
 *
 * @tparam Args The argument types for the slots.
 */
template <typename... Args>
class ThreadSafeSignal {
public:
    using SlotType = std::function<void(Args...)>;

    /**
     * @brief Connect a slot to the signal.
     *
     * @param slot The slot to connect.
     */
    void connect(SlotType slot) {
        std::lock_guard lock(mutex_);
        slots_.push_back(std::move(slot));
    }

    /**
     * @brief Disconnect a slot from the signal.
     *
     * @param slot The slot to disconnect.
     */
    void disconnect(const SlotType& slot) {
        std::lock_guard lock(mutex_);
        slots_.erase(std::remove_if(slots_.begin(), slots_.end(),
                                    [&](const SlotType& s) {
                                        return s.target_type() ==
                                               slot.target_type();
                                    }),
                     slots_.end());
    }

    /**
     * @brief Emit the signal, calling all connected slots in a thread-safe
     * manner.
     *
     * @param args The arguments to pass to the slots.
     */
    void emit(Args... args) {
        std::vector<std::function<void()>> tasks;
        {
            std::lock_guard lock(mutex_);
            for (const auto& slot : slots_) {
                tasks.emplace_back([slot, args...]() { slot(args...); });
            }
        }
        for (auto& task : tasks) {
            std::async(std::launch::async, task).get();
        }
    }

private:
    std::vector<SlotType> slots_;
    std::mutex mutex_;
};

/**
 * @brief A signal class that allows broadcasting to chained signals.
 *
 * @tparam Args The argument types for the slots.
 */
template <typename... Args>
class BroadcastSignal {
public:
    using SlotType = std::function<void(Args...)>;

    /**
     * @brief Connect a slot to the signal.
     *
     * @param slot The slot to connect.
     */
    void connect(SlotType slot) {
        std::lock_guard lock(mutex_);
        slots_.push_back(std::move(slot));
    }

    /**
     * @brief Disconnect a slot from the signal.
     *
     * @param slot The slot to disconnect.
     */
    void disconnect(const SlotType& slot) {
        std::lock_guard lock(mutex_);
        slots_.erase(std::remove_if(slots_.begin(), slots_.end(),
                                    [&](const SlotType& s) {
                                        return s.target_type() ==
                                               slot.target_type();
                                    }),
                     slots_.end());
    }

    /**
     * @brief Emit the signal, calling all connected slots and chained signals.
     *
     * @param args The arguments to pass to the slots.
     */
    void emit(Args... args) {
        std::lock_guard lock(mutex_);
        for (const auto& slot : slots_) {
            slot(args...);
        }
        for (const auto& signal : chainedSignals_) {
            signal->emit(args...);
        }
    }

    /**
     * @brief Add a chained signal to be emitted after this signal.
     *
     * @param signal The next signal to chain.
     */
    void addChain(BroadcastSignal<Args...>& signal) {
        std::lock_guard lock(mutex_);
        chainedSignals_.push_back(&signal);
    }

private:
    std::vector<SlotType> slots_;
    std::vector<BroadcastSignal<Args...>*> chainedSignals_;
    std::mutex mutex_;
};

/**
 * @brief A signal class that limits the number of times it can be emitted.
 *
 * @tparam Args The argument types for the slots.
 */
template <typename... Args>
class LimitedSignal {
public:
    using SlotType = std::function<void(Args...)>;

    /**
     * @brief Construct a new Limited Signal object.
     *
     * @param maxCalls The maximum number of times the signal can be emitted.
     */
    explicit LimitedSignal(size_t maxCalls) : maxCalls_(maxCalls) {}

    /**
     * @brief Connect a slot to the signal.
     *
     * @param slot The slot to connect.
     */
    void connect(SlotType slot) {
        std::lock_guard lock(mutex_);
        slots_.push_back(std::move(slot));
    }

    /**
     * @brief Disconnect a slot from the signal.
     *
     * @param slot The slot to disconnect.
     */
    void disconnect(const SlotType& slot) {
        std::lock_guard lock(mutex_);
        slots_.erase(std::remove_if(slots_.begin(), slots_.end(),
                                    [&](const SlotType& s) {
                                        return s.target_type() ==
                                               slot.target_type();
                                    }),
                     slots_.end());
    }

    /**
     * @brief Emit the signal, calling all connected slots up to the maximum
     * number of calls.
     *
     * @param args The arguments to pass to the slots.
     */
    void emit(Args... args) {
        std::lock_guard lock(mutex_);
        if (callCount_ >= maxCalls_) {
            return;
        }
        for (const auto& slot : slots_) {
            slot(args...);
        }
        ++callCount_;
    }

private:
    std::vector<SlotType> slots_;
    size_t maxCalls_;
    size_t callCount_{};
    std::mutex mutex_;
};

/**
 * @brief A signal class that allows dynamic slot management.
 *
 * @tparam Args The argument types for the slots.
 */
template <typename... Args>
class DynamicSignal {
public:
    using SlotType = std::function<void(Args...)>;

    /**
     * @brief Connect a slot to the signal.
     *
     * @param slot The slot to connect.
     */
    void connect(SlotType slot) {
        std::lock_guard lock(mutex_);
        slots_.push_back(std::move(slot));
    }

    /**
     * @brief Disconnect a slot from the signal.
     *
     * @param slot The slot to disconnect.
     */
    void disconnect(const SlotType& slot) {
        std::lock_guard lock(mutex_);
        slots_.erase(std::remove_if(slots_.begin(), slots_.end(),
                                    [&](const SlotType& s) {
                                        return s.target_type() ==
                                               slot.target_type();
                                    }),
                     slots_.end());
    }

    /**
     * @brief Emit the signal, calling all connected slots.
     *
     * @param args The arguments to pass to the slots.
     */
    void emit(Args... args) {
        std::lock_guard lock(mutex_);
        for (const auto& slot : slots_) {
            slot(args...);
        }
    }

private:
    std::vector<SlotType> slots_;
    std::mutex mutex_;
};

/**
 * @brief A signal class that allows scoped slot management.
 *
 * @tparam Args The argument types for the slots.
 */
template <typename... Args>
class ScopedSignal {
public:
    using SlotType = std::function<void(Args...)>;

    /**
     * @brief Connect a slot to the signal using a shared pointer.
     *
     * @param slotPtr The shared pointer to the slot to connect.
     */
    void connect(std::shared_ptr<SlotType> slotPtr) {
        std::lock_guard lock(mutex_);
        slots_.push_back(slotPtr);
    }

    /**
     * @brief Emit the signal, calling all connected slots.
     *
     * @param args The arguments to pass to the slots.
     */
    void emit(Args... args) {
        std::lock_guard lock(mutex_);
        auto it = slots_.begin();
        while (it != slots_.end()) {
            if (auto slot = *it; slot) {
                (*slot)(args...);
                ++it;
            } else {
                it = slots_.erase(it);
            }
        }
    }

private:
    std::vector<std::shared_ptr<SlotType>> slots_;
    std::mutex mutex_;
};

}  // namespace atom::async

#endif