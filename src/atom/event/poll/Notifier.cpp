/*
 * notifier.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-10

Description: Notifier for poll

**************************************************/

#include "kevdefs.hpp"

#ifdef ATOM_OS_WIN
#include <Ws2tcpip.h>
#include <windows.h>
#elif defined(ATOM_OS_LINUX)
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netinet/in.h>

#elif defined(ATOM_OS_MAC)
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>

#else
#error "UNSUPPORTED OS"
#endif

#ifdef ATOM_OS_LINUX
#include "event_notifier.hpp"
#elif !defined(ATOM_OS_WIN)
#include "pipe_notifier.hpp"
#else
#include "socket_notifier.hpp"
#endif

ATOM_NS_USING

NotifierPtr Notifier::createNotifier()
{
#ifdef ATOM_OS_LINUX
    return NotifierPtr(new EventNotifier());
#elif !defined(ATOM_OS_WIN)
    return NotifierPtr(new PipeNotifier());
#else
    return NotifierPtr(new SocketNotifier());
#endif
}
