#include "config.h"
#include <set>
#include <string>
#include <list>
#include <map>
#include <unordered_map>
#include <vector>
#include <thread>
#include <mutex>

#include <assert.h>

#include "hydrogenapi.h"
#include "hydrogendevapi.h"
#include "sharedblob.h"
#include "lilxml.h"
#include "base64.h"

#include "client_info.hpp"
#include "concurrent.hpp"
#include "driver_info.hpp"

#include "io.hpp"
#include "message_queue.hpp"
#include "message.hpp"
#include "property.hpp"
#include "serialize.hpp"
#include "tcp_server.hpp"
#include "time.hpp"
#include "xml_util.hpp"

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

#include <ev++.h>

int main(int ac, char *av[])
{
    /* log startup */
    logStartup(ac, av);

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
#ifdef ENABLE_INDI_SHARED_MEMORY
            case 'u':
                if (ac < 2)
                {
                    fprintf(stderr, "-u requires local socket path\n");
                    usage();
                }
                UnixServer::unixSocketPath = *++av;
                ac--;
                break;
#endif // ENABLE_INDI_SHARED_MEMORY
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
#endif

    /* at this point there are ac args in av[] to name our drivers */
    if (ac == 0 && !fifo)
        usage();

    /* take care of some unixisms */
    noSIGPIPE();

    /* start each driver */
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

    /* announce we are online */
    (new TcpServer(port))->listen();

#ifdef ENABLE_INDI_SHARED_MEMORY
    /* create a new unix server */
    (new UnixServer(UnixServer::unixSocketPath))->listen();
#endif
    /* Load up FIFO, if available */
    if (fifo)
    {
        // New started drivers will not inherit server's prefix anymore

        // JM 2022.07.23: This causes an issue on MacOS. Disabled for now until investigated further.
        // unsetenv("INDIPREFIX");
        fifo->listen();
    }

    /* handle new clients and all io */
    loop.loop();

    /* will not happen unless no more listener left ! */
    log("unexpected return from event loop\n");
    return (1);
}

/* record we have started and our args */
static void logStartup(int ac, char *av[])
{
    int i;

    std::string startupMsg = "startup:";
    for (i = 0; i < ac; i++)
    {
        startupMsg += " ";
        startupMsg += av[i];
    }
    startupMsg += '\n';
    log(startupMsg);
}

/* print usage message and exit (2) */
static void usage(void)
{
    fprintf(stderr, "Usage: %s [options] driver [driver ...]\n", me);
    fprintf(stderr, "Purpose: server for local and remote INDI drivers\n");
    fprintf(stderr, "INDI Library: %s\nCode %s. Protocol %g.\n", CMAKE_INDI_VERSION_STRING, GIT_TAG_STRING, INDIV);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, " -l d     : log driver messages to <d>/YYYY-MM-DD.islog\n");
    fprintf(stderr, " -m m     : kill client if gets more than this many MB behind, default %d\n", DEFMAXQSIZ);
    fprintf(stderr,
            " -d m     : drop streaming blobs if client gets more than this many MB behind, default %d. 0 to disable\n",
            DEFMAXSSIZ);
#ifdef ENABLE_INDI_SHARED_MEMORY
    fprintf(stderr, " -u path  : Path for the local connection socket (abstract), default %s\n", INDIUNIXSOCK);
#endif
    fprintf(stderr, " -p p     : alternate IP port, default %d\n", INDIPORT);
    fprintf(stderr, " -r r     : maximum driver restarts on error, default %d\n", DEFMAXRESTART);
    fprintf(stderr, " -f path  : Path to fifo for dynamic startup and shutdown of drivers.\n");
    fprintf(stderr, " -v       : show key events, no traffic\n");
    fprintf(stderr, " -vv      : -v + key message content\n");
    fprintf(stderr, " -vvv     : -vv + complete xml\n");
    fprintf(stderr, "driver    : executable or [device]@host[:port]\n");

    exit(2);
}

#ifdef _WIN32
/* turn off SIGPIPE on bad write so we can handle it inline */
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

ConcurrentSet<DvrInfo> DvrInfo::drivers;

ConcurrentSet<ClInfo> ClInfo::clients;
