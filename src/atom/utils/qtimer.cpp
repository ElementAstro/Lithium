#include "qtimer.hpp"

namespace atom::utils {
// Constructor
ElapsedTimer::ElapsedTimer() = default;

// Start or restart the timer
void ElapsedTimer::start() { start_time_ = Clock::now(); }

// Invalidate the timer
void ElapsedTimer::invalidate() { start_time_.reset(); }

// Check if the timer has been started and is valid
auto ElapsedTimer::isValid() const -> bool { return start_time_.has_value(); }

// Get elapsed time in nanoseconds
auto ElapsedTimer::elapsedNs() const -> int64_t {
    return isValid() ? std::chrono::duration_cast<Nanoseconds>(
                           Clock::now() - start_time_.value())
                           .count()
                     : 0;
}

// Get elapsed time in microseconds
auto ElapsedTimer::elapsedUs() const -> int64_t {
    return isValid() ? std::chrono::duration_cast<Microseconds>(
                           Clock::now() - start_time_.value())
                           .count()
                     : 0;
}

// Get elapsed time in milliseconds
auto ElapsedTimer::elapsedMs() const -> int64_t {
    return isValid() ? std::chrono::duration_cast<Milliseconds>(
                           Clock::now() - start_time_.value())
                           .count()
                     : 0;
}

// Get elapsed time in seconds
auto ElapsedTimer::elapsedSec() const -> int64_t {
    return isValid() ? std::chrono::duration_cast<Seconds>(Clock::now() -
                                                           start_time_.value())
                           .count()
                     : 0;
}

// Get elapsed time in minutes
auto ElapsedTimer::elapsedMin() const -> int64_t {
    return isValid() ? std::chrono::duration_cast<Minutes>(Clock::now() -
                                                           start_time_.value())
                           .count()
                     : 0;
}

// Get elapsed time in hours
auto ElapsedTimer::elapsedHrs() const -> int64_t {
    return isValid() ? std::chrono::duration_cast<Hours>(Clock::now() -
                                                         start_time_.value())
                           .count()
                     : 0;
}

// Get elapsed time in milliseconds (same as elapsedMs for compatibility)
auto ElapsedTimer::elapsed() const -> int64_t { return elapsedMs(); }

// Check if a specified duration (in milliseconds) has passed
auto ElapsedTimer::hasExpired(int64_t ms) const -> bool {
    return elapsedMs() >= ms;
}

// Get the remaining time until the specified duration (in milliseconds) has
// passed
auto ElapsedTimer::remainingTimeMs(int64_t ms) const -> int64_t {
    int64_t elapsed = elapsedMs();
    return ms > elapsed ? ms - elapsed : 0;
}

// Get the current absolute time in milliseconds since epoch
auto ElapsedTimer::currentTimeMs() -> int64_t {
    return std::chrono::duration_cast<Milliseconds>(
               Clock::now().time_since_epoch())
        .count();
}

// Compare two timers
auto ElapsedTimer::operator<(const ElapsedTimer& other) const -> bool {
    return start_time_ < other.start_time_;
}

auto ElapsedTimer::operator>(const ElapsedTimer& other) const -> bool {
    return start_time_ > other.start_time_;
}

auto ElapsedTimer::operator<=(const ElapsedTimer& other) const -> bool {
    return start_time_ <= other.start_time_;
}

auto ElapsedTimer::operator>=(const ElapsedTimer& other) const -> bool {
    return start_time_ >= other.start_time_;
}

auto ElapsedTimer::operator==(const ElapsedTimer& other) const -> bool {
    return start_time_ == other.start_time_;
}

auto ElapsedTimer::operator!=(const ElapsedTimer& other) const -> bool {
    return start_time_ != other.start_time_;
}

}  // namespace atom::utils
