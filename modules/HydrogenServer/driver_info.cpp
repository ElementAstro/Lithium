#include "driver_info.hpp"

#include <unistd.h>
#include <fcntl.h>

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <sys/socket.h>
#include <libgen.h>
#include <netdb.h>
#include <netinet/in.h>
#endif

#include "lilxml.h"
#include "client_info.hpp"
#include "xml_util.hpp"
#include "hydrogen_server.hpp"

#include "loguru/loguru.hpp"

/* start the given local HYDROGEN driver process.
 * exit if trouble.
 */
#ifdef _WIN32
void LocalDvrInfo::start()
{
    Msg *mp;
    HANDLE hReadPipe, hWritePipe, hErrorPipe;
    SECURITY_ATTRIBUTES sa;
    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    HANDLE hEfd;
    DWORD pid;

#ifdef OSX_EMBEDED_MODE
    fprintf(stderr, "STARTING \"%s\"\n", name.c_str());
    fflush(stderr);
#endif

    HANDLE hRp, hWp, hEp; // 修改变量声明为HANDLE类型

    /* build three pipes: r, w and error*/
    if (useSharedBuffer)
    {
        // FIXME: lots of FD are opened by hydrogenserver. FD_CLOEXEC is a must + check other fds
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = TRUE;
        sa.lpSecurityDescriptor = NULL;
        if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0))
        {
            // log(fmt("CreatePipe: %d\n", GetLastError()));
            // Bye();
        }
        hErrorPipe = hWritePipe;
    }
    else
    {

        if (!CreatePipe(&hRp, &hWp, NULL, 4096))
        {
            // log(fmt("CreatePipe read: %d\n", GetLastError()));
            // Bye();
        }
        if (!CreatePipe(&hRp, &hEp, NULL, 4096))
        {
            // log(fmt("CreatePipe error: %d\n", GetLastError()));
            // Bye();
        }
        hReadPipe = hRp;
        hWritePipe = hWp;
        hErrorPipe = hEp;
    }

    /* fork&exec new process */
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.hStdInput = hReadPipe;
    si.hStdOutput = hWritePipe;
    si.hStdError = hErrorPipe;
    si.dwFlags |= STARTF_USESTDHANDLES;

    if (!CreateProcess(NULL, (LPSTR)name.c_str(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
    {
        // log(fmt("CreateProcess: %d\n", GetLastError()));
        // Bye();
    }

    if (useSharedBuffer)
    {
        /* don't need child's other socket end */
        CloseHandle(hReadPipe);

        /* record pid, io channels, init lp and snoop list */
        setFds(reinterpret_cast<intptr_t>(hWritePipe), reinterpret_cast<intptr_t>(hWritePipe));
    }
    else
    {
        /* don't need child's side of pipes */
        CloseHandle(hWritePipe);
        CloseHandle(hReadPipe);

        /* record pid, io channels, init lp and snoop list */
        setFds(reinterpret_cast<intptr_t>(int) hRp, reinterpret_cast<intptr_t>(int) hWp); // 修改为使用新的变量名
    }

    CloseHandle(hErrorPipe);

    // Watch pid
    this->pid = pi.dwProcessId;
    do
    {
        (this->pidwatcher).pid = (this->pid);
        (this->pidwatcher).flags = !!(1);
    } while (0);
    ev_child_set()
    this->pidwatcher.set(this->pid);
    //this->pidwatcher.start();

    // Watch input on efd
    hEfd = hEp; // 修改为使用新的变量名
    SetHandleInformation(hEfd, HANDLE_FLAG_INHERIT, 0);
    this->efd = _open_osfhandle(reinterpret_cast<intptr_t>(hEfd), _O_RDONLY | _O_BINARY);
    unsigned long mode = 1;
    ioctlsocket(this->efd, FIONBIO, &mode);
    this->eio.start(this->efd, ev::READ);

    /* first message primes driver to report its properties -- dev known
     * if restarting
     */
    if (verbose > 0)
        LOG_F(INFO, "pid=%d rfd=%d wfd=%d efd=%d\n", pid, (int)hRp, (int)hWp, (int)hEp);

    XMLEle *root = addXMLEle(NULL, "getProperties");
    addXMLAtt(root, "version", TO_STRING(HYDROGENV));
    mp = new Msg(nullptr, root);

    // pushmsg can kill mp. do at end
    pushMsg(mp);
}

#else
void LocalDvrInfo::start()
{
    Msg *mp;
    int rp[2], wp[2], ep[2];
    int ux[2];
    int pid;

#ifdef OSX_EMBEDED_MODE
    fprintf(stderr, "STARTING \"%s\"\n", name.c_str());
    fflush(stderr);
#endif

    /* build three pipes: r, w and error*/
    if (useSharedBuffer)
    {
        // FIXME: lots of FD are opened by hydrogenserver. FD_CLOEXEC is a must + check other fds
        if (socketpair(AF_UNIX, SOCK_STREAM, 0, ux) == -1)
        {
            // log(fmt("socketpair: %s\n", strerror(errno)));
            // Bye();
        }
    }
    else
    {
        if (pipe(rp) < 0)
        {
            // log(fmt("read pipe: %s\n", strerror(errno)));
            // Bye();
        }
        if (pipe(wp) < 0)
        {
            // log(fmt("write pipe: %s\n", strerror(errno)));
            // Bye();
        }
    }
    if (pipe(ep) < 0)
    {
        // log(fmt("stderr pipe: %s\n", strerror(errno)));
        // Bye();
    }

    /* fork&exec new process */
    pid = fork();
    if (pid < 0)
    {
        // log(fmt("fork: %s\n", strerror(errno)));
        // Bye();
    }
    if (pid == 0)
    {
        /* child: exec name */
        int fd;

        /* rig up pipes */
        if (useSharedBuffer)
        {
            // For unix socket, the same socket end can be used for both read & write
            dup2(ux[0], 0); /* driver stdin reads from ux[0] */
            dup2(ux[0], 1); /* driver stdout writes to ux[0] */
            ::close(ux[0]);
            ::close(ux[1]);
        }
        else
        {
            dup2(wp[0], 0); /* driver stdin reads from wp[0] */
            dup2(rp[1], 1); /* driver stdout writes to rp[1] */
        }
        dup2(ep[1], 2); /* driver stderr writes to e[]1] */
        for (fd = 3; fd < 100; fd++)
            (void)::close(fd);

        if (!envDev.empty())
            setenv("HYDROGENDEV", envDev.c_str(), 1);
        /* Only reset environment variable in case of FIFO */
        else if (fifo)
            unsetenv("HYDROGENDEV");
        if (!envConfig.empty())
            setenv("HYDROGENCONFIG", envConfig.c_str(), 1);
        else if (fifo)
            unsetenv("HYDROGENCONFIG");
        if (!envSkel.empty())
            setenv("HYDROGENSKEL", envSkel.c_str(), 1);
        else if (fifo)
            unsetenv("HYDROGENSKEL");
        std::string executable;
        if (!envPrefix.empty())
        {
            setenv("HYDROGENPREFIX", envPrefix.c_str(), 1);
#if defined(OSX_EMBEDED_MODE)
            executable = envPrefix + "/Contents/MacOS/" + name;
#elif defined(__APPLE__)
            executable = envPrefix + "/" + name;
#else
            executable = envPrefix + "/bin/" + name;
#endif

            fprintf(stderr, "%s\n", executable.c_str());

            execlp(executable.c_str(), name.c_str(), NULL);
        }
        else
        {
            if (name[0] == '.')
            {
                executable = std::string(dirname((char *)me)) + "/" + name;
                execlp(executable.c_str(), name.c_str(), NULL);
            }
            else
            {
                execlp(name.c_str(), name.c_str(), NULL);
            }
        }

#ifdef OSX_EMBEDED_MODE
        fprintf(stderr, "FAILED \"%s\"\n", name.c_str());
        fflush(stderr);
#endif
        // log(fmt("execlp %s: %s\n", executable.c_str(), strerror(errno)));
        _exit(1); /* parent will notice EOF shortly */
    }

    if (useSharedBuffer)
    {
        /* don't need child's other socket end */
        ::close(ux[0]);

        /* record pid, io channels, init lp and snoop list */
        setFds(ux[1], ux[1]);
        rp[0] = ux[1];
        wp[1] = ux[1];
    }
    else
    {
        /* don't need child's side of pipes */
        ::close(wp[0]);
        ::close(rp[1]);

        /* record pid, io channels, init lp and snoop list */
        setFds(rp[0], wp[1]);
    }

    ::close(ep[1]);

    // Watch pid
    this->pid = pid;
    this->pidwatcher.set(pid);
    this->pidwatcher.start();

    // Watch input on efd
    this->efd = ep[0];
    fcntl(this->efd, F_SETFL, fcntl(this->efd, F_GETFL, 0) | O_NONBLOCK);
    this->eio.start(this->efd, ev::READ);

    /* first message primes driver to report its properties -- dev known
     * if restarting
     */
    if (verbose > 0)
        LOG_F(INFO, "pid=%d rfd=%d wfd=%d efd=%d\n", pid, rp[0], wp[1], ep[0]);

    XMLEle *root = addXMLEle(NULL, "getProperties");
    addXMLAtt(root, "version", TO_STRING(HYDROGENV));
    mp = new Msg(nullptr, root);

    // pushmsg can kill mp. do at end
    pushMsg(mp);
}
#endif

void RemoteDvrInfo::extractRemoteId(const std::string &name, std::string &o_host, int &o_port, std::string &o_dev) const
{
    char dev[MAXHYDROGENDEVICE] = {0};
    char host[MAXSBUF] = {0};

    /* extract host and port from name*/
    int hydrogen_port = HYDROGENPORT;
    if (sscanf(name.c_str(), "%[^@]@%[^:]:%d", dev, host, &hydrogen_port) < 2)
    {
        // Device missing? Try a different syntax for all devices
        if (sscanf(name.c_str(), "@%[^:]:%d", host, &hydrogen_port) < 1)
        {
            // log(fmt("Bad remote device syntax: %s\n", name.c_str()));
            // Bye();
        }
    }

    o_host = host;
    o_port = hydrogen_port;
    o_dev = dev;
}

/* start the given remote HYDROGEN driver connection.
 * exit if trouble.
 */
void RemoteDvrInfo::start()
{
    int sockfd;
    std::string dev;
    extractRemoteId(name, host, port, dev);

    /* connect */
    sockfd = openHYDROGENServer();

    /* record flag pid, io channels, init lp and snoop list */

    this->setFds(sockfd, sockfd);

    if (verbose > 0)
        LOG_F(INFO, "socket=%d\n", sockfd);

    /* N.B. storing name now is key to limiting outbound traffic to this
     * dev.
     */
    if (!dev.empty())
        this->dev.insert(dev);

    /* Sending getProperties with device lets remote server limit its
     * outbound (and our inbound) traffic on this socket to this device.
     */
    XMLEle *root = addXMLEle(NULL, "getProperties");

    if (!dev.empty())
    {
        addXMLAtt(root, "device", dev.c_str());
        addXMLAtt(root, "version", TO_STRING(HYDROGENV));
    }
    else
    {
        // This informs downstream server that it is connecting to an upstream server
        // and not a regular client. The difference is in how it treats snooping properties
        // among properties.
        addXMLAtt(root, "device", "*");
        addXMLAtt(root, "version", TO_STRING(HYDROGENV));
    }

    Msg *mp = new Msg(nullptr, root);

    // pushmsg can kill this. do at end
    pushMsg(mp);
}

int RemoteDvrInfo::openHYDROGENServer()
{
    struct sockaddr_in serv_addr;
    struct hostent *hp;
    int sockfd;

    /* lookup host address */
    hp = gethostbyname(host.c_str());
    if (!hp)
    {
        LOG_F(ERROR, "gethostbyname(%s): %s\n", host.c_str(), strerror(errno));
        // Bye();
    }

    /* create a socket to the HYDROGEN server */
    (void)memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr_list[0]))->s_addr;
    serv_addr.sin_port = htons(port);
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        LOG_F(ERROR, "socket(%s,%d): %s\n", host.c_str(), port, strerror(errno));
        // Bye();
    }

    /* connect */
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        LOG_F(ERROR, "connect(%s,%d): %s\n", host.c_str(), port, strerror(errno));
        // Bye();
    }

    /* ok */
    return (sockfd);
}

