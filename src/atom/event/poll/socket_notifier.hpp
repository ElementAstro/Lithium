/*
 * socket_notifier.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-10

Description: Socket Notifier

*************************************************/

#ifndef ATOM_EVENT_SOCKET_NOTIFIER_HPP
#define ATOM_EVENT_SOCKET_NOTIFIER_HPP

#include "utils/utils.hpp"
#include "utils/defer.hpp"
#include "utils/skutils.hpp"
#include "notifier.hpp"

ATOM_NS_BEGIN

class SocketNotifier : public Notifier
{
public:
    enum
    {
        READ_FD = 0,
        WRITE_FD
    };
    ~SocketNotifier()
    {
        cleanup();
    }
    bool init() override
    {
        cleanup();

        sockaddr_storage ss_addr{0};
        struct addrinfo hints = {0};
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_NUMERICHOST | AI_ADDRCONFIG;
        if (km_set_sock_addr("127.0.0.1", 0, &hints, (struct sockaddr *)&ss_addr, sizeof(ss_addr)) != 0)
        {
            return false;
        }
        auto lfd = ::socket(AF_INET, SOCK_STREAM, 0);
        DEFER(SKUtils::close(lfd));
        if (::bind(lfd, (const sockaddr *)&ss_addr, sizeof(sockaddr_in)) != 0)
        {
            return false;
        }
        socklen_t addr_len = sizeof(ss_addr);
        if (::getsockname(lfd, (struct sockaddr *)&ss_addr, &addr_len) != 0)
        {
            return false;
        }
        char ip[128] = {0};
        uint16_t port = 0;
        km_get_sock_addr((struct sockaddr *)&ss_addr, addr_len, ip, sizeof(ip), &port);
        if (km_set_sock_addr("127.0.0.1", port, &hints, (struct sockaddr *)&ss_addr, sizeof(ss_addr)) != 0)
        {
            return false;
        }
        if (::listen(lfd, 1) != 0)
        {
            return false;
        }

        fds_[WRITE_FD] = ::socket(AF_INET, SOCK_STREAM, 0);
        if (::connect(fds_[WRITE_FD], (const sockaddr *)&ss_addr, sizeof(sockaddr_in)) != 0)
        {
            cleanup();
            return false;
        }
        fds_[READ_FD] = ::accept(lfd, NULL, NULL);
        if (INVALID_FD == fds_[READ_FD])
        {
            cleanup();
            return false;
        }
        set_nonblocking(fds_[READ_FD]);
        set_nonblocking(fds_[WRITE_FD]);
        set_tcpnodelay(fds_[READ_FD]);
        set_tcpnodelay(fds_[WRITE_FD]);
        return true;
    }
    bool ready() override
    {
        return fds_[READ_FD] != INVALID_FD && fds_[WRITE_FD] != INVALID_FD;
    }
    void notify()
    {
        char c = 1;
        SKUtils::send(fds_[WRITE_FD], &c, sizeof(c), 0);
    }

    SOCKET_FD getReadFD() override
    {
        return fds_[READ_FD];
    }

    Result onEvent(KMEvent ev) override
    {
        char buf[1024];
        ssize_t ret = 0;
        do
        {
            ret = SKUtils::recv(fds_[READ_FD], buf, sizeof(buf), 0);
        } while (ret == sizeof(buf));
        return Result::OK;
    }

private:
    void cleanup()
    {
        if (fds_[READ_FD] != INVALID_FD)
        {
            SKUtils::close(fds_[READ_FD]);
            fds_[READ_FD] = INVALID_FD;
        }
        if (fds_[WRITE_FD] != INVALID_FD)
        {
            SKUtils::close(fds_[WRITE_FD]);
            fds_[WRITE_FD] = INVALID_FD;
        }
    }

    SOCKET_FD fds_[2]{INVALID_FD, INVALID_FD};
};

ATOM_NS_END

#endif
