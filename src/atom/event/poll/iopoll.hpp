/*
 * iopoll.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-10

Description: IOPoll implementation

**************************************************/

#ifndef __IOPoll_HPP
#define __IOPoll_HPP

#include "kevdefs.hpp"

#ifdef ATOM_OS_WIN
#include <Ws2tcpip.h>
#include <time.h>
#include <windows.h>

#elif defined(ATOM_OS_LINUX)
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>


#elif defined(ATOM_OS_MAC)
#include <pthread.h>
#include <string.h>
#include <unistd.h>


#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>


#else
#error "UNSUPPORTED OS"
#endif

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


#include <list>
#include <map>
#include <vector>


ATOM_NS_BEGIN

struct PollItem {
    void reset() {
        fd = INVALID_FD;
        idx = -1;
        events = 0;
        revents = 0;
        cb = nullptr;
    }
    SOCKET_FD fd{INVALID_FD};
    int idx{-1};
    KMEvent events{0};   // kuma events registered
    KMEvent revents{0};  // kuma events received
    IOCallback cb;
};
typedef std::vector<PollItem> PollItemVector;

class IOPoll {
public:
    virtual ~IOPoll() {}

    virtual bool init() = 0;
    virtual Result registerFd(SOCKET_FD fd, KMEvent events, IOCallback cb) = 0;
    virtual Result unregisterFd(SOCKET_FD fd) = 0;
    virtual Result updateFd(SOCKET_FD fd, KMEvent events) = 0;
    virtual Result wait(uint32_t wait_time_ms) = 0;
    virtual void notify() = 0;
    virtual PollType getType() const = 0;
    virtual bool isLevelTriggered() const = 0;

protected:
    void resizePollItems(SOCKET_FD fd) {
        auto count = poll_items_.size();
        if (fd >= count) {
            if (fd > count + 1024) {
                poll_items_.resize(fd + 1);
            } else {
                poll_items_.resize(count + 1024);
            }
        }
    }
    PollItemVector poll_items_;
};

ATOM_NS_END

#endif
