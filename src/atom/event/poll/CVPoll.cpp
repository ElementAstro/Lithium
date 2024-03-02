/*
 * cvpoll.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-10

Description: cvpoll

**************************************************/

#include "iopoll.hpp"

#include <chrono>
#include <condition_variable>

using namespace std::chrono;

ATOM_NS_BEGIN

class CVPoll : public IOPoll {
public:
    bool init() override;
    Result registerFd(SOCKET_FD fd, KMEvent events, IOCallback cb) override {
        return Result::NOT_SUPPORTED;
    }
    Result unregisterFd(SOCKET_FD fd) override { return Result::NOT_SUPPORTED; }
    Result updateFd(SOCKET_FD fd, KMEvent events) override {
        return Result::NOT_SUPPORTED;
    }
    Result wait(uint32_t wait_ms) override;
    void notify() override;
    PollType getType() const override { return PollType::STLCV; }
    bool isLevelTriggered() const override { return false; }

private:
    bool ready_{false};
    std::mutex mutex_;
    std::condition_variable cv_;
};

bool CVPoll::init() {
    {
        std::lock_guard<std::mutex> g(mutex_);
        ready_ = false;
    }
    return true;
}

Result CVPoll::wait(uint32_t wait_ms) {
    auto ms = milliseconds(wait_ms);
    {
        std::unique_lock<std::mutex> lk(mutex_);
        // bool ret = timeBeginPeriod(1) == TIMERR_NOERROR;
        if (cv_.wait_for(lk, ms, [this] { return ready_; })) {
            ready_ = false;
        }
        // if (ret) timeEndPeriod(1);
    }
    return Result::OK;
}

void CVPoll::notify() {
    {
        std::lock_guard<std::mutex> g(mutex_);
        ready_ = true;
    }
    cv_.notify_one();
}

IOPoll *createCVPoll() { return new CVPoll(); }

ATOM_NS_END
