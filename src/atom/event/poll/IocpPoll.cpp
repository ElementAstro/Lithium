/* Copyright (c) 2014-2017, Fengping Bao <jamol@live.com>
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
#include "utils/kmtrace.h"
#include "utils/utils.h"

KEV_NS_BEGIN

class IocpPoll : public IOPoll
{
public:
    IocpPoll();
    ~IocpPoll();
    
    bool init();
    Result registerFd(SOCKET_FD fd, KMEvent events, IOCallback cb);
    Result unregisterFd(SOCKET_FD fd);
    Result updateFd(SOCKET_FD fd, KMEvent events);
    Result wait(uint32_t wait_ms);
    void notify();
    PollType getType() const { return PollType::IOCP; }
    bool isLevelTriggered() const { return false; }

protected:
    HANDLE hCompPort_ = nullptr;
};

IocpPoll::IocpPoll()
{
    
}

IocpPoll::~IocpPoll()
{
    if (hCompPort_) {
        CloseHandle(hCompPort_);
        hCompPort_ = nullptr;
    }
}

bool IocpPoll::init()
{
    if (hCompPort_) {
        return true;
    }
    hCompPort_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 1);
    if (!hCompPort_) {
        KM_ERRTRACE("IocpPoll::init, CreateIoCompletionPort failed, err=" << GetLastError());
        return false;
    }
    return true;
}

Result IocpPoll::registerFd(SOCKET_FD fd, KMEvent events, IOCallback cb)
{
    KM_INFOTRACE("IocpPoll::registerFd, fd=" << fd << ", events=" << events);
    if (CreateIoCompletionPort((HANDLE)fd, hCompPort_, (ULONG_PTR)fd, 0) == NULL) {
        return Result::POLL_ERROR;
    }
    resizePollItems(fd);
    poll_items_[fd].fd = fd;
    poll_items_[fd].cb = std::move(cb);
    return Result::OK;
}

Result IocpPoll::unregisterFd(SOCKET_FD fd)
{
    KM_INFOTRACE("IocpPoll::unregisterFd, fd="<<fd);
    SOCKET_FD max_fd = poll_items_.size() - 1;
    if (fd < 0 || -1 == max_fd || fd > max_fd) {
        KM_WARNTRACE("IocpPoll::unregisterFd, failed, max_fd=" << max_fd);
        return Result::INVALID_PARAM;
    }
    if (fd == max_fd) {
        poll_items_.pop_back();
    } else {
        poll_items_[fd].cb = nullptr;
        poll_items_[fd].fd = INVALID_FD;
    }
    
    return Result::OK;
}

Result IocpPoll::updateFd(SOCKET_FD fd, KMEvent events)
{
    return Result::NOT_SUPPORTED;
}

#if 1
Result IocpPoll::wait(uint32_t wait_ms)
{
    OVERLAPPED_ENTRY entries[128];
    ULONG count = 0;
    auto success = GetQueuedCompletionStatusEx(hCompPort_, entries, ARRAY_SIZE(entries), &count, wait_ms, FALSE);
    if (success) {
        for (ULONG i = 0; i < count; ++i) {
            if (entries[i].lpOverlapped) {
                SOCKET_FD fd = (SOCKET_FD)entries[i].lpCompletionKey;
                if (fd < poll_items_.size()) {
                    IOCallback &cb = poll_items_[fd].cb;
                    size_t io_size = entries[i].dwNumberOfBytesTransferred;
                    if (cb) cb(fd, 0, entries[i].lpOverlapped, io_size);
                }
            }
        }
    }
    else {
        auto err = ::GetLastError();
        if (err != WAIT_TIMEOUT) {
            KM_ERRTRACE("IocpPoll::wait, err="<<err);
        }
    }
    return Result::OK;
}
#else
Result IocpPoll::wait(uint32_t wait_ms)
{
    DWORD bytes;
    ULONG_PTR key;
    OVERLAPPED *pOverlapped = NULL;
    auto success = GetQueuedCompletionStatus(hCompPort_, &bytes, &key, &pOverlapped, wait_ms);
    if (success) {
        SOCKET_FD fd = (SOCKET_FD)key;
        if (fd < poll_items_.size()) {
            IOCallback &cb = poll_items_[fd].cb;
            size_t io_size = bytes;
            if (cb) cb(fd, 0, pOverlapped, io_size);
        }
    }
    else {
        auto err = ::GetLastError();
        if (NULL == pOverlapped) { // GetQueuedCompletionStatus() failed
            if (err != WAIT_TIMEOUT) {
                KM_ERRTRACE("IocpPoll::wait, err="<<err);
            }
        } else { // async IO failed
            if (key) {
                SOCKET_FD fd = (SOCKET_FD)key;
            }
        }
    }
    return Result::OK;
}
#endif

void IocpPoll::notify()
{
    if (hCompPort_ != nullptr) {
        PostQueuedCompletionStatus(hCompPort_, 0, 0, NULL);
    }
}

IOPoll* createIocpPoll() {
    return new IocpPoll();
}

KEV_NS_END
