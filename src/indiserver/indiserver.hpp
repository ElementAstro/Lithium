
#pragma once

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <poll.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/un.h>
#include <libgen.h>
#ifdef MSG_ERRQUEUE
#include <linux/errqueue.h>
#endif

#include <set>
#include <string>
#include <list>
#include <map>
#include <unordered_map>
#include <vector>
#include <thread>
#include <mutex>
#include <cassert>

#include "indicore/config.h"
#include <libindi/indidevapi.h>
#include <libindi/sharedblob.h>
#include <libindi/lilxml.h>
#include <libindi/base64.h>

#include <ev++.h>

#include <spdlog/spdlog.h>

#include "indidvrinfo.hpp"
#include "indimsg.hpp"
#include "indismsg.hpp"

#define INDIPORT 7624 /* default TCP/IP port to listen */
#define INDIUNIXSOCK "/tmp/indiserver"
#define MAXSBUF 512
#define MAXRBUF 49152        /* max read buffering here */
#define MAXWSIZ 49152        /* max bytes/write */
#define SHORTMSGSIZ 2048     /* buf size for most messages */
#define DEFMAXQSIZ 128       /* default max q behind, MB */
#define DEFMAXSSIZ 5         /* default max stream behind, MB */
#define DEFMAXRESTART 10     /* default max restarts */
#define MAXFD_PER_MESSAGE 16 /* No more than 16 buffer attached to a message */
#define STRINGIFY_TOK(x) #x
#define TO_STRING(x) STRINGIFY_TOK(x)
#define GIT_TAG_STRING 1.7

static ev::default_loop loop;

/* device + property name */
class Property
{
public:
    std::string dev;
    std::string name;
    BLOBHandling blob = B_NEVER; /* when to snoop BLOBs */

    Property(const std::string &dev, const std::string &name) : dev(dev), name(name) {}
};

static char *indi_tstamp(char *s);

static const char *me;                                         /* our name */
static int port = INDIPORT;                                    /* public INDI port */
static int verbose;                                            /* chattiness */
static char *ldir;                                             /* where to log driver messages */
static unsigned int maxqsiz = (DEFMAXQSIZ * 1024 * 1024);      /* kill if these bytes behind */
static unsigned int maxstreamsiz = (DEFMAXSSIZ * 1024 * 1024); /* drop blobs if these bytes behind while streaming*/
static int maxrestarts = DEFMAXRESTART;

/* turn off SIGPIPE on bad write so we can handle it inline */
static void noSIGPIPE()
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    (void)sigaction(SIGPIPE, &sa, NULL);
}

/* fill s with current UT string.
 * if no s, use a static buffer
 * return s or buffer.
 * N.B. if use our buffer, be sure to use before calling again
 */
static char *indi_tstamp(char *s)
{
    static char sbuf[64];
    struct tm *tp;
    time_t t;

    time(&t);
    tp = gmtime(&t);
    if (!s)
        s = sbuf;
    strftime(s, sizeof(sbuf), "%Y-%m-%dT%H:%M:%S", tp);
    return (s);
}

/* log message in root known to be from device dev to ldir, if any.
 */
static void logDMsg(XMLEle *root, const char *dev)
{
    char stamp[64];
    char logfn[1024];
    const char *ts, *ms;
    FILE *fp;

    /* get message, if any */
    ms = findXMLAttValu(root, "message");
    if (!ms[0])
        return;

    /* get timestamp now if not provided */
    ts = findXMLAttValu(root, "timestamp");
    if (!ts[0])
    {
        indi_tstamp(stamp);
        ts = stamp;
    }

    /* append to log file, name is date portion of time stamp */
    sprintf(logfn, "%s/%.10s.islog", ldir, ts);
    fp = fopen(logfn, "a");
    if (!fp)
        return; /* oh well */
    spdlog::debug("{}: {}: {}", ts, dev, ms);
    fclose(fp);
}

/* log when then exit */
static void Bye()
{
    fprintf(stderr, "%s: good bye\n", indi_tstamp(NULL));
    exit(1);
}

static std::vector<XMLEle *> findBlobElements(XMLEle *root)
{
    std::vector<XMLEle *> result;
    for (auto ep = nextXMLEle(root, 1); ep; ep = nextXMLEle(root, 0))
    {
        if (strcmp(tagXMLEle(ep), "oneBLOB") == 0)
        {
            result.push_back(ep);
        }
    }
    return result;
}

static void log(const std::string &log)
{
    fprintf(stderr, "%s: ", indi_tstamp(NULL));
    fprintf(stderr, "%s", log.c_str());
}

static int readFdError(int fd)
{
#ifdef MSG_ERRQUEUE
    char rcvbuf[128]; /* Buffer for normal data (not expected here...) */
    char cbuf[512];   /* Buffer for ancillary data (errors) */
    struct iovec iov;
    struct msghdr msg;

    iov.iov_base = &rcvbuf;
    iov.iov_len = sizeof(rcvbuf);

    msg.msg_name = nullptr;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_flags = 0;
    msg.msg_control = cbuf;
    msg.msg_controllen = sizeof(cbuf);

    int recv_bytes = recvmsg(fd, &msg, MSG_ERRQUEUE | MSG_DONTWAIT);
    if (recv_bytes == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return 0;
        return errno;
    }

    /* Receive auxiliary data in msgh */
    for (struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; cmsg = CMSG_NXTHDR(&msg, cmsg))
    {
        fprintf(stderr, "cmsg_len=%zu, cmsg_level=%u, cmsg_type=%u\n", cmsg->cmsg_len, cmsg->cmsg_level, cmsg->cmsg_type);

        if (cmsg->cmsg_level == SOL_IP && cmsg->cmsg_type == IP_RECVERR)
        {
            return ((struct sock_extended_err *)CMSG_DATA(cmsg))->ee_errno;
        }
    }
#else
    (void)fd;
#endif

    // Default to EIO as a generic error path
    return EIO;
}

static void *attachSharedBuffer(int fd, size_t &size)
{
    struct stat sb;
    if (fstat(fd, &sb) == -1)
    {
        perror("invalid shared buffer fd");
        Bye();
    }
    size = sb.st_size;
    void *ret = mmap(0, size, PROT_READ, MAP_SHARED, fd, 0);

    if (ret == MAP_FAILED)
    {
        perror("mmap");
        Bye();
    }

    return ret;
}

static void dettachSharedBuffer(int fd, void *ptr, size_t size)
{
    (void)fd;
    if (munmap(ptr, size) == -1)
    {
        perror("shared buffer munmap");
        Bye();
    }
}

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

    /* create the public INDI Driver endpoint lsocket on port.
     * return server socket else exit.
     */
    void listen();
};