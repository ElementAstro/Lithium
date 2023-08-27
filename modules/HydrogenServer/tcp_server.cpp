#include "tcp_server.hpp"

#ifdef _WIN32

#else
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#endif

#include "io.hpp"
#include "client_info.hpp"
#include "hydrogen_server.hpp"

#include "loguru/loguru.hpp"

#ifdef ENABLE_HYDROGEN_SHARED_MEMORY

UnixServer::UnixServer(const std::string &path) : path(path)
{
    sfdev.set<UnixServer, &UnixServer::ioCb>(this);
}

void UnixServer::log(const std::string &str) const
{
    std::string logLine = "Local server: ";
    logLine += str;
    ::log(logLine);
}

void UnixServer::ioCb(ev::io &, int revents)
{
    if (revents & EV_ERROR)
    {
        int sockErrno = readFdError(this->sfd);
        if (sockErrno)
        {
            log(fmt("Error on unix socket: %s\n", strerror(sockErrno)));
            // Bye();
        }
    }
    if (revents & EV_READ)
    {
        accept();
    }
}

static void initUnixSocketAddr(const std::string &unixAddr, struct sockaddr_un &serv_addr_un, socklen_t &addrlen, bool bind)
{
    memset(&serv_addr_un, 0, sizeof(serv_addr_un));
    serv_addr_un.sun_family = AF_UNIX;

#ifdef __linux__
    (void)bind;

    // Using abstract socket path to avoid filesystem boilerplate
    strncpy(serv_addr_un.sun_path + 1, unixAddr.c_str(), sizeof(serv_addr_un.sun_path) - 1);

    int len = offsetof(struct sockaddr_un, sun_path) + unixAddr.size() + 1;

    addrlen = len;
#else
    // Using filesystem socket path
    strncpy(serv_addr_un.sun_path, unixAddr.c_str(), sizeof(serv_addr_un.sun_path) - 1);

    int len = offsetof(struct sockaddr_un, sun_path) + unixAddr.size();

    if (bind)
    {
        unlink(unixAddr.c_str());
    }
#endif
    addrlen = len;
}

void UnixServer::listen()
{
    struct sockaddr_un serv_socket;

    /* make socket endpoint */
    if ((sfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
        log(fmt("socket: %s\n", strerror(errno)));
        // Bye();
    }

    int reuse = 1;
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
    {
        log(fmt("setsockopt: %s\n", strerror(errno)));
        // Bye();
    }

    /* bind to given path as unix address */
    socklen_t len;
    initUnixSocketAddr(path, serv_socket, len, true);

    if (bind(sfd, (struct sockaddr *)&serv_socket, len) < 0)
    {
        log(fmt("bind: %s\n", strerror(errno)));
        // Bye();
    }

    /* willing to accept connections with a backlog of 5 pending */
    if (::listen(sfd, 5) < 0)
    {
        log(fmt("listen: %s\n", strerror(errno)));
        // Bye();
    }

    fcntl(sfd, F_SETFL, fcntl(sfd, F_GETFL, 0) | O_NONBLOCK);
    sfdev.start(sfd, EV_READ);

    /* ok */
    if (verbose > 0)
        log(fmt("listening on local domain at: @%s\n", path.c_str()));
}

void UnixServer::accept()
{
    int cli_fd;

    /* get a private connection to new client */
    cli_fd = ::accept(sfd, 0, 0);
    if (cli_fd < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;

        log(fmt("accept: %s\n", strerror(errno)));
        // Bye();
    }

    ClInfo *cp = new ClInfo(true);

    /* rig up new clinfo entry */
    cp->setFds(cli_fd, cli_fd);

    if (verbose > 0)
    {
#ifdef SO_PEERCRED
        struct ucred ucred;

        socklen_t len = sizeof(struct ucred);
        if (getsockopt(cli_fd, SOL_SOCKET, SO_PEERCRED, &ucred, &len) == -1)
        {
            log(fmt("getsockopt failed: %s\n", strerror(errno)));
            // Bye();
        }

        cp->log(fmt("new arrival from local pid %ld (user: %ld:%ld) - welcome!\n", (long)ucred.pid, (long)ucred.uid,
                    (long)ucred.gid));
#else
        cp->log(fmt("new arrival from local domain  - welcome!\n"));
#endif
    }

#ifdef OSX_EMBEDED_MODE
    fprintf(stderr, "CLIENTS %d\n", clients.size());
    fflush(stderr);
#endif
}

#endif // ENABLE_HYDROGEN_SHARED_MEMORY

TcpServer::TcpServer(int port) : port(port)
{
    sfdev.set<TcpServer, &TcpServer::ioCb>(this);
}

void TcpServer::ioCb(ev::io &, int revents)
{
    if (revents & EV_ERROR)
    {
        int sockErrno = readFdError(this->sfd);
        if (sockErrno)
        {
            LOG_F(ERROR, "Error on tcp server socket: %s\n", strerror(sockErrno));
            // Bye();
        }
    }
    if (revents & EV_READ)
    {
        accept();
    }
}

void TcpServer::listen()
{
    struct sockaddr_in serv_socket;
    int reuse = 1;

    /* make socket endpoint */
    if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        LOG_F(ERROR, "socket: %s\n", strerror(errno));
        // Bye();
    }

    /* bind to given port for any IP address */
    memset(&serv_socket, 0, sizeof(serv_socket));
    serv_socket.sin_family = AF_INET;
#ifdef SSH_TUNNEL
    serv_socket.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
#else
    serv_socket.sin_addr.s_addr = htonl(INADDR_ANY);
#endif
    serv_socket.sin_port = htons((unsigned short)port);
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
    {
        LOG_F(ERROR, "setsockopt: %s\n", strerror(errno));
        // Bye();
    }
    if (bind(sfd, (struct sockaddr *)&serv_socket, sizeof(serv_socket)) < 0)
    {
        LOG_F(ERROR, "bind: %s\n", strerror(errno));
        // Bye();
    }

    /* willing to accept connections with a backlog of 5 pending */
    if (::listen(sfd, 5) < 0)
    {
        LOG_F(ERROR, "listen: %s\n", strerror(errno));
        // Bye();
    }

    fcntl(sfd, F_SETFL, fcntl(sfd, F_GETFL, 0) | O_NONBLOCK);
    sfdev.start(sfd, EV_READ);

    /* ok */
    if (verbose > 0)
        LOG_F(INFO, "listening to port %d on fd %d\n", port, sfd);
}

void TcpServer::accept()
{
    struct sockaddr_in cli_socket;
    socklen_t cli_len;
    int cli_fd;

    /* get a private connection to new client */
    cli_len = sizeof(cli_socket);
    cli_fd = ::accept(sfd, (struct sockaddr *)&cli_socket, &cli_len);
    if (cli_fd < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;

        LOG_F(ERROR, "accept: %s\n", strerror(errno));
        // Bye();
    }

    ClInfo *cp = new ClInfo(false);

    /* rig up new clinfo entry */
    cp->setFds(cli_fd, cli_fd);

#ifdef OSX_EMBEDED_MODE
    fprintf(stderr, "CLIENTS %d\n", clients.size());
    fflush(stderr);
#endif
}
