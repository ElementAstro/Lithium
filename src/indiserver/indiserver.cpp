#include "indiserver.hpp"

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
            spdlog::critical("Error on tcp server socket: {}", strerror(sockErrno));
            Bye();
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
        spdlog::info("socket: {}", strerror(errno));
        Bye();
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
        spdlog::critical("setsockopt: {}", strerror(errno));
        Bye();
    }
    if (bind(sfd, (struct sockaddr *)&serv_socket, sizeof(serv_socket)) < 0)
    {
        spdlog::critical("bind: {}", strerror(errno));
        Bye();
    }

    /* willing to accept connections with a backlog of 5 pending */
    if (::listen(sfd, 5) < 0)
    {
        spdlog::critical("listen: {}", strerror(errno));
        Bye();
    }

    fcntl(sfd, F_SETFL, fcntl(sfd, F_GETFL, 0) | O_NONBLOCK);
    sfdev.start(sfd, EV_READ);

    /* ok */
    if (verbose > 0)
        spdlog::info("listening to port {} on fd {}", port, sfd);
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

        spdlog::critical("accept: {}", strerror(errno));
        Bye();
    }

    ClInfo *cp = new ClInfo(false);

    /* rig up new clinfo entry */
    cp->setFds(cli_fd, cli_fd);

    if (verbose > 0)
    {
        spdlog::info("new arrival from {}:{} - welcome!",
                     inet_ntoa(cli_socket.sin_addr), ntohs(cli_socket.sin_port));
    }
}

ConcurrentSet<ClInfo> ClInfo::clients;

int main(int ac, char *av[])
{

    spdlog::set_level(spdlog::level::debug);
    /* save our name */
    me = av[0];

    /* crack args */
    while ((--ac > 0) && ((*++av)[0] == '-'))
    {
        char *s;
        for (s = av[0] + 1; *s != '\0'; s++)
            switch (*s)
            {
            case 'l':
                if (ac < 2)
                {
                    fprintf(stderr, "-l requires log directory\n");
                    // usage();
                }
                ldir = *++av;
                ac--;
                break;
            case 'm':
                if (ac < 2)
                {
                    fprintf(stderr, "-m requires max MB behind\n");
                    // usage();
                }
                maxqsiz = 1024 * 1024 * atoi(*++av);
                ac--;
                break;
            case 'p':
                if (ac < 2)
                {
                    fprintf(stderr, "-p requires port value\n");
                    // usage();
                }
                port = atoi(*++av);
                ac--;
                break;
            case 'd':
                if (ac < 2)
                {
                    fprintf(stderr, "-d requires max stream MB behind\n");
                    // usage();
                }
                maxstreamsiz = 1024 * 1024 * atoi(*++av);
                ac--;
                break;
            case 'r':
                if (ac < 2)
                {
                    fprintf(stderr, "-r requires number of restarts\n");
                    // usage();
                }
                maxrestarts = atoi(*++av);
                if (maxrestarts < 0)
                    maxrestarts = 0;
                ac--;
                break;
            case 'v':
                verbose++;
                break;
            default:
                // usage();
                printf("hello");
            }
    }

    /* at this point there are ac args in av[] to name our drivers */
    if (ac == 0)
    {
    }
    // usage();

    /* take care of some unixisms */
    noSIGPIPE();

    /* start each driver */
    while (ac-- > 0)
    {
        std::string dvrName = *av++;
        spdlog::debug("Start {}", dvrName);
        DvrInfo *dr;
        dr = new LocalDvrInfo();
        dr->name = dvrName;
        dr->start();
    }

    /* announce we are online */
    (new TcpServer(port))->listen();

    /* handle new clients and all io */
    loop.loop();

    /* will not happen unless no more listener left ! */
    spdlog::error("unexpected return from event loop");
    return (1);
}