void DvrInfo::onMessage(XMLEle *root, std::list<int> &sharedBuffers)
{
    char *roottag = tagXMLEle(root);
    const char *dev = findXMLAttValu(root, "device");
    const char *name = findXMLAttValu(root, "name");
    int isblob = !strcmp(tagXMLEle(root), "setBLOBVector");

    if (verbose > 2)
        traceMsg("read ", root);
    else if (verbose > 1)
    {
        LOG_F(ERROR, "read <%s device='%s' name='%s'>\n", tagXMLEle(root), findXMLAttValu(root, "device"), findXMLAttValu(root, "name"));
    }

    /* that's all if driver is just registering a snoop */
    /* JM 2016-05-18: Send getProperties to upstream chained servers as well.*/
    if (!strcmp(roottag, "getProperties"))
    {
        this->addSDevice(dev, name);
        Msg *mp = new Msg(this, root);
        /* send to interested chained servers upstream */
        // FIXME: no use of root here
        ClInfo::q2Servers(this, mp, root);
        /* Send to snooped drivers if they exist so that they can echo back the snooped propertly immediately */
        // FIXME: no use of root here
        q2RDrivers(dev, mp, root);

        mp->queuingDone();

        return;
    }

    /* that's all if driver desires to snoop BLOBs from other drivers */
    if (!strcmp(roottag, "enableBLOB"))
    {
        Property *sp = findSDevice(dev, name);
        if (sp)
            crackBLOB(pcdataXMLEle(root), &sp->blob);
        delXMLEle(root);
        return;
    }

    /* Found a new device? Let's add it to driver info */
    if (dev[0] && !this->isHandlingDevice(dev))
    {
#ifdef OSX_EMBEDED_MODE
        if (this->dev.empty())
            fprintf(stderr, "STARTED \"%s\"\n", dp->name.c_str());
        fflush(stderr);
#endif
        this->dev.insert(dev);
    }

    /* log messages if any and wanted */
    if (ldir)
        logDMsg(root, dev);

    if (!strcmp(roottag, "pingRequest"))
    {
        setXMLEleTag(root, "pingReply");

        Msg *mp = new Msg(this, root);
        pushMsg(mp);
        mp->queuingDone();
        return;
    }

    /* build a new message -- set content iff anyone cares */
    Msg *mp = Msg::fromXml(this, root, sharedBuffers);
    if (!mp)
    {
        close();
        return;
    }

    /* send to interested clients */
    ClInfo::q2Clients(NULL, isblob, dev, name, mp, root);

    /* send to snooping drivers */
    DvrInfo::q2SDrivers(this, isblob, dev, name, mp, root);

    /* set message content if anyone cares else forget it */
    mp->queuingDone();
}

