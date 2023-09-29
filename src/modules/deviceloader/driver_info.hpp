#pragma once

#include <string>
#include <list>
#include <set>

#include "message_queue.hpp"
#include "property.hpp"
#include "concurrent.hpp"

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

    /* start the HYDROGEN driver process or connection.
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
