/*
 * selectpoll.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-10

Description: Select poll implementation

*************************************************/

#include "iopoll.hpp"
#include "notifier.hpp"
#include "../utils/kmtrace.hpp"

#include <algorithm>

ATOM_NS_BEGIN

class SelectPoll : public IOPoll
{
public:
    SelectPoll();
    ~SelectPoll();

    bool init() override;
    Result registerFd(SOCKET_FD fd, KMEvent events, IOCallback cb) override;
    Result unregisterFd(SOCKET_FD fd) override;
    Result updateFd(SOCKET_FD fd, KMEvent events) override;
    Result wait(uint32_t wait_time_ms) override;
    void notify() override;
    PollType getType() const override { return PollType::SELECT; }
    bool isLevelTriggered() const override { return true; }

private:
    struct PollFD
    {
        SOCKET_FD fd = INVALID_FD;
        KMEvent events = 0;
    };

private:
    using PollFdVector = std::vector<PollFD>;
    NotifierPtr notifier_{Notifier::createNotifier()};
    PollFdVector poll_fds_;

    fd_set read_fds_;
    fd_set write_fds_;
    fd_set except_fds_;
    SOCKET_FD max_fd_;

private:
    void updateFdSet(SOCKET_FD fd, KMEvent events);
};

SelectPoll::SelectPoll()
    : max_fd_(0)
{
    FD_ZERO(&read_fds_);
    FD_ZERO(&write_fds_);
    FD_ZERO(&except_fds_);
}

SelectPoll::~SelectPoll()
{
    poll_fds_.clear();
}

bool SelectPoll::init()
{
    if (!notifier_->ready())
    {
        if (!notifier_->init())
        {
            return false;
        }
        IOCallback cb([this](SOCKET_FD, KMEvent ev, void *, size_t)
                      { notifier_->onEvent(ev); });
        registerFd(notifier_->getReadFD(), kEventRead | kEventError, std::move(cb));
    }
    return true;
}

Result SelectPoll::registerFd(SOCKET_FD fd, KMEvent events, IOCallback cb)
{
    if (fd < 0)
    {
        return Result::INVALID_PARAM;
    }
    KM_INFOTRACE("SelectPoll::registerFd, fd=" << fd);
    resizePollItems(fd);
    if (INVALID_FD == poll_items_[fd].fd || -1 == poll_items_[fd].idx)
    {
        PollFD pfd;
        pfd.fd = fd;
        pfd.events = events;
        poll_fds_.push_back(pfd);
        poll_items_[fd].idx = int(poll_fds_.size() - 1);
    }
    poll_items_[fd].fd = fd;
    poll_items_[fd].events = events;
    poll_items_[fd].cb = std::move(cb);
    updateFdSet(fd, events);
    return Result::OK;
}

Result SelectPoll::unregisterFd(SOCKET_FD fd)
{
    auto max_fd = SOCKET_FD(poll_items_.size() - 1);
    KM_INFOTRACE("SelectPoll::unregisterFd, fd=" << fd << ", max_fd=" << max_fd);
    if (fd < 0 || fd > max_fd)
    {
        KM_WARNTRACE("SelectPoll::unregisterFd, failed, max_fd=" << max_fd);
        return Result::INVALID_PARAM;
    }
    updateFdSet(fd, 0);
    int idx = poll_items_[fd].idx;
    if (fd < max_fd)
    {
        poll_items_[fd].reset();
    }
    else if (fd == max_fd)
    {
        poll_items_.pop_back();
    }
    int last_idx = int(poll_fds_.size() - 1);
    if (idx > last_idx || -1 == idx)
    {
        return Result::OK;
    }
    if (idx != last_idx)
    {
        std::iter_swap(poll_fds_.begin() + idx, poll_fds_.end() - 1);
        poll_items_[poll_fds_[idx].fd].idx = idx;
    }
    poll_fds_.pop_back();
    return Result::OK;
}

