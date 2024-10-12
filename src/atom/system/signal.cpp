#include "signal.hpp"
#include <algorithm>
#include <csignal>
#include <iostream>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "atom/log/loguru.hpp"

constexpr int SLEEP_DURATION_MS = 10;

auto SignalHandlerWithPriority::operator<(
    const SignalHandlerWithPriority& other) const -> bool {
    return priority > other.priority;  // Higher priority handlers run first
}

auto SignalHandlerRegistry::getInstance() -> SignalHandlerRegistry& {
    static SignalHandlerRegistry instance;
    return instance;
}

void SignalHandlerRegistry::setSignalHandler(SignalID signal,
                                             const SignalHandler& handler,
                                             int priority) {
    std::lock_guard lock(mutex_);
    handlers_[signal].emplace(handler, priority);
    auto previousHandler = std::signal(signal, signalDispatcher);
    if (previousHandler == SIG_ERR) {
        std::cerr << "Error setting signal handler for signal " << signal << std::endl;
    }
}

void SignalHandlerRegistry::removeSignalHandler(SignalID signal,
                                                const SignalHandler& handler) {
    std::lock_guard lock(mutex_);
    auto handlerIterator = handlers_.find(signal);
    if (handlerIterator != handlers_.end()) {
        auto handlerWithPriority =
            std::find_if(handlerIterator->second.begin(), handlerIterator->second.end(),
                         [&handler](const SignalHandlerWithPriority& handlerPriority) {
                             return handlerPriority.handler.target<void(SignalID)>() ==
                                    handler.target<void(SignalID)>();
                         });
        if (handlerWithPriority != handlerIterator->second.end()) {
            handlerIterator->second.erase(handlerWithPriority);
        }
        if (handlerIterator->second.empty()) {
            handlers_.erase(handlerIterator);
            auto previousHandler = std::signal(signal, SIG_DFL);
            if (previousHandler == SIG_ERR) {
                std::cerr << "Error resetting signal handler for signal " << signal << std::endl;
            }
        }
    }
}

void SignalHandlerRegistry::setStandardCrashHandlerSignals(
    const SignalHandler& handler, int priority) {
    #pragma unroll
    for (SignalID signal : getStandardCrashSignals()) {
        setSignalHandler(signal, handler, priority);
    }
}

SignalHandlerRegistry::SignalHandlerRegistry() = default;

SignalHandlerRegistry::~SignalHandlerRegistry() = default;

void SignalHandlerRegistry::signalDispatcher(int signal) {
    SignalHandlerRegistry& registry = getInstance();
    std::lock_guard lock(registry.mutex_);
    auto handlerIterator = registry.handlers_.find(signal);
    if (handlerIterator != registry.handlers_.end()) {
        #pragma unroll
        for (const auto& handler : handlerIterator->second) {
            handler.handler(signal);
        }
    }
}

auto SignalHandlerRegistry::getStandardCrashSignals() -> std::set<SignalID> {
#if defined(_WIN32) || defined(_WIN64)
    return {SIGABRT, SIGFPE, SIGILL, SIGSEGV, SIGTERM};
#else
    return {SIGABRT, SIGILL, SIGFPE, SIGSEGV, SIGBUS, SIGQUIT};
#endif
}

SafeSignalManager::SafeSignalManager() : keepRunning_(true) {
    signalHandlingThread_ = std::jthread([this]() { processSignals(); });
}

SafeSignalManager::~SafeSignalManager() {
    keepRunning_ = false;
    clearSignalQueue();
    signalHandlingThread_.join();
}

void SafeSignalManager::addSafeSignalHandler(SignalID signal,
                                             const SignalHandler& handler,
                                             int priority) {
    std::lock_guard lock(queueMutex_);
    safeHandlers_[signal].emplace(handler, priority);
}

void SafeSignalManager::removeSafeSignalHandler(SignalID signal,
                                                const SignalHandler& handler) {
    std::lock_guard lock(queueMutex_);
    auto handlerIterator = safeHandlers_.find(signal);
    if (handlerIterator != safeHandlers_.end()) {
        auto handlerWithPriority =
            std::find_if(handlerIterator->second.begin(), handlerIterator->second.end(),
                         [&handler](const SignalHandlerWithPriority& handlerPriority) {
                             return handlerPriority.handler.target<void(SignalID)>() ==
                                    handler.target<void(SignalID)>();
                         });
        if (handlerWithPriority != handlerIterator->second.end()) {
            handlerIterator->second.erase(handlerWithPriority);
        }
        if (handlerIterator->second.empty()) {
            safeHandlers_.erase(handlerIterator);
        }
    }
}

void SafeSignalManager::safeSignalDispatcher(int signal) {
    auto& manager = getInstance();
    std::lock_guard lock(manager.queueMutex_);
    manager.signalQueue_.push_back(signal);
}

auto SafeSignalManager::getInstance() -> SafeSignalManager& {
    static SafeSignalManager instance;
    return instance;
}

void SafeSignalManager::processSignals() {
    while (keepRunning_) {
        int signal = 0;
        {
            std::lock_guard lock(queueMutex_);
            if (!signalQueue_.empty()) {
                signal = signalQueue_.front();
                signalQueue_.erase(signalQueue_.begin());
            }
        }

        if (signal != 0) {
            auto handlerIterator = safeHandlers_.find(signal);
            if (handlerIterator != safeHandlers_.end()) {
                #pragma unroll
                for (const auto& handler : handlerIterator->second) {
                    handler.handler(signal);
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_DURATION_MS));
    }
}

void SafeSignalManager::clearSignalQueue() {
    std::lock_guard lock(queueMutex_);
    signalQueue_.clear();  // Clear any remaining signals to avoid deadlock.
}

// Cross-platform signal installation for safe handling
void installPlatformSpecificHandlers() {
#if defined(_WIN32) || defined(_WIN64)
    // Windows-specific signal handling
    SignalHandlerRegistry::getInstance().setStandardCrashHandlerSignals(
        [](int signal) {
            LOG_F(ERROR, "Caught signal {} on Windows", signal);
        });
#else
    // POSIX (Linux, macOS) specific signals
    SignalHandlerRegistry::getInstance().setStandardCrashHandlerSignals(
        [](int signal) {
            LOG_F(ERROR, "Caught signal {} on POSIX system", signal);
        });
#endif
}
