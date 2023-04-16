#pragma once

#include "indismsg.hpp"
#include "indidvrinfo.hpp"
#include "indiserver.hpp"
template <class M>
class ConcurrentSet
{
    unsigned long identifier = 1;
    std::map<unsigned long, M *> items;

public:
    void insert(M *item)
    {
        item->id = identifier++;
        items[item->id] = item;
        item->current = (ConcurrentSet<void> *)this;
    }

    void erase(M *item)
    {
        items.erase(item->id);
        item->id = 0;
        item->current = nullptr;
    }

    std::vector<unsigned long> ids() const
    {
        std::vector<unsigned long> result;
        for (auto item : items)
        {
            result.push_back(item.first);
        }
        return result;
    }

    M *operator[](unsigned long id) const
    {
        auto e = items.find(id);
        if (e == items.end())
        {
            return nullptr;
        }
        return e->second;
    }

    class iterator
    {
        friend class ConcurrentSet<M>;
        const ConcurrentSet<M> *parent;
        std::vector<unsigned long> ids;
        // Will be -1 when done
        long int pos = 0;

        void skip()
        {
            if (pos == -1)
                return;
            while (pos < (long int)ids.size() && !(*parent)[ids[pos]])
            {
                pos++;
            }
            if (pos == (long int)ids.size())
            {
                pos = -1;
            }
        }

    public:
        iterator(const ConcurrentSet<M> *parent) : parent(parent) {}

        bool operator!=(const iterator &o)
        {
            return pos != o.pos;
        }

        iterator &operator++()
        {
            if (pos != -1)
            {
                pos++;
                skip();
            }
            return *this;
        }

        M *operator*() const
        {
            return (*parent)[ids[pos]];
        }
    };

    iterator begin() const
    {
        iterator result(this);
        for (auto item : items)
        {
            result.ids.push_back(item.first);
        }
        result.skip();
        return result;
    }

    iterator end() const
    {
        iterator result(nullptr);
        result.pos = -1;
        return result;
    }
};

/* An object that can be put in a ConcurrentSet, and provide a heartbeat
 * to detect removal from ConcurrentSet
 */
class Collectable
{
    template <class P>
    friend class ConcurrentSet;
    unsigned long id = 0;
    const ConcurrentSet<void> *current;

    /* Keep the id */
    class HeartBeat
    {
        friend class Collectable;
        unsigned long id;
        const ConcurrentSet<void> *current;
        HeartBeat(unsigned long id, const ConcurrentSet<void> *current)
            : id(id), current(current) {}

    public:
        bool alive() const
        {
            return id != 0 && (*current)[id] != nullptr;
        }
    };

protected:
    /* heartbeat.alive will return true as long as this item has not changed collection.
     * Also detect deletion of the Collectable */
    HeartBeat heartBeat() const
    {
        return HeartBeat(id, current);
    }
};
/**
 * A MsgChunk is either:
 *  a raw xml fragment
 *  a ref to a shared buffer in the message
 */
class MsgChunck
{
    friend class SerializedMsg;
    friend class SerializedMsgWithSharedBuffer;
    friend class SerializedMsgWithoutSharedBuffer;
    friend class MsgChunckIterator;

    MsgChunck();
    MsgChunck(char *content, unsigned long length);

    char *content;
    unsigned long contentLength;

    std::vector<int> sharedBufferIdsToAttach;
};

class MsgChunckIterator
{
    friend class SerializedMsg;
    std::size_t chunckId;
    unsigned long chunckOffset;
    bool endReached;

public:
    MsgChunckIterator()
    {
        reset();
    }

    // Point to start of message.
    void reset()
    {
        chunckId = 0;
        chunckOffset = 0;
        // No risk of 0 length message, so always false here
        endReached = false;
    }

    bool done() const
    {
        return endReached;
    }
};

class MsgQueue : public Collectable
{
    int rFd, wFd;
    LilXML *lp;      /* XML parsing context */
    ev::io rio, wio; /* Event loop io events */
    void ioCb(ev::io &watcher, int revents);

    // Update the status of FD read/write ability
    void updateIos();

    std::set<SerializedMsg *> readBlocker; /* The message that block this queue */

    std::list<SerializedMsg *> msgq;      /* To send msg queue */
    std::list<int> incomingSharedBuffers; /* During reception, fds accumulate here */

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

    virtual void log(const std::string &log) const;
};

class Msg
{
    friend class SerializedMsg;
    friend class SerializedMsgWithSharedBuffer;
    friend class SerializedMsgWithoutSharedBuffer;

private:
    // Present for sure until message queing is doned. Prune asap then
    XMLEle *xmlContent;

    // Present until message was queued.
    MsgQueue *from;

    int queueSize;
    bool hasInlineBlobs;
    bool hasSharedBufferBlobs;

    std::vector<int> sharedBuffers; /* fds of shared buffer */

    // Convertion task and resultat of the task
    SerializedMsg *convertionToSharedBuffer;
    SerializedMsg *convertionToInline;

    SerializedMsg *buildConvertionToSharedBuffer();
    SerializedMsg *buildConvertionToInline();

    bool fetchBlobs(std::list<int> &incomingSharedBuffers);

    void releaseXmlContent();
    void releaseSharedBuffers(const std::set<int> &keep);

    // Remove resources that can be removed.
    // Will be called when queuingDone is true and for every change of staus from convertionToXXX
    void prune();

    void releaseSerialization(SerializedMsg *form);

    ~Msg();

public:
    /* Message will not be queued anymore. Release all possible resources, incl self */
    void queuingDone();

    Msg(MsgQueue *from, XMLEle *root);

    static Msg *fromXml(MsgQueue *from, XMLEle *root, std::list<int> &incomingSharedBuffers);

    /**
     * Handle multiple cases:
     *
     *  - inline => attached.
     * Exceptional. The inline is already in memory within xml. It must be converted to shared buffer async.
     * FIXME: The convertion should block the emitter.
     *
     *  - attached => attached
     * Default case. No convertion is required.
     *
     *  - inline => inline
     * Frequent on system not supporting attachment.
     *
     *  - attached => inline
     * Frequent. The convertion will be made during write. The convert/write must be offshored to a dedicated thread.
     *
     * The returned AsyncTask will be ready once "to" can write the message
     */
    SerializedMsg *serialize(MsgQueue *from);
};
