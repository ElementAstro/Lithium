/* Copyright (c) 2022, Fengping Bao <jamol@live.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "IOPoll.h"

#include <chrono>
#include <condition_variable>

using namespace std::chrono;

KEV_NS_BEGIN

class CVPoll : public IOPoll
{
public:
    bool init() override;
    Result registerFd(SOCKET_FD fd, KMEvent events, IOCallback cb) override
    {
        return Result::NOT_SUPPORTED;
    }
    Result unregisterFd(SOCKET_FD fd) override
    {
        return Result::NOT_SUPPORTED;
    }
    Result updateFd(SOCKET_FD fd, KMEvent events) override
    {
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

bool CVPoll::init()
{
    {
        std::lock_guard<std::mutex> g(mutex_);
        ready_ = false;
    }
    return true;
}

Result CVPoll::wait(uint32_t wait_ms)
{
    auto ms = milliseconds(wait_ms);
    {
        std::unique_lock<std::mutex> lk(mutex_);
        //bool ret = timeBeginPeriod(1) == TIMERR_NOERROR;
        if (cv_.wait_for(lk, ms, [this] { return ready_; })) {
            ready_ = false;
        }
        //if (ret) timeEndPeriod(1);
    }
    return Result::OK;
}

void CVPoll::notify()
{
    {
        std::lock_guard<std::mutex> g(mutex_);
        ready_ = true;
    }
    cv_.notify_one();
}

IOPoll* createCVPoll() {
    return new CVPoll();
}

KEV_NS_END