Result SelectPoll::updateFd(SOCKET_FD fd, KMEvent events)
{
    auto max_fd = SOCKET_FD(poll_items_.size() - 1);
    if (fd < 0 || -1 == max_fd || fd > max_fd)
    {
        KM_WARNTRACE("SelectPoll::updateFd, failed, fd=" << fd << ", max_fd=" << max_fd);
        return Result::INVALID_PARAM;
    }
    if (poll_items_[fd].fd != fd)
    {
        KM_WARNTRACE("SelectPoll::updateFd, failed, fd=" << fd << ", item_fd=" << poll_items_[fd].fd);
        return Result::INVALID_PARAM;
    }
    int idx = poll_items_[fd].idx;
    if (idx < 0 || idx >= (int)poll_fds_.size())
    {
        KM_WARNTRACE("SelectPoll::updateFd, failed, index=" << idx);
        return Result::INVALID_STATE;
    }
    if (poll_fds_[idx].fd != fd)
    {
        KM_WARNTRACE("SelectPoll::updateFd, failed, fd=" << fd << ", pfds_fd=" << poll_fds_[idx].fd);
        return Result::INVALID_PARAM;
    }
    poll_fds_[idx].events = events;
    poll_items_[fd].events = events;
    updateFdSet(fd, events);
    return Result::OK;
}

void SelectPoll::updateFdSet(SOCKET_FD fd, KMEvent events)
{
    if (events != 0)
    {
        if (events & kEventRead)
        {
            FD_SET(fd, &read_fds_);
        }
        else
        {
            FD_CLR(fd, &read_fds_);
        }
        if (events & kEventWrite)
        {
            FD_SET(fd, &write_fds_);
        }
        else
        {
            FD_CLR(fd, &write_fds_);
        }
        if (events & kEventError)
        {
            FD_SET(fd, &except_fds_);
        }
        if (fd > max_fd_)
        {
            max_fd_ = fd;
        }
    }
    else
    {
        FD_CLR(fd, &read_fds_);
        FD_CLR(fd, &write_fds_);
        FD_CLR(fd, &except_fds_);
        if (max_fd_ == fd)
        {
            auto it = std::max_element(poll_fds_.begin(), poll_fds_.end(), [](PollFD &pf1, PollFD &pf2)
                                       { return pf1.fd < pf2.fd; });
            max_fd_ = it != poll_fds_.end() ? (*it).fd : 0;
        }
    }
}

Result SelectPoll::wait(uint32_t wait_ms)
{
    fd_set readfds, writefds, exceptfds;
    memcpy(&readfds, &read_fds_, sizeof(read_fds_));
    memcpy(&writefds, &write_fds_, sizeof(write_fds_));
    memcpy(&exceptfds, &except_fds_, sizeof(except_fds_));
    struct timeval tval
    {
        0, 0
    };
    if (wait_ms != -1)
    {
        tval.tv_sec = wait_ms / 1000;
        tval.tv_usec = (wait_ms - tval.tv_sec * 1000) * 1000;
    }
    int nready = ::select(static_cast<int>(max_fd_ + 1), &readfds, &writefds, &exceptfds, wait_ms == -1 ? nullptr : &tval);
    if (nready <= 0)
    {
        return Result::OK;
    }
    // copy poll fds since event handler may unregister fd
    PollFdVector poll_fds = poll_fds_;
    int fds_count = int(poll_fds.size());
    for (int i = 0; i < fds_count && nready > 0; ++i)
    {
        KMEvent revents = 0;
        SOCKET_FD fd = poll_fds[i].fd;
        if (FD_ISSET(fd, &readfds))
        {
            revents |= kEventRead;
            --nready;
        }
        if (nready > 0 && FD_ISSET(fd, &writefds))
        {
            revents |= kEventWrite;
            --nready;
        }
        if (nready > 0 && FD_ISSET(fd, &exceptfds))
        {
            revents |= kEventError;
            --nready;
        }
        if (fd < poll_items_.size())
        {
            revents &= poll_items_[fd].events;
            if (revents)
            {
                auto &cb = poll_items_[fd].cb;
                if (cb)
                    cb(fd, revents, nullptr, 0);
            }
        }
    }
    return Result::OK;
}

void SelectPoll::notify()
{
    notifier_->notify();
}

IOPoll *createSelectPoll()
{
    return new SelectPoll();
}

ATOM_NS_END
