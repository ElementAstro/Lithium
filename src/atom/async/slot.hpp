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
template <typename... Args>
class Signal {
public:
    using SlotType = std::function<void(Args...)>;

    void connect(SlotType slot) {
        std::lock_guard lock(mutex_);
        slots_.push_back(std::move(slot));
    }

    void disconnect(const SlotType& slot) {
        std::lock_guard lock(mutex_);
        slots_.erase(std::remove_if(slots_.begin(), slots_.end(),
                                    [&](const SlotType& s) {
                                        return s.target_type() ==
                                               slot.target_type();
                                    }),
                     slots_.end());
    }

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

template <typename... Args>
class AsyncSignal {
public:
    using SlotType = std::function<void(Args...)>;

    void connect(SlotType slot) {
        std::lock_guard lock(mutex_);
        slots_.push_back(std::move(slot));
    }

    void disconnect(const SlotType& slot) {
        std::lock_guard lock(mutex_);
        slots_.erase(std::remove_if(slots_.begin(), slots_.end(),
                                    [&](const SlotType& s) {
                                        return s.target_type() ==
                                               slot.target_type();
                                    }),
                     slots_.end());
    }

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

template <typename... Args>
class AutoDisconnectSignal {
public:
    using SlotType = std::function<void(Args...)>;

    auto connect(SlotType slot) -> int {
        std::lock_guard lock(mutex_);
        auto id = nextId_++;
        slots_.emplace(id, std::move(slot));
        return id;
    }

    void disconnect(int id) {
        std::lock_guard lock(mutex_);
        slots_.erase(id);
    }

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

template <typename... Args>
class ChainedSignal {
public:
    using SlotType = std::function<void(Args...)>;

    void connect(SlotType slot) {
        std::lock_guard lock(mutex_);
        slots_.push_back(std::move(slot));
    }

    void addChain(ChainedSignal<Args...>& nextSignal) {
        std::lock_guard lock(mutex_);
        chains_.push_back(&nextSignal);
    }

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

template <typename... Args>
class TemplateSignal {
public:
    using SlotType = std::function<void(Args...)>;

    void connect(SlotType slot) {
        std::lock_guard lock(mutex_);
        slots_.push_back(std::move(slot));
    }

    void disconnect(const SlotType& slot) {
        std::lock_guard lock(mutex_);
        slots_.erase(std::remove_if(slots_.begin(), slots_.end(),
                                    [&](const SlotType& s) {
                                        return s.target_type() ==
                                               slot.target_type();
                                    }),
                     slots_.end());
    }

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

template <typename... Args>
class ThreadSafeSignal {
public:
    using SlotType = std::function<void(Args...)>;

    void connect(SlotType slot) {
        std::lock_guard lock(mutex_);
        slots_.push_back(std::move(slot));
    }

    void disconnect(const SlotType& slot) {
        std::lock_guard lock(mutex_);
        slots_.erase(std::remove_if(slots_.begin(), slots_.end(),
                                    [&](const SlotType& s) {
                                        return s.target_type() ==
                                               slot.target_type();
                                    }),
                     slots_.end());
    }

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

template <typename... Args>
class BroadcastSignal {
public:
    using SlotType = std::function<void(Args...)>;

    void connect(SlotType slot) {
        std::lock_guard lock(mutex_);
        slots_.push_back(std::move(slot));
    }

    void disconnect(const SlotType& slot) {
        std::lock_guard lock(mutex_);
        slots_.erase(std::remove_if(slots_.begin(), slots_.end(),
                                    [&](const SlotType& s) {
                                        return s.target_type() ==
                                               slot.target_type();
                                    }),
                     slots_.end());
    }

    void emit(Args... args) {
        std::lock_guard lock(mutex_);
        for (const auto& slot : slots_) {
            slot(args...);
        }
        for (const auto& signal : chainedSignals_) {
            signal->emit(args...);
        }
    }

    void addChain(BroadcastSignal<Args...>& signal) {
        std::lock_guard lock(mutex_);
        chainedSignals_.push_back(&signal);
    }

private:
    std::vector<SlotType> slots_;
    std::vector<BroadcastSignal<Args...>*> chainedSignals_;
    std::mutex mutex_;
};

template <typename... Args>
class LimitedSignal {
public:
    using SlotType = std::function<void(Args...)>;

    explicit LimitedSignal(size_t maxCalls) : maxCalls_(maxCalls) {}

    void connect(SlotType slot) {
        std::lock_guard lock(mutex_);
        slots_.push_back(std::move(slot));
    }

    void disconnect(const SlotType& slot) {
        std::lock_guard lock(mutex_);
        slots_.erase(std::remove_if(slots_.begin(), slots_.end(),
                                    [&](const SlotType& s) {
                                        return s.target_type() ==
                                               slot.target_type();
                                    }),
                     slots_.end());
    }

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

template <typename... Args>
class DynamicSignal {
public:
    using SlotType = std::function<void(Args...)>;

    void connect(SlotType slot) {
        std::lock_guard lock(mutex_);
        slots_.push_back(std::move(slot));
    }

    void disconnect(const SlotType& slot) {
        std::lock_guard lock(mutex_);
        slots_.erase(std::remove_if(slots_.begin(), slots_.end(),
                                    [&](const SlotType& s) {
                                        return s.target_type() ==
                                               slot.target_type();
                                    }),
                     slots_.end());
    }

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

template <typename... Args>
class ScopedSignal {
public:
    using SlotType = std::function<void(Args...)>;

    void connect(std::shared_ptr<SlotType> slotPtr) {
        std::lock_guard lock(mutex_);
        slots_.push_back(slotPtr);
    }

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