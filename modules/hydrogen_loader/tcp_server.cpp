#include "tcp_server.hpp"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <netdb.h>
#endif

#include <fcntl.h>
#include <unistd.h>

#include "io.hpp"
#include "client_info.hpp"
#include "hydrogen_server.hpp"

#include "atom/log/loguru.hpp"

TcpServer::TcpServer(int port) : port(port)
{
#ifdef USE_LIBUV
    poll_watcher.data = this;
#else
    sfdev.set<TcpServer, &TcpServer::ioCb>(this);
#endif
}

#ifdef USE_LIBUV
void TcpServer::ioCb(uv_poll_t *watcher, int status, int revents)
{
    if (status < 0)
    {
        LOG_F(ERROR, "Error on tcp server socket: {}", uv_strerror(status));
        // Bye();
    }
    else if (revents & UV_READABLE)
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
#ifdef _WIN32
    if (setsockopt(sfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&reuse, sizeof(reuse)) < 0)
    {
        LOG_F(ERROR, "Failed to set receive timeout.");
        close(sfd);
        sfd = -1;
        return;
    }
#else
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
    {
        LOG_F(ERROR, "setsockopt: %s\n", strerror(errno));
        // Bye();
    }
#endif
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

#ifdef _WIN32
    int fd = _fileno(reinterpret_cast<FILE *>(_get_osfhandle(sfd)));
    _setmode(fd, _O_BINARY);
#else
    fcntl(sfd, F_SETFL, fcntl(sfd, F_GETFL, 0) | O_NONBLOCK);
#endif

    // 创建uv_poll_t对象，并注册READABLE事件回调函数
    uv_poll_init_socket(loop, &poll_watcher, sfd);
    uv_poll_start(&poll_watcher, UV_READABLE, (uv_poll_cb)&TcpServer::ioCb);

    /* 开始事件循环 */
    uv_run(loop, UV_RUN_DEFAULT);

    /* ok */
    if (verbose > 0)
        DLOG_F(INFO, "listening to port %d on fd %d\n", port, sfd);
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

#else

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
#ifdef _WIN32
    if (setsockopt(sfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&reuse, sizeof(reuse)) < 0)
    {
        LOG_F(ERROR, "Failed to set receive timeout.");
        close(sfd);
        sfd = -1;
        return;
    }
#else
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
    {
        LOG_F(ERROR, "setsockopt: %s\n", strerror(errno));
        // Bye();
    }
#endif
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

#ifdef _WIN32
    int fd = _fileno(reinterpret_cast<FILE *>(_get_osfhandle(sfd)));
    _setmode(fd, _O_BINARY);
#else
    fcntl(sfd, F_SETFL, fcntl(sfd, F_GETFL, 0) | O_NONBLOCK);
#endif
    sfdev.start(sfd, EV_READ);

    /* ok */
    if (verbose > 0)
        DLOG_F(INFO, "listening to port %d on fd %d\n", port, sfd);
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
#endif
