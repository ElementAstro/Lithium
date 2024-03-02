/*
 * iocp.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-10

Description: IOCP poller for Windows

**************************************************/

#include "iopoll.hpp"
#include "utils/kmtrace.hpp"
#include "utils/utils.hpp"

ATOM_NS_BEGIN

class IocpPoll : public IOPoll {
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

IocpPoll::IocpPoll() {}

IocpPoll::~IocpPoll() {
    if (hCompPort_) {
        CloseHandle(hCompPort_);
        hCompPort_ = nullptr;
    }
}

bool IocpPoll::init() {
    if (hCompPort_) {
        return true;
    }
    hCompPort_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 1);
    if (!hCompPort_) {
        KM_ERRTRACE("IocpPoll::init, CreateIoCompletionPort failed, err="
                    << GetLastError());
        return false;
    }
    return true;
}

Result IocpPoll::registerFd(SOCKET_FD fd, KMEvent events, IOCallback cb) {
    KM_INFOTRACE("IocpPoll::registerFd, fd=" << fd << ", events=" << events);
    if (CreateIoCompletionPort((HANDLE)fd, hCompPort_, (ULONG_PTR)fd, 0) ==
        NULL) {
        return Result::POLL_ERROR;
    }
    resizePollItems(fd);
    poll_items_[fd].fd = fd;
    poll_items_[fd].cb = std::move(cb);
    return Result::OK;
}

Result IocpPoll::unregisterFd(SOCKET_FD fd) {
    KM_INFOTRACE("IocpPoll::unregisterFd, fd=" << fd);
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

Result IocpPoll::updateFd(SOCKET_FD fd, KMEvent events) {
    return Result::NOT_SUPPORTED;
}

#if 1
Result IocpPoll::wait(uint32_t wait_ms) {
    OVERLAPPED_ENTRY entries[128];
    ULONG count = 0;
    auto success = GetQueuedCompletionStatusEx(
        hCompPort_, entries, ARRAY_SIZE(entries), &count, wait_ms, FALSE);
    if (success) {
        for (ULONG i = 0; i < count; ++i) {
            if (entries[i].lpOverlapped) {
                SOCKET_FD fd = (SOCKET_FD)entries[i].lpCompletionKey;
                if (fd < poll_items_.size()) {
                    IOCallback &cb = poll_items_[fd].cb;
                    size_t io_size = entries[i].dwNumberOfBytesTransferred;
                    if (cb)
                        cb(fd, 0, entries[i].lpOverlapped, io_size);
                }
            }
        }
    } else {
        auto err = ::GetLastError();
        if (err != WAIT_TIMEOUT) {
            KM_ERRTRACE("IocpPoll::wait, err=" << err);
        }
    }
    return Result::OK;
}
#else
Result IocpPoll::wait(uint32_t wait_ms) {
    DWORD bytes;
    ULONG_PTR key;
    OVERLAPPED *pOverlapped = NULL;
    auto success = GetQueuedCompletionStatus(hCompPort_, &bytes, &key,
                                             &pOverlapped, wait_ms);
    if (success) {
        SOCKET_FD fd = (SOCKET_FD)key;
        if (fd < poll_items_.size()) {
            IOCallback &cb = poll_items_[fd].cb;
            size_t io_size = bytes;
            if (cb)
                cb(fd, 0, pOverlapped, io_size);
        }
    } else {
        auto err = ::GetLastError();
        if (NULL == pOverlapped) {  // GetQueuedCompletionStatus() failed
            if (err != WAIT_TIMEOUT) {
                KM_ERRTRACE("IocpPoll::wait, err=" << err);
            }
        } else {  // async IO failed
            if (key) {
                SOCKET_FD fd = (SOCKET_FD)key;
            }
        }
    }
    return Result::OK;
}
#endif

void IocpPoll::notify() {
    if (hCompPort_ != nullptr) {
        PostQueuedCompletionStatus(hCompPort_, 0, 0, NULL);
    }
}

IOPoll *createIocpPoll() { return new IocpPoll(); }

ATOM_NS_END
