#include "signal.hpp"
#include <algorithm>
#include <csignal>

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
    handlers_[signal].insert({handler, priority});
    std::signal(signal, signalDispatcher);
}

void SignalHandlerRegistry::removeSignalHandler(SignalID signal,
                                                const SignalHandler& handler) {
    std::lock_guard lock(mutex_);
    auto it = handlers_.find(signal);
    if (it != handlers_.end()) {
        auto handlerWithPriority =
            std::find_if(it->second.begin(), it->second.end(),
                         [&handler](const SignalHandlerWithPriority& hp) {
                             return hp.handler.target<void(SignalID)>() ==
                                    handler.target<void(SignalID)>();
                         });
        if (handlerWithPriority != it->second.end()) {
            it->second.erase(handlerWithPriority);
        }
        if (it->second.empty()) {
            handlers_.erase(it);
            std::signal(signal, SIG_DFL);
        }
    }
}

void SignalHandlerRegistry::setStandardCrashHandlerSignals(
    const SignalHandler& handler, int priority) {
    for (SignalID sig : getStandardCrashSignals()) {
        setSignalHandler(sig, handler, priority);
    }
}

SignalHandlerRegistry::SignalHandlerRegistry() = default;

SignalHandlerRegistry::~SignalHandlerRegistry() = default;

void SignalHandlerRegistry::signalDispatcher(int signal) {
    SignalHandlerRegistry& registry = getInstance();
    std::lock_guard lock(registry.mutex_);
    auto it = registry.handlers_.find(signal);
    if (it != registry.handlers_.end()) {
        for (const auto& handler : it->second) {
            handler.handler(signal);
        }
    }
}

auto SignalHandlerRegistry::getStandardCrashSignals() -> std::set<SignalID> {
    return {SIGABRT, SIGILL, SIGFPE, SIGSEGV, SIGBUS, SIGQUIT};
}

SafeSignalManager::SafeSignalManager() : keepRunning_(true) {
    signalHandlingThread_ = std::thread([this]() { processSignals(); });
}

SafeSignalManager::~SafeSignalManager() {
    keepRunning_ = false;
    {
        std::lock_guard lock(queueMutex_);
        signalQueue_.clear();  // Clear any remaining signals to avoid deadlock.
    }
    signalHandlingThread_.join();
}

void SafeSignalManager::addSafeSignalHandler(SignalID signal,
                                             const SignalHandler& handler,
                                             int priority) {
    std::lock_guard lock(queueMutex_);
    safeHandlers_[signal].insert({handler, priority});
}

void SafeSignalManager::removeSafeSignalHandler(SignalID signal,
                                                const SignalHandler& handler) {
    std::lock_guard lock(queueMutex_);
    auto it = safeHandlers_.find(signal);
    if (it != safeHandlers_.end()) {
        auto handlerWithPriority =
            std::find_if(it->second.begin(), it->second.end(),
                         [&handler](const SignalHandlerWithPriority& hp) {
                             return hp.handler.target<void(SignalID)>() ==
                                    handler.target<void(SignalID)>();
                         });
        if (handlerWithPriority != it->second.end()) {
            it->second.erase(handlerWithPriority);
        }
        if (it->second.empty()) {
            safeHandlers_.erase(it);
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
            auto it = safeHandlers_.find(signal);
            if (it != safeHandlers_.end()) {
                for (const auto& handler : it->second) {
                    handler.handler(signal);
                }
            }
        }
    }
}
