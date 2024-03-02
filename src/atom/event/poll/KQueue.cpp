/*
 * kqueue.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-10

Description: kqueue poller

**************************************************/

#include "iopoll.hpp"
#include "notifier.hpp"
#include "utils/kmtrace.hpp"

#include <sys/event.h>
#include <sys/time.h>
#include <sys/types.h>


ATOM_NS_BEGIN

#define MAX_EVENT_NUM 256

class KQueue : public IOPoll {
public:
    KQueue();
    ~KQueue();

    bool init() override;
    Result registerFd(SOCKET_FD fd, KMEvent events, IOCallback cb) override;
    Result unregisterFd(SOCKET_FD fd) override;
    Result updateFd(SOCKET_FD fd, KMEvent events) override;
    Result wait(uint32_t wait_time_ms) override;
    void notify() override;
    PollType getType() const override { return PollType::KQUEUE; }

    // can be false on ET mode, but return true to removing the write event
    // and thus reduce the kqueue eventlist size
    bool isLevelTriggered() const override { return true; }

private:
    int kqueue_fd_{-1};
    NotifierPtr notifier_;

    // on ET mode (EV_CLEAR is set), it seems EVFILT_READ won't be triggered
    // if EVFILT_READ is set after data arrived
    bool work_on_et_mode_{false};
};

KQueue::KQueue() {}

KQueue::~KQueue() {
    if (INVALID_FD != kqueue_fd_) {
        ::close(kqueue_fd_);
        kqueue_fd_ = INVALID_FD;
    }
}

bool KQueue::init() {
    if (INVALID_FD != kqueue_fd_) {
        return true;
    }
    kqueue_fd_ = ::kqueue();
    if (INVALID_FD == kqueue_fd_) {
        return false;
    }
#if defined(EVFILT_USER) && defined(NOTE_TRIGGER)
    struct kevent ev;
    EV_SET(&ev, 0, EVFILT_USER, EV_ADD | EV_CLEAR, 0, 0, 0);
    if (::kevent(kqueue_fd_, &ev, 1, 0, 0, 0) != -1) {
        notifier_.reset();
    } else
#endif
    {
        notifier_ = Notifier::createNotifier();
    }
    if (notifier_ && !notifier_->ready()) {
        if (!notifier_->init()) {
            ::close(kqueue_fd_);
            kqueue_fd_ = INVALID_FD;
            return false;
        }
        IOCallback cb([this](SOCKET_FD, KMEvent ev, void *, size_t) {
            notifier_->onEvent(ev);
        });
        registerFd(notifier_->getReadFD(), kEventRead | kEventError,
                   std::move(cb));
    }
    return true;
}

Result KQueue::registerFd(SOCKET_FD fd, KMEvent events, IOCallback cb) {
    if (fd < 0) {
        return Result::INVALID_PARAM;
    }
    resizePollItems(fd);
    poll_items_[fd].fd = fd;
    poll_items_[fd].cb = std::move(cb);
    auto ret = updateFd(fd, events);
    if (ret != Result::OK) {
        poll_items_[fd].reset();
    }
    KM_INFOTRACE("KQueue::registerFd, fd=" << fd << ", ev=" << events
                                           << ", ret=" << (int)ret);
    return ret;
}

Result KQueue::unregisterFd(SOCKET_FD fd) {
    int max_fd = int(poll_items_.size() - 1);
    KM_INFOTRACE("KQueue::unregisterFd, fd=" << fd << ", max_fd=" << max_fd);
    if (fd < 0 || fd > max_fd) {
        KM_WARNTRACE("KQueue::unregisterFd, failed, max_fd=" << max_fd);
        return Result::INVALID_PARAM;
    }
    struct kevent kevents[2];
    int nchanges = 0;
    if (poll_items_[fd].events & kEventRead) {
        EV_SET(&kevents[nchanges++], fd, EVFILT_READ, EV_DELETE, 0, 0, 0);
    }
    if (poll_items_[fd].events & kEventWrite) {
        EV_SET(&kevents[nchanges++], fd, EVFILT_WRITE, EV_DELETE, 0, 0, 0);
    }
    ::kevent(kqueue_fd_, kevents, nchanges, 0, 0, 0);
    if (fd < max_fd) {
        poll_items_[fd].reset();
    } else if (fd == max_fd) {
        poll_items_.pop_back();
    }
    return Result::OK;
}