void DvrInfo::closeWritePart()
{
    // Don't want any half-dead drivers
    close();
}

void DvrInfo::close()
{
    // Tell client driver is dead.
    for (auto dev : dev)
    {
        /* Inform clients that this driver is dead */
        XMLEle *root = addXMLEle(NULL, "delProperty");
        addXMLAtt(root, "device", dev.c_str());

        prXMLEle(stderr, root, 0);
        Msg *mp = new Msg(this, root);

        ClInfo::q2Clients(NULL, 0, dev.c_str(), "", mp, root);
        mp->queuingDone();
    }

    bool terminate;
    if (!restart)
    {
        terminate = true;
    }
    else
    {
        if (restarts >= maxrestarts)
        {
            // log(fmt("Terminated after #%d restarts.\n", restarts));
            terminate = true;
        }
        else
        {
            // log(fmt("restart #%d\n", restarts));
            ++restarts;
            terminate = false;
        }
    }

#ifdef OSX_EMBEDED_MODE
    fprintf(stderr, "STOPPED \"%s\"\n", name.c_str());
    fflush(stderr);
#endif

    // FIXME: we loose stderr from dying driver
    if (terminate)
    {
        delete (this);
        if ((!fifo) && (drivers.ids().empty()))
            // Bye();
            return;
    }
    else
    {
        DvrInfo *restarted = this->clone();
        delete (this);
        restarted->start();
    }
}

