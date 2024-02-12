/*
 * event_notifier.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-10

Description: EventNotifier

**************************************************/

#ifndef __EventNotifier_HPP
#define __EventNotifier_HPP

#include "utils/utils.hpp"
#include "notifier.hpp"

#include <errno.h>
#include <sys/eventfd.h>

ATOM_NS_BEGIN

class EventNotifier : public Notifier
{
public:
    ~EventNotifier()
    {
        cleanup();
    }
    bool init() override
    {
        cleanup();
        efd_ = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
        return efd_ >= 0;
    }
    bool ready() override
    {
        return efd_ >= 0;
    }
    void notify() override
    {
        if (efd_ >= 0)
        {
            // eventfd_write(efd_, 1);
            ssize_t ret = 0;
            do
            {
                uint64_t count = 1;
                ret = write(efd_, &count, sizeof(count));
            } while (ret < 0 && errno == EINTR);
        }
    }

    SOCKET_FD getReadFD() override
    {
        return efd_;
    }

    Result onEvent(KMEvent ev) override
    {
        uint64_t count = 0;
        ssize_t ret = 0;
        do
        {
            // eventfd_t val;
            // eventfd_read(efd_, &val);
            ret = read(efd_, &count, sizeof(count));
        } while (ret < 0 && errno == EINTR);
        return Result::OK;
    }

private:
    void cleanup()
    {
        if (efd_ != -1)
        {
            close(efd_);
            efd_ = -1;
        }
    }

    int efd_{-1};
};

ATOM_NS_END

#endif
