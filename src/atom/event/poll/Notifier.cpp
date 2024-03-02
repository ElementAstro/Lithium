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

#ifdef ATOM_OS_LINUX
#include "event_notifier.hpp"
#elif !defined(ATOM_OS_WIN)
#include "pipe_notifier.hpp"
#else
#include "socket_notifier.hpp"
#endif

ATOM_NS_USING

NotifierPtr Notifier::createNotifier() {
#ifdef ATOM_OS_LINUX
    return NotifierPtr(new EventNotifier());
#elif !defined(ATOM_OS_WIN)
    return NotifierPtr(new PipeNotifier());
#else
    return NotifierPtr(new SocketNotifier());
#endif
}