void DvrInfo::q2RDrivers(const std::string &dev, Msg *mp, XMLEle *root)
{
    char *roottag = tagXMLEle(root);

    /* queue message to each interested driver.
     * N.B. don't send generic getProps to more than one remote driver,
     *   otherwise they all fan out and we get multiple responses back.
     */
    std::set<std::string> remoteAdvertised;
    for (auto dpId : drivers.ids())
    {
        auto dp = drivers[dpId];
        if (dp == nullptr)
            continue;

        std::string remoteUid = dp->remoteServerUid();
        bool isRemote = !remoteUid.empty();

        /* driver known to not support this dev */
        if ((!dev.empty()) && dev[0] != '*' && !dp->isHandlingDevice(dev))
            continue;

        /* Only send message to each *unique* remote driver at a particular host:port
         * Since it will be propogated to all other devices there */
        if (dev.empty() && isRemote)
        {
            if (remoteAdvertised.find(remoteUid) != remoteAdvertised.end())
                continue;

            /* Retain last remote driver data so that we do not send the same info again to a driver
             * residing on the same host:port */
            remoteAdvertised.insert(remoteUid);
        }

        /* JM 2016-10-30: Only send enableBLOB to remote drivers */
        if (isRemote == 0 && !strcmp(roottag, "enableBLOB"))
            continue;

        // pushmsg can kill dp. do at end
        dp->pushMsg(mp);
    }
}

