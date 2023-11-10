#pragma once

#include <string>
#include <list>

#include "concurrent.hpp"
#include "message_queue.hpp"

#include "hydrogendevapi.hpp"
#include "lilxml.hpp"

class Msg;
class Property;
class DvrInfo;

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