#include "config.h"
#include <set>
#include <string>
#include <list>
#include <map>
#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif
#include <vector>
#include <thread>
#include <mutex>

#include <assert.h>

#include "hydrogenapi.h"
#include "hydrogendevapi.hpp"
#include "sharedblob.hpp"
#include "lilxml.hpp"
#include "base64.hpp"

#include "client_info.hpp"
#include "concurrent.hpp"
#include "driver_info.hpp"
#include "local_driver.hpp"
#include "remote_driver.hpp"

#include "io.hpp"
#include "message_queue.hpp"
#include "message.hpp"
#include "property.hpp"
#include "serialize.hpp"
#include "tcp_server.hpp"
#include "time.hpp"
#include "xml_util.hpp"
#include "signal.hpp"

#include "hydrogen_server.hpp"

#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/un.h>
#include <netdb.h>
#include <poll.h>
#ifdef MSG_ERRQUEUE
#include <linux/errqueue.h>
#endif
#endif

#ifdef USE_LIBUV
#include <uv.h>
#else
#include <ev++.h>
#endif

#include "atom/log/loguru.hpp"
#include "backward/backward.hpp"

extern ConcurrentSet<ClInfo> ClInfo::clients;
extern ConcurrentSet<DvrInfo> DvrInfo::drivers;

/* print usage message and exit (2) */
static void usage(void)
{
    fprintf(stderr, "Usage: %s [options] driver [driver ...]\n", me);
    fprintf(stderr, "Purpose: server for local and remote HYDROGEN drivers\n");
    fprintf(stderr, "HYDROGEN Protocol %g.\n", HYDROGENV);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, " -l d     : log driver messages to <d>/YYYY-MM-DD.islog\n");
    fprintf(stderr, " -m m     : kill client if gets more than this many MB behind, default %d\n", DEFMAXQSIZ);
    fprintf(stderr,
            " -d m     : drop streaming blobs if client gets more than this many MB behind, default %d. 0 to disable\n",
            DEFMAXSSIZ);
#ifdef ENABLE_HYDROGEN_SHARED_MEMORY
    fprintf(stderr, " -u path  : Path for the local connection socket (abstract), default %s\n", HYDROGENUNIXSOCK);
#endif
    fprintf(stderr, " -p p     : alternate IP port, default %d\n", HYDROGENPORT);
    fprintf(stderr, " -r r     : maximum driver restarts on error, default %d\n", DEFMAXRESTART);
    fprintf(stderr, " -f path  : Path to fifo for dynamic startup and shutdown of drivers.\n");
    fprintf(stderr, " -v       : show key events, no traffic\n");
    fprintf(stderr, " -vv      : -v + key message content\n");
    fprintf(stderr, " -vvv     : -vv + complete xml\n");
    fprintf(stderr, "driver    : executable or [device]@host[:port]\n");

    exit(2);
}

/*
#ifdef _WIN32
static void noSIGPIPE()
{
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
}
#else
static void noSIGPIPE()
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    (void)sigaction(SIGPIPE, &sa, NULL);
}
#endif
*/
void noSIGPIPE()
{
    SignalHandler::registerHandler(SIGPIPE, []() {});
}

void cleanup()
{
    SignalHandler::unregisterHandler(SIGPIPE);
}

#ifdef MAIN_FUNC

