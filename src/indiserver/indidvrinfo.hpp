#pragma once

#include "indimsg.hpp"
#include "indismsg.hpp"
#include "indiserver.hpp"

class DvrInfo;
class Property;
template <class M>
class ConcurrentSet;

/* info for each connected client */
class ClInfo : public MsgQueue
{
protected:
    /* send message to each appropriate driver.
     * also send all newXXX() to all other interested clients.
     */
    virtual void onMessage(XMLEle *root, std::list<int> &sharedBuffers);

    /* Update the client property BLOB handling policy */
    void crackBLOBHandling(const std::string &dev, const std::string &name, const char *enableBLOB);

    /* close down the given client */
    virtual void close();

public:
    std::list<Property *> props; /* props we want */
    int allprops = 0;            /* saw getProperties w/o device */
    BLOBHandling blob = B_NEVER; /* when to send setBLOBs */

    ClInfo(bool useSharedBuffer);
    virtual ~ClInfo();

    /* return 0 if cp may be interested in dev/name else -1
     */
    int findDevice(const std::string &dev, const std::string &name) const;

    /* add the given device and property to the props[] list of client if new.
     */
    void addDevice(const std::string &dev, const std::string &name, int isblob);

    virtual void log(const std::string &log) const;

    /* put Msg mp on queue of each chained server client, except notme.
     */
    static void q2Servers(DvrInfo *me, Msg *mp, XMLEle *root);

    /* put Msg mp on queue of each client interested in dev/name, except notme.
     * if BLOB always honor current mode.
     */
    static void q2Clients(ClInfo *notme, int isblob, const std::string &dev, const std::string &name, Msg *mp, XMLEle *root);

    /* Reference to all active clients */
    static ConcurrentSet<ClInfo> clients;
};

/* info for each connected driver */
class DvrInfo : public MsgQueue
{
    /* add dev/name to this device's snooping list.
     * init with blob mode set to B_NEVER.
     */
    void addSDevice(const std::string &dev, const std::string &name);

public:
    /* return Property if dp is this driver is snooping dev/name, else NULL.
     */
    Property *findSDevice(const std::string &dev, const std::string &name) const;

protected:
    /* send message to each interested client
     */
    virtual void onMessage(XMLEle *root, std::list<int> &sharedBuffers);

    /* override to kill driver that are not reachable anymore */
    virtual void closeWritePart();

    /* Construct an instance that will start the same driver */
    DvrInfo(const DvrInfo &model);

public:
    std::string name; /* persistent name */

    std::set<std::string> dev;    /* device served by this driver */
    std::list<Property *> sprops; /* props we snoop */
    int restarts;                 /* times process has been restarted */
    bool restart = true;          /* Restart on shutdown */

    DvrInfo(bool useSharedBuffer);
    virtual ~DvrInfo();

    bool isHandlingDevice(const std::string &dev) const;

    /* start the INDI driver process or connection.
     * exit if trouble.
     */
    virtual void start() = 0;

    /* close down the given driver and restart if set*/
    virtual void close();

    /* Allocate an instance that will start the same driver */
    virtual DvrInfo *clone() const = 0;

    virtual void log(const std::string &log) const;

    virtual const std::string remoteServerUid() const = 0;

    /* put Msg mp on queue of each driver responsible for dev, or all drivers
     * if dev empty.
     */
    static void q2RDrivers(const std::string &dev, Msg *mp, XMLEle *root);

    /* put Msg mp on queue of each driver snooping dev/name.
     * if BLOB always honor current mode.
     */
    static void q2SDrivers(DvrInfo *me, int isblob, const std::string &dev, const std::string &name, Msg *mp, XMLEle *root);

    /* Reference to all active drivers */
    static ConcurrentSet<DvrInfo> drivers;

    // decoding of attached blobs from driver is not supported ATM. Be conservative here
    virtual bool acceptSharedBuffers() const
    {
        return false;
    }
};

class LocalDvrInfo : public DvrInfo
{
    char errbuff[1024]; /* buffer for stderr pipe. line too long will be clipped */
    int errbuffpos = 0; /* first free pos in buffer */
    ev::io eio;         /* Event loop io events */
    ev::child pidwatcher;
    void onEfdEvent(ev::io &watcher, int revents); /* callback for data on efd */
    void onPidEvent(ev::child &watcher, int revents);

    int pid = 0;  /* process id or 0 for N/A (not started/terminated) */
    int efd = -1; /* stderr from driver, or -1 when N/A */

    void closeEfd();
    void closePid();

protected:
    LocalDvrInfo(const LocalDvrInfo &model);

public:
    std::string envDev;
    std::string envConfig;
    std::string envSkel;
    std::string envPrefix;

    LocalDvrInfo();
    virtual ~LocalDvrInfo();

    virtual void start();

    virtual LocalDvrInfo *clone() const;

    virtual const std::string remoteServerUid() const
    {
        return "";
    }
};
