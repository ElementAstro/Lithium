#pragma once

#ifdef _WIN32

#else
#include <sys/socket.h>
#endif

#include <ev++.h>

class TcpServer
{
    int port;
    int sfd = -1;
    ev::io sfdev;

    /* prepare for new client arriving on socket.
     * exit if trouble.
     */
    void accept();
    void ioCb(ev::io &watcher, int revents);

public:
    TcpServer(int port);

    /* create the public HYDROGEN Driver endpoint lsocket on port.
     * return server socket else exit.
     */
    void listen();
};

#ifdef ENABLE_HYDROGEN_SHARED_MEMORY

class UnixServer
{
    std::string path;
    int sfd = -1;
    ev::io sfdev;

    void accept();
    void ioCb(ev::io &watcher, int revents);

    virtual void log(const std::string &log) const;

public:
    UnixServer(const std::string &path);

    /* create the public HYDROGEN Driver endpoint over UNIX (local) domain.
     * exit on failure
     */
    void listen();

    static std::string unixSocketPath;
};

std::string UnixServer::unixSocketPath = HYDROGENUNIXSOCK;

#endif