void DvrInfo::q2SDrivers(DvrInfo *me, int isblob, const std::string &dev, const std::string &name, Msg *mp, XMLEle *root)
{
    std::string meRemoteServerUid = me ? me->remoteServerUid() : "";
    for (auto dpId : drivers.ids())
    {
        auto dp = drivers[dpId];
        if (dp == nullptr)
            continue;

        Property *sp = dp->findSDevice(dev, name);

        /* nothing for dp if not snooping for dev/name or wrong BLOB mode */
        if (!sp)
            continue;
        if ((isblob && sp->blob == B_NEVER) || (!isblob && sp->blob == B_ONLY))
            continue;

        // Do not send snoop data to remote drivers at the same host
        // since they will manage their own snoops remotely
        if ((!meRemoteServerUid.empty()) && dp->remoteServerUid() == meRemoteServerUid)
            continue;

        // pushmsg can kill dp. do at end
        dp->pushMsg(mp);
    }
}

void DvrInfo::addSDevice(const std::string &dev, const std::string &name)
{
    Property *sp;

    /* no dups */
    sp = findSDevice(dev, name);
    if (sp)
        return;

    /* add dev to sdevs list */
    sp = new Property(dev, name);
    sp->blob = B_NEVER;
    sprops.push_back(sp);

    if (verbose)
        LOG_F(INFO, "snooping on %s.%s\n", dev.c_str(), name.c_str());
}

Property *DvrInfo::findSDevice(const std::string &dev, const std::string &name) const
{
    for (auto sp : sprops)
    {
        if ((sp->dev == dev) && (sp->name.empty() || sp->name == name))
            return (sp);
    }

    return nullptr;
}

DvrInfo::DvrInfo(bool useSharedBuffer) : MsgQueue(useSharedBuffer),
                                         restarts(0)
{
    drivers.insert(this);
}

DvrInfo::DvrInfo(const DvrInfo &model) : MsgQueue(model.useSharedBuffer),
                                         name(model.name),
                                         restarts(model.restarts)
{
    drivers.insert(this);
}

DvrInfo::~DvrInfo()
{
    drivers.erase(this);
    for (auto prop : sprops)
    {
        delete prop;
    }
}

bool DvrInfo::isHandlingDevice(const std::string &dev) const
{
    return this->dev.find(dev) != this->dev.end();
}

