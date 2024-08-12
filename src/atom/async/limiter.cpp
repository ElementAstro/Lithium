#include "limiter.hpp"

namespace atom::async {
RateLimiter::Settings::Settings(size_t max_requests,
                                std::chrono::seconds time_window)
    : maxRequests(max_requests), timeWindow(time_window) {}

// Implementation of RateLimiter constructor
RateLimiter::RateLimiter() {}

// Implementation of Awaiter constructor
RateLimiter::Awaiter::Awaiter(RateLimiter& limiter,
                              const std::string& function_name)
    : limiter_(limiter), function_name_(function_name) {}

// Implementation of Awaiter::await_ready
auto RateLimiter::Awaiter::await_ready() -> bool { return false; }

// Implementation of Awaiter::await_suspend
void RateLimiter::Awaiter::await_suspend(std::coroutine_handle<> handle) {
    std::unique_lock<std::mutex> lock(limiter_.mutex_);
    auto& settings = limiter_.settings_[function_name_];
    limiter_.cleanup(function_name_, settings.timeWindow);
    if (limiter_.paused_ ||
        limiter_.requests_[function_name_].size() >= settings.maxRequests) {
        limiter_.waiters_[function_name_].emplace_back(handle);
        limiter_.rejected_requests_[function_name_]++;
    } else {
        limiter_.requests_[function_name_].emplace_back(
            std::chrono::steady_clock::now());
        lock.unlock();
        handle.resume();
    }
}

// Implementation of Awaiter::await_resume
void RateLimiter::Awaiter::await_resume() {}

// Implementation of RateLimiter::acquire
RateLimiter::Awaiter RateLimiter::acquire(const std::string& function_name) {
    return Awaiter(*this, function_name);
}

// Implementation of RateLimiter::setFunctionLimit
void RateLimiter::setFunctionLimit(const std::string& function_name,
                                   size_t max_requests,
                                   std::chrono::seconds time_window) {
    std::unique_lock<std::mutex> lock(mutex_);
    settings_[function_name] = Settings(max_requests, time_window);
}

// Implementation of RateLimiter::pause
void RateLimiter::pause() {
    std::unique_lock<std::mutex> lock(mutex_);
    paused_ = true;
}

// Implementation of RateLimiter::resume
void RateLimiter::resume() {
    std::unique_lock<std::mutex> lock(mutex_);
    paused_ = false;
    process_waiters();
}

// Implementation of RateLimiter::printLog
void RateLimiter::printLog() {
#if ENABLE_DEBUG
    std::unique_lock<std::mutex> lock(mutex_);
    for (const auto& [function_name, timestamps] : log_) {
        std::cout << "Request log for " << function_name << ":\n";
        for (const auto& timestamp : timestamps) {
            std::cout << "Request at " << timestamp.time_since_epoch().count()
                      << std::endl;
        }
    }
#endif
}

// Implementation of RateLimiter::get_rejected_requests
size_t RateLimiter::get_rejected_requests(const std::string& function_name) {
    std::unique_lock<std::mutex> lock(mutex_);
    return rejected_requests_[function_name];
}

// Implementation of RateLimiter::cleanup
void RateLimiter::cleanup(const std::string& function_name,
                          const std::chrono::seconds& time_window) {
    auto now = std::chrono::steady_clock::now();
    auto& reqs = requests_[function_name];
    while (!reqs.empty() && now - reqs.front() > time_window) {
        reqs.pop_front();
    }
}

// Implementation of RateLimiter::process_waiters
void RateLimiter::process_waiters() {
    for (auto& [function_name, wait_queue] : waiters_) {
        auto& settings = settings_[function_name];
        while (!wait_queue.empty() &&
               requests_[function_name].size() < settings.maxRequests) {
            auto waiter = wait_queue.front();
            wait_queue.pop_front();
            requests_[function_name].emplace_back(
                std::chrono::steady_clock::now());
            mutex_.unlock();
            waiter.resume();
            mutex_.lock();
        }
    }
}

Debounce::Debounce(std::function<void()> func, std::chrono::milliseconds delay,
                   bool leading,
                   std::optional<std::chrono::milliseconds> maxWait)
    : func_(std::move(func)),
      delay_(delay),
      leading_(leading),
      maxWait_(maxWait) {}

void Debounce::operator()() {
    auto now = std::chrono::steady_clock::now();
    std::unique_lock lock(mutex_);

    if (leading_ && !scheduled_) {
        scheduled_ = true;
        func_();
        ++call_count_;
    }

    last_call_ = now;
    if (!thread_.joinable()) {
        thread_ = std::jthread([this]() { this->run(); });
    }
}

void Debounce::cancel() {
    std::unique_lock lock(mutex_);
    scheduled_ = false;
    last_call_.reset();
}

void Debounce::flush() {
    std::unique_lock lock(mutex_);
    if (scheduled_) {
        func_();
        ++call_count_;
        scheduled_ = false;
    }
}

void Debounce::reset() {
    std::unique_lock lock(mutex_);
    last_call_.reset();
    scheduled_ = false;
}

size_t Debounce::callCount() const {
    std::unique_lock lock(mutex_);
    return call_count_;
}

void Debounce::run() {
    while (true) {
        std::this_thread::sleep_for(delay_);
        std::unique_lock lock(mutex_);
        auto now = std::chrono::steady_clock::now();
        if (last_call_ && now - last_call_.value() >= delay_) {
            if (scheduled_) {
                func_();
                ++call_count_;
                scheduled_ = false;
            }
            return;
        }
        if (maxWait_ && now - last_call_.value() >= maxWait_) {
            if (scheduled_) {
                func_();
                ++call_count_;
                scheduled_ = false;
            }
            return;
        }
    }
}

Throttle::Throttle(std::function<void()> func,
                   std::chrono::milliseconds interval, bool leading,
                   std::optional<std::chrono::milliseconds> maxWait)
    : func_(std::move(func)),
      interval_(interval),
      last_call_(std::chrono::steady_clock::now() - interval),
      leading_(leading),
      maxWait_(maxWait) {}

void Throttle::operator()() {
    auto now = std::chrono::steady_clock::now();
    std::unique_lock lock(mutex_);

    if (leading_ && !called_) {
        called_ = true;
        func_();
        last_call_ = now;
        ++call_count_;
        return;
    }

    if (now - last_call_ >= interval_) {
        last_call_ = now;
        func_();
        ++call_count_;
    } else if (maxWait_ && (now - last_call_ >= maxWait_)) {
        last_call_ = now;
        func_();
        ++call_count_;
    }
}

void Throttle::cancel() {
    std::unique_lock lock(mutex_);
    called_ = false;
}

void Throttle::reset() {
    std::unique_lock lock(mutex_);
    last_call_ = std::chrono::steady_clock::now() - interval_;
    called_ = false;
}

auto Throttle::callCount() const -> size_t {
    std::unique_lock lock(mutex_);
    return call_count_;
}

}  // namespace atom::async