Result KQueue::updateFd(SOCKET_FD fd, KMEvent events) {
    if (fd < 0 || fd >= poll_items_.size() ||
        INVALID_FD == poll_items_[fd].fd) {
        return Result::INVALID_PARAM;
    }

    struct kevent kevents[2];
    int nchanges = 0;
    if (!!(poll_items_[fd].events & kEventRead) && !(events & kEventRead)) {
        EV_SET(&kevents[nchanges++], fd, EVFILT_READ, EV_DELETE, 0, 0, 0);
        poll_items_[fd].events &= ~kEventRead;
    }
    if (!!(poll_items_[fd].events & kEventWrite) && !(events & kEventWrite)) {
        EV_SET(&kevents[nchanges++], fd, EVFILT_WRITE, EV_DELETE, 0, 0, 0);
        poll_items_[fd].events &= ~kEventWrite;
    }
    if (nchanges) {  // remove events
        ::kevent(kqueue_fd_, kevents, nchanges, 0, 0, 0);
    }
    if (poll_items_[fd].events == events) {
        return Result::OK;
    }
    nchanges = 0;
    unsigned short op = EV_ADD;
    if (work_on_et_mode_) {
        op |= EV_CLEAR;
    }
    if (events & kEventRead) {
        EV_SET(&kevents[nchanges++], fd, EVFILT_READ, op, 0, 0, 0);
    }
    if (events & kEventWrite) {
        EV_SET(&kevents[nchanges++], fd, EVFILT_WRITE, op, 0, 0, 0);
    }
    if (nchanges && ::kevent(kqueue_fd_, kevents, nchanges, 0, 0, 0) == -1) {
        KM_ERRTRACE("KQueue::updateFd error, fd=" << fd << ", errno=" << errno);
        return Result::FAILED;
    }
    poll_items_[fd].events = events;
    // KM_INFOTRACE("KQueue::updateFd, fd="<<fd<<", ev="<<events);
    return Result::OK;
}

Result KQueue::wait(uint32_t wait_ms) {
    timespec tval = {0, 0};
    if (wait_ms != -1) {
        tval.tv_sec = wait_ms / 1000;
        tval.tv_nsec = (wait_ms - tval.tv_sec * 1000) * 1000 * 1000;
    }
    struct kevent kevents[MAX_EVENT_NUM];
    int nevents = kevent(kqueue_fd_, 0, 0, kevents, MAX_EVENT_NUM,
                         wait_ms == -1 ? NULL : &tval);
    if (nevents < 0) {
        if (errno != EINTR) {
            KM_ERRTRACE("KQueue::wait, errno=" << errno);
        }
        KM_INFOTRACE("KQueue::wait, nevents=" << nevents
                                              << ", errno=" << errno);
    } else {
        std::pair<SOCKET_FD, size_t> fds[MAX_EVENT_NUM];
        int nfds = 0;
        int max_fd = int(poll_items_.size() - 1);
        for (int i = 0; i < nevents; ++i) {
            SOCKET_FD fd = (SOCKET_FD)kevents[i].ident;
            if (fd >= 0 && fd <= max_fd) {
                KMEvent revents = 0;
                size_t io_size = 0;
                if (kevents[i].filter == EVFILT_READ) {
                    revents |= kEventRead;
                    io_size = kevents[i].data;
                } else if (kevents[i].filter == EVFILT_WRITE) {
                    revents |= kEventWrite;
                    io_size = kevents[i].data;
                }
#if defined(EVFILT_USER)
                else if (kevents[i].filter == EVFILT_USER) {
                    continue;
                }
#endif
                if (kevents[i].flags & EV_ERROR) {
                    revents |= kEventError;
                }
                if (!revents) {
                    continue;
                }
                if (poll_items_[fd].revents == 0) {
                    fds[nfds++] = {fd, io_size};
                }
                poll_items_[fd].revents = revents;
            }
        }
        for (int i = 0; i < nfds; ++i) {
            SOCKET_FD fd = fds[i].first;
            if (fd < poll_items_.size()) {
                uint32_t revents = poll_items_[fd].revents;
                poll_items_[fd].revents = 0;
                // in case a processed event may modify this event
                revents &= poll_items_[fd].events;
                if (revents) {
                    auto &cb = poll_items_[fd].cb;
                    if (cb)
                        cb(fd, revents, nullptr, fds[i].second);
                }
            }
        }
    }
    return Result::OK;
}

void KQueue::notify() {
    if (notifier_) {
        notifier_->notify();
    } else {
#if defined(EVFILT_USER) && defined(NOTE_TRIGGER)
        struct kevent ev;
        EV_SET(&ev, 0, EVFILT_USER, 0, NOTE_TRIGGER, 0, 0);
        while (::kevent(kqueue_fd_, &ev, 1, 0, 0, 0) == -1 && errno == EINTR)
            ;
#endif
    }
}

IOPoll *createKQueue() { return new KQueue(); }

ATOM_NS_END
