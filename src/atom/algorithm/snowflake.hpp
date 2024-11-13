#ifndef ATOM_ALGORITHM_SNOWFLAKE_HPP
#define ATOM_ALGORITHM_SNOWFLAKE_HPP

#include <atomic>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <random>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace atom::algorithm {

// Custom exception classes for clearer error handling
class SnowflakeException : public std::runtime_error {
public:
    explicit SnowflakeException(const std::string &message)
        : std::runtime_error(message) {}
};

class InvalidWorkerIdException : public SnowflakeException {
public:
    InvalidWorkerIdException(uint64_t worker_id, uint64_t max)
        : SnowflakeException("Worker ID " + std::to_string(worker_id) +
                             " exceeds maximum of " + std::to_string(max)) {}
};

class InvalidDatacenterIdException : public SnowflakeException {
public:
    InvalidDatacenterIdException(uint64_t datacenter_id, uint64_t max)
        : SnowflakeException("Datacenter ID " + std::to_string(datacenter_id) +
                             " exceeds maximum of " + std::to_string(max)) {}
};

class InvalidTimestampException : public SnowflakeException {
public:
    InvalidTimestampException(uint64_t timestamp)
        : SnowflakeException("Timestamp " + std::to_string(timestamp) +
                             " is invalid or out of range.") {}
};

class SnowflakeNonLock {
public:
    void lock() {}
    void unlock() {}
};

template <uint64_t Twepoch, typename Lock = SnowflakeNonLock>
class Snowflake {
    static_assert(std::is_same_v<Lock, SnowflakeNonLock> ||
                      std::is_same_v<Lock, std::mutex>,
                  "Lock must be SnowflakeNonLock or std::mutex");

public:
    using lock_type = Lock;
    static constexpr uint64_t TWEPOCH = Twepoch;
    static constexpr uint64_t WORKER_ID_BITS = 5;
    static constexpr uint64_t DATACENTER_ID_BITS = 5;
    static constexpr uint64_t MAX_WORKER_ID = (1ULL << WORKER_ID_BITS) - 1;
    static constexpr uint64_t MAX_DATACENTER_ID =
        (1ULL << DATACENTER_ID_BITS) - 1;
    static constexpr uint64_t SEQUENCE_BITS = 12;
    static constexpr uint64_t WORKER_ID_SHIFT = SEQUENCE_BITS;
    static constexpr uint64_t DATACENTER_ID_SHIFT =
        SEQUENCE_BITS + WORKER_ID_BITS;
    static constexpr uint64_t TIMESTAMP_LEFT_SHIFT =
        SEQUENCE_BITS + WORKER_ID_BITS + DATACENTER_ID_BITS;
    static constexpr uint64_t SEQUENCE_MASK = (1ULL << SEQUENCE_BITS) - 1;

    explicit Snowflake(uint64_t worker_id = 0, uint64_t datacenter_id = 0)
        : workerid_(worker_id), datacenterid_(datacenter_id) {
        initialize();
    }

    Snowflake(const Snowflake &) = delete;
    auto operator=(const Snowflake &) -> Snowflake & = delete;

    void init(uint64_t worker_id, uint64_t datacenter_id) {
        std::lock_guard<lock_type> lock(lock_);
        if (worker_id > MAX_WORKER_ID) {
            throw InvalidWorkerIdException(worker_id, MAX_WORKER_ID);
        }
        if (datacenter_id > MAX_DATACENTER_ID) {
            throw InvalidDatacenterIdException(datacenter_id,
                                               MAX_DATACENTER_ID);
        }
        workerid_ = worker_id;
        datacenterid_ = datacenter_id;
    }

    [[nodiscard]] auto nextid() -> uint64_t {
        std::lock_guard<lock_type> lock(lock_);
        uint64_t timestamp = current_millis();
        if (timestamp < last_timestamp_) {
            throw InvalidTimestampException(timestamp);
        }

        if (last_timestamp_ == timestamp) {
            sequence_ = (sequence_ + 1) & SEQUENCE_MASK;
            if (sequence_ == 0) {
                timestamp = wait_next_millis(last_timestamp_);
                if (timestamp < last_timestamp_) {
                    throw InvalidTimestampException(timestamp);
                }
            }
        } else {
            sequence_ = 0;
        }

        last_timestamp_ = timestamp;

        uint64_t id = ((timestamp - TWEPOCH) << TIMESTAMP_LEFT_SHIFT) |
                      (datacenterid_ << DATACENTER_ID_SHIFT) |
                      (workerid_ << WORKER_ID_SHIFT) | sequence_;

        return id ^ secret_key_;
    }

    void parseId(uint64_t encrypted_id, uint64_t &timestamp,
                 uint64_t &datacenter_id, uint64_t &worker_id,
                 uint64_t &sequence) const {
        uint64_t id = encrypted_id ^ secret_key_;

        timestamp = (id >> TIMESTAMP_LEFT_SHIFT) + TWEPOCH;
        datacenter_id = (id >> DATACENTER_ID_SHIFT) & MAX_DATACENTER_ID;
        worker_id = (id >> WORKER_ID_SHIFT) & MAX_WORKER_ID;
        sequence = id & SEQUENCE_MASK;
    }

    // Additional functionality: Reset the Snowflake generator
    void reset() {
        std::lock_guard<lock_type> lock(lock_);
        last_timestamp_ = 0;
        sequence_ = 0;
    }

    // Additional functionality: Retrieve current worker ID
    [[nodiscard]] auto getWorkerId() const -> uint64_t { return workerid_; }

    // Additional functionality: Retrieve current datacenter ID
    [[nodiscard]] auto getDatacenterId() const -> uint64_t {
        return datacenterid_;
    }

private:
    uint64_t workerid_ = 0;
    uint64_t datacenterid_ = 0;
    uint64_t sequence_ = 0;
    mutable lock_type lock_;
    uint64_t secret_key_;

    std::atomic<uint64_t> last_timestamp_{0};
    std::chrono::steady_clock::time_point start_time_point_ =
        std::chrono::steady_clock::now();
    uint64_t start_millisecond_ = get_system_millis();

    void initialize() {
        std::random_device rd;
        std::mt19937_64 eng(rd());
        std::uniform_int_distribution<uint64_t> distr;
        secret_key_ = distr(eng);

        if (workerid_ > MAX_WORKER_ID) {
            throw InvalidWorkerIdException(workerid_, MAX_WORKER_ID);
        }
        if (datacenterid_ > MAX_DATACENTER_ID) {
            throw InvalidDatacenterIdException(datacenterid_,
                                               MAX_DATACENTER_ID);
        }
    }

    [[nodiscard]] auto get_system_millis() const -> uint64_t {
        return static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch())
                .count());
    }

    [[nodiscard]] auto current_millis() const -> uint64_t {
        auto now = std::chrono::steady_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(
                        now - start_time_point_)
                        .count();
        return start_millisecond_ + static_cast<uint64_t>(diff);
    }

    [[nodiscard]] auto wait_next_millis(uint64_t last) const -> uint64_t {
        uint64_t timestamp = current_millis();
        while (timestamp <= last) {
            timestamp = current_millis();
        }
        return timestamp;
    }
};

}  // namespace atom::algorithm

#endif  // ATOM_ALGORITHM_SNOWFLAKE_HPP