int main(int ac, char *av[])
{
    /* save our name */
    me = av[0];

#ifdef OSX_EMBEDED_MODE

    char logname[128];
    snprintf(logname, 128, LOGNAME, getlogin());
    fprintf(stderr, "switching stderr to %s", logname);
    freopen(logname, "w", stderr);

    fifo = new Fifo();
    fifo->name = FIFONAME;
    verbose = 1;
    ac = 0;

#else

    /* crack args */
    /*
        Old:
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
                    usage();
                }
                ldir = *++av;
                ac--;
                break;
            case 'm':
                if (ac < 2)
                {
                    fprintf(stderr, "-m requires max MB behind\n");
                    usage();
                }
                maxqsiz = 1024 * 1024 * atoi(*++av);
                ac--;
                break;
            case 'p':
                if (ac < 2)
                {
                    fprintf(stderr, "-p requires port value\n");
                    usage();
                }
                port = atoi(*++av);
                ac--;
                break;
            case 'd':
                if (ac < 2)
                {
                    fprintf(stderr, "-d requires max stream MB behind\n");
                    usage();
                }
                maxstreamsiz = 1024 * 1024 * atoi(*++av);
                ac--;
                break;
#ifdef ENABLE_HYDROGEN_SHARED_MEMORY
            case 'u':
                if (ac < 2)
                {
                    fprintf(stderr, "-u requires local socket path\n");
                    usage();
                }
                UnixServer::unixSocketPath = *++av;
                ac--;
                break;
#endif // ENABLE_HYDROGEN_SHARED_MEMORY
            case 'f':
                if (ac < 2)
                {
                    fprintf(stderr, "-f requires fifo node\n");
                    usage();
                }
                fifo = new Fifo(*++av);
                ac--;
                break;
            case 'r':
                if (ac < 2)
                {
                    fprintf(stderr, "-r requires number of restarts\n");
                    usage();
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
                usage();
            }
    }
    */
    int opt;
    while ((opt = getopt(ac, av, "l:m:p:d:u:f:r:v")) != -1)
    {
        switch (opt)
        {
        case 'l':
            ldir = optarg;
            break;
        case 'm':
            maxqsiz = std::stoi(optarg) * 1024 * 1024;
            break;
        case 'p':
            port = std::stoi(optarg);
            break;
        case 'd':
            maxstreamsiz = std::stoi(optarg) * 1024 * 1024;
            break;
#ifdef ENABLE_HYDROGEN_SHARED_MEMORY
        case 'u':
            UnixServer::unixSocketPath = optarg;
            break;
#endif // ENABLE_HYDROGEN_SHARED_MEMORY
        case 'f':
            fifo = new Fifo(optarg);
            break;
        case 'r':
            maxrestarts = std::stoi(optarg);
            if (maxrestarts < 0)
                maxrestarts = 0;
            break;
        case 'v':
            verbose++;
            break;
        default: // '?'
            usage();
        }
    }

#endif

    /* at this point there are ac args in av[] to name our drivers */
    if (ac == 0 && !fifo)
        usage();

    /* take care of some unixisms */
    noSIGPIPE();

    /* start each driver */
    /* Old:
        while (ac-- > 0)
        {
            std::string dvrName = *av++;
            DvrInfo *dr;
            if (dvrName.find('@') != std::string::npos)
            {
                dr = new RemoteDvrInfo();
            }
            else
            {
                dr = new LocalDvrInfo();
            }
            dr->name = dvrName;
            dr->start();
        }
    */
    DLOG_F(INFO, "Start loading driver...");
    int count = ac - 1;
    std::vector<std::shared_ptr<DvrInfo>> drivers;
    for (int i = 0; i < count; i++)
    {
        std::string dvrName = av[i + 1];
        std::shared_ptr<DvrInfo> driver;
        if (dvrName.find('@') != std::string::npos)
        {
            driver = std::make_unique<RemoteDvrInfo>();
        }
        else
        {
            driver = std::make_unique<LocalDvrInfo>();
        }
        driver->name = dvrName;
        drivers.push_back(driver);
        drivers[i]->start();
        DLOG_F(INFO, "Started {}", driver->name);
    }

    /* announce we are online */
    // Old: (new TcpServer(port))->listen();
    std::shared_ptr<TcpServer> tcp_server;
    tcp_server = std::make_shared<TcpServer>(port);
    tcp_server->listen();

#ifdef ENABLE_HYDROGEN_SHARED_MEMORY
    /* create a new unix server */
    (new UnixServer(UnixServer::unixSocketPath))->listen();
#endif
    /* Load up FIFO, if available */
    if (fifo)
    {
        // New started drivers will not inherit server's prefix anymore

        // JM 2022.07.23: This causes an issue on MacOS. Disabled for now until investigated further.
        // unsetenv("HYDROGENPREFIX");
        DLOG_F(INFO, "Starting FIFO server");
        fifo->listen();
    }

    /* handle new clients and all io */
    DLOG_F(INFO, "Main loop started");
#ifdef USE_LIBUV
    uv_run(loop, UV_RUN_DEFAULT);
#else
    loop.loop();
#endif

    /* will not happen unless no more listener left ! */
    LOG_F(ERROR, "unexpected return from event loop");
    return (1);
}

#else

void run_hydrogen_server(std::unordered_map<std::string, std::string> m_params)
{
    /* save our name */
    me = "hydrogen_server_inside";

    std::shared_ptr<TcpServer> tcp_server;
    tcp_server = std::make_shared<TcpServer>(port);
    tcp_server->listen();

    fifo = new Fifo();
    fifo->name = "/tmp/hydrogenserverFIFO";
    /* Load up FIFO, if available */
    if (fifo)
    {
        DLOG_F(INFO, "Starting FIFO server");
        fifo->listen();
    }

    /* handle new clients and all io */
    DLOG_F(INFO, "Main loop started");
#ifdef USE_LIBUV
    uv_run(loop, UV_RUN_DEFAULT);
#else
    loop.loop();
#endif
    /* will not happen unless no more listener left ! */
    LOG_F(ERROR, "unexpected return from event loop");
}

void start_hydrogen_driver(const std::string &driver_binary,const std::string &driver_skeleton)
{
    std::string cmd = "start " + driver->binary;
    if (!driver_skeleton.empty())
    {
        cmd += " -s \"" + driver->skeleton + "\"";
    }
    cmd = std::regex_replace(cmd, std::regex("\""), "\\\"");
    if (fifo)
    {
        fifo->processLine(cmd.c_str());
    }
}

void stop_hydrogen_driver(const std::string &driver_binary, const std::string &driver_lable = "")
{
    std::string cmd = "stop " + driver_binary;
    if (driver_binary.find("@") == std::string::npos)
    {
        cmd += " -n \"" + driver_label + "\"";
    }
    cmd = std::regex_replace(cmd, std::regex("\""), "\\\"");
    if (fifo)
    {
        fifo->processLine(cmd.c_str());
    }
}
#endif
