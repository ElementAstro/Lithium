#include "indidvrinfo.hpp"

/* start the given local INDI driver process.
 * exit if trouble.
 */
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
        // FIXME: lots of FD are opened by indiserver. FD_CLOEXEC is a must + check other fds
        if (socketpair(AF_UNIX, SOCK_STREAM, 0, ux) == -1)
        {
            spdlog::error("socketpair: {}", strerror(errno));
            Bye();
        }
    }
    else
    {
        if (pipe(rp) < 0)
        {
            spdlog::error("read pipe: {}", strerror(errno));
            Bye();
        }
        if (pipe(wp) < 0)
        {
            spdlog::error("write pipe: {}", strerror(errno));
            Bye();
        }
    }
    if (pipe(ep) < 0)
    {
        spdlog::error("stderr pipe: {}", strerror(errno));
        Bye();
    }

    /* fork&exec new process */
    pid = fork();
    if (pid < 0)
    {
        spdlog::error("fork: {}", strerror(errno));
        Bye();
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
            setenv("INDIDEV", envDev.c_str(), 1);
        if (!envConfig.empty())
            setenv("INDICONFIG", envConfig.c_str(), 1);
        if (!envSkel.empty())
            setenv("INDISKEL", envSkel.c_str(), 1);
        std::string executable;
        if (!envPrefix.empty())
        {
            setenv("INDIPREFIX", envPrefix.c_str(), 1);
            executable = envPrefix + "/bin/" + name;
            spdlog::debug("{}", executable.c_str());
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
        spdlog::info("execlp {}: {}", executable.c_str(), strerror(errno));
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
        spdlog::info("pid={} rfd={} wfd={} efd={}", pid, rp[0], wp[1], ep[0]);

    XMLEle *root = addXMLEle(NULL, "getProperties");
    addXMLAtt(root, "version", TO_STRING(INDIV));
    mp = new Msg(nullptr, root);

    // pushmsg can kill mp. do at end
    pushMsg(mp);
}

// root will be released
void ClInfo::onMessage(XMLEle *root, std::list<int> &sharedBuffers)
{
    char *roottag = tagXMLEle(root);

    const char *dev = findXMLAttValu(root, "device");
    const char *name = findXMLAttValu(root, "name");
    int isblob = !strcmp(tagXMLEle(root), "setBLOBVector");

    /* snag interested properties.
     * N.B. don't open to alldevs if seen specific dev already, else
     *   remote client connections start returning too much.
     */
    if (dev[0])
    {
        // Signature for CHAINED SERVER
        // Not a regular client.
        if (dev[0] == '*' && !this->props.size())
            this->allprops = 2;
        else
            addDevice(dev, name, isblob);
    }
    else if (!strcmp(roottag, "getProperties") && !this->props.size() && this->allprops != 2)
        this->allprops = 1;

    /* snag enableBLOB -- send to remote drivers too */
    if (!strcmp(roottag, "enableBLOB"))
        crackBLOBHandling(dev, name, pcdataXMLEle(root));

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
        spdlog::info("Closing after malformed message");
        close();
        return;
    }

    /* send message to driver(s) responsible for dev */
    DvrInfo::q2RDrivers(dev, mp, root);

    /* JM 2016-05-18: Upstream client can be a chained INDI server. If any driver locally is snooping
     * on any remote drivers, we should catch it and forward it to the responsible snooping driver. */
    /* send to snooping drivers. */
    // JM 2016-05-26: Only forward setXXX messages
    if (!strncmp(roottag, "set", 3))
        DvrInfo::q2SDrivers(NULL, isblob, dev, name, mp, root);

    /* echo new* commands back to other clients */
    if (!strncmp(roottag, "new", 3))
    {
        q2Clients(this, isblob, dev, name, mp, root);
    }

    mp->queuingDone();
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
        spdlog::info("read <{} device='{}' name='{}'>",
                     tagXMLEle(root), findXMLAttValu(root, "device"), findXMLAttValu(root, "name"));
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

void ClInfo::close()
{
    if (verbose > 0)
        spdlog::info("shut down complete - bye!");

    delete (this);
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
            spdlog::error("Terminated after #{} restarts.", restarts);
            terminate = true;
        }
        else
        {
            spdlog::error("restart #{}", restarts);
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
        if (drivers.ids().empty())
            Bye();
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

        /* ok: queue message to this driver */
        if (verbose > 1)
        {
            spdlog::debug("queuing responsible for <{} device='{}' name='{}'>",
                          tagXMLEle(root), findXMLAttValu(root, "device"), findXMLAttValu(root, "name"));
        }

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

        /* ok: queue message to this device */
        if (verbose > 1)
        {
            spdlog::debug("queuing snooped <{} device='{}' name='{}'>",
                          tagXMLEle(root), findXMLAttValu(root, "device"), findXMLAttValu(root, "name"));
        }

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
        spdlog::info("snooping on {}.{}", dev.c_str(), name.c_str());
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

void ClInfo::q2Clients(ClInfo *notme, int isblob, const std::string &dev, const std::string &name, Msg *mp, XMLEle *root)
{
    /* queue message to each interested client */
    for (auto cpId : clients.ids())
    {
        auto cp = clients[cpId];
        if (cp == nullptr)
            continue;

        /* cp in use? notme? want this dev/name? blob? */
        if (cp == notme)
            continue;
        if (cp->findDevice(dev, name) < 0)
            continue;

        // if ((isblob && cp->blob==B_NEVER) || (!isblob && cp->blob==B_ONLY))
        if (!isblob && cp->blob == B_ONLY)
            continue;

        if (isblob)
        {
            if (cp->props.size() > 0)
            {
                Property *blobp = nullptr;
                for (auto pp : cp->props)
                {
                    if (pp->dev == dev && pp->name == name)
                    {
                        blobp = pp;
                        break;
                    }
                }

                if ((blobp && blobp->blob == B_NEVER) || (!blobp && cp->blob == B_NEVER))
                    continue;
            }
            else if (cp->blob == B_NEVER)
                continue;
        }

        /* shut down this client if its q is already too large */
        unsigned long ql = cp->msgQSize();
        if (isblob && maxstreamsiz > 0 && ql > maxstreamsiz)
        {
            // Drop frames for streaming blobs
            /* pull out each name/BLOB pair, decode */
            XMLEle *ep = NULL;
            int streamFound = 0;
            for (ep = nextXMLEle(root, 1); ep; ep = nextXMLEle(root, 0))
            {
                if (strcmp(tagXMLEle(ep), "oneBLOB") == 0)
                {
                    XMLAtt *fa = findXMLAtt(ep, "format");

                    if (fa && strstr(valuXMLAtt(fa), "stream"))
                    {
                        streamFound = 1;
                        break;
                    }
                }
            }
            if (streamFound)
            {
                if (verbose > 1)
                    spdlog::info("{} bytes behind. Dropping stream BLOB...", ql);
                continue;
            }
        }
        if (ql > maxqsiz)
        {
            if (verbose)
                spdlog::info("{} bytes behind, shutting down", ql);
            cp->close();
            continue;
        }

        if (verbose > 1)
            spdlog::info("queuing <{} device='{}' name='{}'>",
                         tagXMLEle(root), findXMLAttValu(root, "device"), findXMLAttValu(root, "name"));

        // pushmsg can kill cp. do at end
        cp->pushMsg(mp);
    }

    return;
}

void ClInfo::q2Servers(DvrInfo *me, Msg *mp, XMLEle *root)
{
    int devFound = 0;

    /* queue message to each interested client */
    for (auto cpId : clients.ids())
    {
        auto cp = clients[cpId];
        if (cp == nullptr)
            continue;

        // Only send the message to the upstream server that is connected specfically to the device in driver dp
        switch (cp->allprops)
        {
        // 0 --> not all props are requested. Check for specific combination
        case 0:
            for (auto pp : cp->props)
            {
                if (me->dev.find(pp->dev) != me->dev.end())
                {
                    devFound = 1;
                    break;
                }
            }
            break;

        // All props are requested. This is client-only mode (not upstream server)
        case 1:
            break;
        // Upstream server mode
        case 2:
            devFound = 1;
            break;
        }

        // If no matching device found, continue
        if (devFound == 0)
            continue;

        /* shut down this client if its q is already too large */
        unsigned long ql = cp->msgQSize();
        if (ql > maxqsiz)
        {
            if (verbose)
                spdlog::info("{} bytes behind, shutting down", ql);
            cp->close();
            continue;
        }

        /* ok: queue message to this client */
        if (verbose > 1)
            spdlog::info("queuing <{} device='{}' name='{}'>",
                         tagXMLEle(root), findXMLAttValu(root, "device"), findXMLAttValu(root, "name"));

        // pushmsg can kill cp. do at end
        cp->pushMsg(mp);
    }
}

int ClInfo::findDevice(const std::string &dev, const std::string &name) const
{
    if (allprops >= 1 || dev.empty())
        return (0);
    for (auto pp : props)
    {
        if ((pp->dev == dev) && (pp->name.empty() || (pp->name == name)))
            return (0);
    }
    return (-1);
}

void ClInfo::addDevice(const std::string &dev, const std::string &name, int isblob)
{
    if (isblob)
    {
        for (auto pp : props)
        {
            if (pp->dev == dev && pp->name == name)
                return;
        }
    }
    /* no dups */
    else if (!findDevice(dev, name))
        return;

    /* add */
    Property *pp = new Property(dev, name);
    props.push_back(pp);
}

void ClInfo::crackBLOBHandling(const std::string &dev, const std::string &name, const char *enableBLOB)
{
    /* If we have EnableBLOB with property name, we add it to Client device list */
    if (!name.empty())
        addDevice(dev, name, 1);
    else
        /* Otherwise, we set the whole client blob handling to what's passed (enableBLOB) */
        crackBLOB(enableBLOB, &blob);

    /* If whole client blob handling policy was updated, we need to pass that also to all children
       and if the request was for a specific property, then we apply the policy to it */
    for (auto pp : props)
    {
        if (name.empty())
            crackBLOB(enableBLOB, &pp->blob);
        else if (pp->dev == dev && pp->name == name)
        {
            crackBLOB(enableBLOB, &pp->blob);
            return;
        }
    }
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
    spdlog::debug(logLine);
}

ConcurrentSet<DvrInfo> DvrInfo::drivers;

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
            spdlog::error("Error on stderr: {}", strerror(sockErrno));
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

                spdlog::info("stderr {}", strerror(errno));
            }
            else
                spdlog::error("stderr EOF");
            closeEfd();
            return;
        }
        errbuffpos += nr;

        for (int i = 0; i < errbuffpos; ++i)
        {
            if (errbuff[i] == '\n')
            {
                spdlog::info("{}", (int)i, errbuff);
                i++;                                       /* count including nl */
                errbuffpos -= i;                           /* remove from nexbuf */
                memmove(errbuff, errbuff + i, errbuffpos); /* slide remaining to front */
                i = -1;                                    /* restart for loop scan */
            }
        }
    }
}

void LocalDvrInfo::onPidEvent(ev::child &, int revents)
{
    if (revents & EV_CHILD)
    {
        if (WIFEXITED(pidwatcher.rstatus))
        {
            spdlog::info("process {} exited with status {}", pid, WEXITSTATUS(pidwatcher.rstatus));
        }
        else if (WIFSIGNALED(pidwatcher.rstatus))
        {
            int signum = WTERMSIG(pidwatcher.rstatus);
            spdlog::info("process {} killed with signal {} - {}", pid, signum, strsignal(signum));
        }
        pid = 0;
        this->pidwatcher.stop();
    }
}

ClInfo::ClInfo(bool useSharedBuffer) : MsgQueue(useSharedBuffer)
{
    clients.insert(this);
}

ClInfo::~ClInfo()
{
    for (auto prop : props)
    {
        delete prop;
    }

    clients.erase(this);
}

void ClInfo::log(const std::string &str) const
{
    spdlog::debug("Client {}: ", this->getRFd());
}