void DvrInfo::log(const std::string &str) const
{
    std::string logLine = "Driver ";
    logLine += name;
    logLine += ": ";
    logLine += str;
    LOG_F(INFO, "%s", logLine.c_str());
}

LocalDvrInfo::LocalDvrInfo() : DvrInfo(true)
{
    eio.set<LocalDvrInfo, &LocalDvrInfo::onEfdEvent>(this);
    pidwatcher.set<LocalDvrInfo, &LocalDvrInfo::onPidEvent>(this);
}

LocalDvrInfo::LocalDvrInfo(const LocalDvrInfo &model) : DvrInfo(model),
                                                        envDev(model.envDev),
                                                        envConfig(model.envConfig),
                                                        envSkel(model.envSkel),
                                                        envPrefix(model.envPrefix)
{
    eio.set<LocalDvrInfo, &LocalDvrInfo::onEfdEvent>(this);
    pidwatcher.set<LocalDvrInfo, &LocalDvrInfo::onPidEvent>(this);
}

LocalDvrInfo::~LocalDvrInfo()
{
    closeEfd();
    if (pid != 0)
    {
        kill(pid, SIGKILL); /* libev insures there will be no zombies */
        pid = 0;
    }
    closePid();
}

LocalDvrInfo *LocalDvrInfo::clone() const
{
    return new LocalDvrInfo(*this);
}

void LocalDvrInfo::closeEfd()
{
    ::close(efd);
    efd = -1;
    eio.stop();
}

void LocalDvrInfo::closePid()
{
    pid = 0;
    pidwatcher.stop();
}

void LocalDvrInfo::onEfdEvent(ev::io &, int revents)
{
    if (EV_ERROR & revents)
    {
        int sockErrno = readFdError(this->efd);
        if (sockErrno)
        {
            LOG_F(ERROR, "Error on stderr: %s\n", strerror(sockErrno));
            closeEfd();
        }
        return;
    }

    if (revents & EV_READ)
    {
        ssize_t nr;

        /* read more */
        nr = read(efd, errbuff + errbuffpos, sizeof(errbuff) - errbuffpos);
        if (nr <= 0)
        {
            if (nr < 0)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    return;

                LOG_F(ERROR, "stderr %s\n", strerror(errno));
            }
            else
                LOG_F(ERROR, "stderr EOF\n");
            closeEfd();
            return;
        }
        errbuffpos += nr;

        for (int i = 0; i < errbuffpos; ++i)
        {
            if (errbuff[i] == '\n')
            {
                LOG_F(ERROR, "%.*s\n", (int)i, errbuff);
                i++;                                       /* count including nl */
                errbuffpos -= i;                           /* remove from nexbuf */
                memmove(errbuff, errbuff + i, errbuffpos); /* slide remaining to front */
                i = -1;                                    /* restart for loop scan */
            }
        }
    }
}

#ifdef _WIN32
void LocalDvrInfo::onPidEvent(ev_child &, int revents)
#else
void LocalDvrInfo::onPidEvent(ev::child &, int revents)
#endif
{
    if (revents & EV_CHILD)
    {
        if (WIFEXITED(pidwatcher.rstatus))
        {
            LOG_F(ERROR, "process %d exited with status %d\n", pid, WEXITSTATUS(pidwatcher.rstatus));
        }
        else if (WIFSIGNALED(pidwatcher.rstatus))
        {
            int signum = WTERMSIG(pidwatcher.rstatus);
            LOG_F(ERROR, "process %d killed with signal %d - %s\n", pid, signum, strsignal(signum));
        }
        pid = 0;
        this->pidwatcher.stop();
    }
}

RemoteDvrInfo::RemoteDvrInfo() : DvrInfo(false)
{
}

RemoteDvrInfo::RemoteDvrInfo(const RemoteDvrInfo &model) : DvrInfo(model),
                                                           host(model.host),
                                                           port(model.port)
{
}

RemoteDvrInfo::~RemoteDvrInfo()
{
}

RemoteDvrInfo *RemoteDvrInfo::clone() const
{
    return new RemoteDvrInfo(*this);
}
