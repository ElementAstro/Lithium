#pragma once

#ifdef _WIN32

#else
#include <sys/socket.h>
#endif

#ifdef USE_LIBUV
#include <uv.h>
#else
#include <ev++.h>
#endif

class TcpServer
{
    int port;
    int sfd = -1;
#ifdef USE_LIBUV
    uv_poll_t poll_watcher;
#else
    ev::io sfdev;
#endif

    
#ifdef USE_LIBUV
    void ioCb(uv_poll_t *watcher, int status, int revents);
    void accept();
#else
    void ioCb(ev::io &watcher, int revents);
    /* prepare for new client arriving on socket.
     * exit if trouble.
     */
    void accept();
#endif

public:
    TcpServer(int port);

    /* create the public HYDROGEN Driver endpoint lsocket on port.
     * return server socket else exit.
     */
    void listen();
};
