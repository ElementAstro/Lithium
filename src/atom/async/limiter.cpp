#include "limiter.hpp"

#include "atom/log/loguru.hpp"

namespace atom::async {
RateLimiter::Settings::Settings(size_t max_requests,
                                std::chrono::seconds time_window)
    : maxRequests(max_requests), timeWindow(time_window) {
    LOG_F(INFO, "Settings created: max_requests=%zu, time_window=%lld seconds",
          max_requests, time_window.count());
}

// Implementation of RateLimiter constructor
RateLimiter::RateLimiter() { LOG_F(INFO, "RateLimiter created"); }

// Implementation of Awaiter constructor
RateLimiter::Awaiter::Awaiter(RateLimiter& limiter,
                              const std::string& function_name)
    : limiter_(limiter), function_name_(function_name) {
    LOG_F(INFO, "Awaiter created for function: %s", function_name.c_str());
}

// Implementation of Awaiter::await_ready
auto RateLimiter::Awaiter::await_ready() -> bool {
    LOG_F(INFO, "Awaiter::await_ready called for function: %s",
          function_name_.c_str());
    return false;
}

// Implementation of Awaiter::await_suspend
void RateLimiter::Awaiter::await_suspend(std::coroutine_handle<> handle) {
    LOG_F(INFO, "Awaiter::await_suspend called for function: %s",
          function_name_.c_str());
    std::unique_lock<std::mutex> lock(limiter_.mutex_);
    auto& settings = limiter_.settings_[function_name_];
    limiter_.cleanup(function_name_, settings.timeWindow);
    if (limiter_.paused_ ||
        limiter_.requests_[function_name_].size() >= settings.maxRequests) {
        limiter_.waiters_[function_name_].emplace_back(handle);
        limiter_.rejected_requests_[function_name_]++;
        LOG_F(WARNING, "Request for function %s rejected. Total rejected: %zu",
              function_name_.c_str(),
              limiter_.rejected_requests_[function_name_]);
    } else {
        limiter_.requests_[function_name_].emplace_back(
            std::chrono::steady_clock::now());
        lock.unlock();
        LOG_F(INFO, "Request for function %s accepted", function_name_.c_str());
        handle.resume();
    }
}

// Implementation of Awaiter::await_resume
void RateLimiter::Awaiter::await_resume() {
    LOG_F(INFO, "Awaiter::await_resume called for function: %s",
          function_name_.c_str());
}

// Implementation of RateLimiter::acquire
RateLimiter::Awaiter RateLimiter::acquire(const std::string& function_name) {
    LOG_F(INFO, "RateLimiter::acquire called for function: %s",
          function_name.c_str());
    return Awaiter(*this, function_name);
}

// Implementation of RateLimiter::setFunctionLimit
void RateLimiter::setFunctionLimit(const std::string& function_name,
                                   size_t max_requests,
                                   std::chrono::seconds time_window) {
    LOG_F(INFO,
          "RateLimiter::setFunctionLimit called for function: %s, "
          "max_requests=%zu, time_window=%lld seconds",
          function_name.c_str(), max_requests, time_window.count());
    std::unique_lock<std::mutex> lock(mutex_);
    settings_[function_name] = Settings(max_requests, time_window);
}

// Implementation of RateLimiter::pause
void RateLimiter::pause() {
    LOG_F(INFO, "RateLimiter::pause called");
    std::unique_lock<std::mutex> lock(mutex_);
    paused_ = true;
}

// Implementation of RateLimiter::resume
void RateLimiter::resume() {
    LOG_F(INFO, "RateLimiter::resume called");
    std::unique_lock<std::mutex> lock(mutex_);
    paused_ = false;
    processWaiters();
}

// Implementation of RateLimiter::printLog
void RateLimiter::printLog() {
#if ENABLE_DEBUG
    LOG_F(INFO, "RateLimiter::printLog called");
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

// Implementation of RateLimiter::getRejectedRequests
auto RateLimiter::getRejectedRequests(const std::string& function_name)
    -> size_t {
    LOG_F(INFO, "RateLimiter::getRejectedRequests called for function: %s",
          function_name.c_str());
    std::unique_lock<std::mutex> lock(mutex_);
    return rejected_requests_[function_name];
}

// Implementation of RateLimiter::cleanup
void RateLimiter::cleanup(const std::string& function_name,
                          const std::chrono::seconds& time_window) {
    LOG_F(INFO,
          "RateLimiter::cleanup called for function: %s, time_window=%lld "
          "seconds",
          function_name.c_str(), time_window.count());
    auto now = std::chrono::steady_clock::now();
    auto& reqs = requests_[function_name];
    while (!reqs.empty() && now - reqs.front() > time_window) {
        reqs.pop_front();
    }
}

// Implementation of RateLimiter::processWaiters
void RateLimiter::processWaiters() {
    LOG_F(INFO, "RateLimiter::processWaiters called");
    for (auto& [function_name, wait_queue] : waiters_) {
        auto& settings = settings_[function_name];
        while (!wait_queue.empty() &&
               requests_[function_name].size() < settings.maxRequests) {
            auto waiter = wait_queue.front();
            wait_queue.pop_front();
            requests_[function_name].emplace_back(
                std::chrono::steady_clock::now());
            mutex_.unlock();
            LOG_F(INFO, "Resuming waiter for function: %s",
                  function_name.c_str());
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
      maxWait_(maxWait) {
    LOG_F(INFO, "Debounce created: delay=%lld ms, leading=%d, maxWait=%lld ms",
          delay.count(), leading, maxWait ? maxWait->count() : 0);
}

void Debounce::operator()() {
    LOG_F(INFO, "Debounce operator() called");
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
    LOG_F(INFO, "Debounce::cancel called");
    std::unique_lock lock(mutex_);
    scheduled_ = false;
    last_call_.reset();
}

void Debounce::flush() {
    LOG_F(INFO, "Debounce::flush called");
    std::unique_lock lock(mutex_);
    if (scheduled_) {
        func_();
        ++call_count_;
        scheduled_ = false;
    }
}

void Debounce::reset() {
    LOG_F(INFO, "Debounce::reset called");
    std::unique_lock lock(mutex_);
    last_call_.reset();
    scheduled_ = false;
}

size_t Debounce::callCount() const {
    std::unique_lock lock(mutex_);
    return call_count_;
}

void Debounce::run() {
    LOG_F(INFO, "Debounce::run started");
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
            LOG_F(INFO, "Debounce::run finished");
            return;
        }
        if (maxWait_ && now - last_call_.value() >= maxWait_) {
            if (scheduled_) {
                func_();
                ++call_count_;
                scheduled_ = false;
            }
            LOG_F(INFO, "Debounce::run finished");
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
      maxWait_(maxWait) {
    LOG_F(INFO,
          "Throttle created: interval=%lld ms, leading=%d, maxWait=%lld ms",
          interval.count(), leading, maxWait ? maxWait->count() : 0);
}

void Throttle::operator()() {
    LOG_F(INFO, "Throttle operator() called");
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
    LOG_F(INFO, "Throttle::cancel called");
    std::unique_lock lock(mutex_);
    called_ = false;
}

void Throttle::reset() {
    LOG_F(INFO, "Throttle::reset called");
    std::unique_lock lock(mutex_);
    last_call_ = std::chrono::steady_clock::now() - interval_;
    called_ = false;
}

auto Throttle::callCount() const -> size_t {
    std::unique_lock lock(mutex_);
    return call_count_;
}

}  // namespace atom::async