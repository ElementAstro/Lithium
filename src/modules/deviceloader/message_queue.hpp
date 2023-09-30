#pragma once

#include <set>
#include <list>

#include "lilxml.hpp"
#include "hydrogendevapi.hpp"

#include "message.hpp"
#include "concurrent.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef USE_LIBUV
#include <uv.h>
#else
#include <ev++.h>
#endif

class MsgQueue : public Collectable
{
    int rFd, wFd;
    LilXML *lp;      /* XML parsing context */
#ifdef USE_LIBUV
    uv_poll_t rio, wio; // Event loop io events
    void ioCb(uv_poll_t *handle, int status, int revents);
    static void IOCallback(uv_poll_t* handle, int status, int events);
    void setReadWriteCallback();
#else
    ev::io rio, wio; /* Event loop io events */
    void ioCb(ev::io &watcher, int revents);
#endif

    // Update the status of FD read/write ability
    void updateIos();

    std::set<SerializedMsg *> readBlocker; /* The message that block this queue */

    std::list<SerializedMsg *> msgq; /* To send msg queue */
#ifdef _WIN32
    std::list<HANDLE> incomingSharedBuffers;
#else
    std::list<int> incomingSharedBuffers; /* During reception, fds accumulate here */
#endif

    // Position in the head message
    MsgChunckIterator nsent;

    // Handle fifo or socket case
    size_t doRead(char *buff, size_t len);
    void readFromFd();

    /* write the next chunk of the current message in the queue to the given
     * client. pop message from queue when complete and free the message if we are
     * the last one to use it. shut down this client if trouble.
     */
    void writeToFd();

protected:
    bool useSharedBuffer;
    int getRFd() const
    {
        return rFd;
    }
    int getWFd() const
    {
        return wFd;
    }

    /* print key attributes and values of the given xml to stderr. */
    void traceMsg(const std::string &log, XMLEle *root);

    /* Close the connection. (May be restarted later depending on driver logic) */
    virtual void close() = 0;

    /* Close the writing part of the connection. By default, shutdown the write part, but keep on reading. May delete this */
    virtual void closeWritePart();

    /* Handle a message. root will be freed by caller. fds of buffers will be closed, unless set to -1 */
    virtual void onMessage(XMLEle *root, std::list<int> &sharedBuffers) = 0;

    /* convert the string value of enableBLOB to our B_ state value.
     * no change if unrecognized
     */
    static void crackBLOB(const char *enableBLOB, BLOBHandling *bp);

    MsgQueue(bool useSharedBuffer);

public:
    virtual ~MsgQueue();

    void pushMsg(Msg *msg);

    /* return storage size of all Msqs on the given q */
    unsigned long msgQSize() const;

    SerializedMsg *headMsg() const;
    void consumeHeadMsg();

    /* Remove all messages from queue */
    void clearMsgQueue();

    void messageMayHaveProgressed(const SerializedMsg *msg);

    void setFds(int rFd, int wFd);

    virtual bool acceptSharedBuffers() const
    {
        return useSharedBuffer;
    }
};