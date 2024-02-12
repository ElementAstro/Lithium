/*
 * kevdefs.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-10

Description: Some defines for kev

**************************************************/

#ifndef __KEVDEFS_HPP
#define __KEVDEFS_HPP

#include "kmconf.hpp"

#include <stdint.h>
#include <stddef.h>
#include <functional>

#define ATOM_NS_BEGIN \
    namespace Atom::Event    \
    {
#define ATOM_NS_END }
#define ATOM_NS_USING using namespace Atom::Event;

ATOM_NS_BEGIN

#ifdef ATOM_OS_WIN
using SOCKET_FD = uintptr_t;
const SOCKET_FD INVALID_FD = (SOCKET_FD)~0;
#else
using SOCKET_FD = int;
const SOCKET_FD INVALID_FD = ((SOCKET_FD)-1);
#endif

using KMEvent = uint32_t;
using IOCallback = std::function<void(SOCKET_FD, KMEvent, void *, size_t)>;

const uint32_t kEventRead = 1;
const uint32_t kEventWrite = (1 << 1);
const uint32_t kEventError = (1 << 2);
const uint32_t kEventNetwork = (kEventRead | kEventWrite | kEventError);

enum class Result : int
{
    OK = 0,
    FAILED = -1,
    FATAL = -2,
    REJECTED = -3,
    CLOSED = -4,
    AGAIN = -5,
    ABORTED = -6,
    TIMEOUT = -7,
    INVALID_STATE = -8,
    INVALID_PARAM = -9,
    INVALID_PROTO = -10,
    ALREADY_EXIST = -11,
    NOT_EXIST = -12,
    SOCK_ERROR = -13,
    POLL_ERROR = -14,
    PROTO_ERROR = -15,
    SSL_ERROR = -16,
    BUFFER_TOO_SMALL = -17,
    BUFFER_TOO_LONG = -18,
    NOT_SUPPORTED = -19,
    NOT_IMPLEMENTED = -20,
    NOT_AUTHORIZED = -21,

    DESTROYED = -699
};

enum class PollType
{
    DEFAULT,
    SELECT,
    POLL,
    EPOLL,
    KQUEUE,
    IOCP,
    RUNLOOP,
    STLCV, // none IO event loop
};

#ifdef ATOM_OS_WIN
struct iovec
{
    unsigned long iov_len;
    char *iov_base;
};
#endif

ATOM_NS_END

#endif
