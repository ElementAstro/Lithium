#ifndef ATOM_ALGORITHM_SNOWFLAKE_HPP
#define ATOM_ALGORITHM_SNOWFLAKE_HPP

#include <atomic>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <random>
#include <stdexcept>

namespace atom::algorithm {
class SnowflakeNonLock {
public:
    void lock() {}
    void unlock() {}
};

template <uint64_t Twepoch, typename Lock = SnowflakeNonLock>
class Snowflake {
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

    using time_point = std::chrono::time_point<std::chrono::steady_clock>;

    time_point start_time_point_ = std::chrono::steady_clock::now();
    uint64_t start_millisecond_ =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count();

    std::atomic<uint64_t> last_timestamp_{0};
    uint64_t workerid_ = 0;
    uint64_t datacenterid_ = 0;
    uint64_t sequence_ = 0;
    lock_type lock_;

    uint64_t secret_key_;

public:
    Snowflake() {
        std::random_device rd;
        std::mt19937_64 eng(rd());
        std::uniform_int_distribution<uint64_t> distr;
        secret_key_ = distr(eng);
    }

    Snowflake(const Snowflake &) = delete;
    auto operator=(const Snowflake &) -> Snowflake & = delete;

    void init(uint64_t worker_id, uint64_t datacenter_id) {
        if (worker_id > MAX_WORKER_ID) {
            throw std::runtime_error("worker Id can't be greater than 31");
        }
        if (datacenter_id > MAX_DATACENTER_ID) {
            throw std::runtime_error("datacenter Id can't be greater than 31");
        }
        workerid_ = worker_id;
        datacenterid_ = datacenter_id;
    }

    [[nodiscard]] auto nextid() -> uint64_t {
        std::lock_guard<lock_type> lock(lock_);
        auto timestamp = millisecond();
        if (last_timestamp_.load() == timestamp) {
            sequence_ = (sequence_ + 1) & SEQUENCE_MASK;
            if (sequence_ == 0) {
                timestamp = waitNextMillis(last_timestamp_.load());
            }
        } else {
            sequence_ = 0;
        }

        last_timestamp_.store(timestamp);

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

private:
    [[nodiscard]] auto millisecond() const noexcept -> uint64_t {
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start_time_point_);
        return start_millisecond_ + diff.count();
    }

    [[nodiscard]] auto waitNextMillis(uint64_t last) const noexcept
        -> uint64_t {
        auto timestamp = millisecond();
        while (timestamp <= last) {
            timestamp = millisecond();
        }
        return timestamp;
    }
};
}  // namespace atom::algorithm

#endif  // ATOM_ALGORITHM_SNOWFLAKE_HPP
