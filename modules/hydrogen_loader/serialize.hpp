#pragma once

#include <set>
#include <mutex>
#include <vector>
#include <list>

#ifdef USE_LIBUV
#include <uv.h>
#else
#include <ev++.h>
#endif

class Msg;
class MsgQueue;
class MsgChunck;
class MsgChunckIterator;

class SerializationRequirement
{
    friend class Msg;
    friend class SerializedMsg;

    // If the xml is still required
    bool xml;
    // Set of sharedBuffer that are still required
    std::set<int> sharedBuffers;

    SerializationRequirement() : sharedBuffers()
    {
        xml = false;
    }

    void add(const SerializationRequirement &from)
    {
        xml |= from.xml;
        for (auto fd : from.sharedBuffers)
        {
            sharedBuffers.insert(fd);
        }
    }

    bool operator==(const SerializationRequirement &sr) const
    {
        return (xml == sr.xml) && (sharedBuffers == sr.sharedBuffers);
    }
};

enum SerializationStatus
{
    PENDING,
    RUNNING,
    CANCELING,
    TERMINATED
};

class SerializedMsg
{
    friend class Msg;
    friend class MsgChunckIterator;

    std::recursive_mutex lock;
#ifdef USE_LIBUV
    uv_async_t asyncProgress;
#else
    ev::async asyncProgress;
#endif

    // Start a thread for execution of asyncRun
    void async_start();
    void async_cancel();

    // Called within main loop when async task did some progress
    void async_progressed();

    // The requirements. Prior to starting, everything is required.
    SerializationRequirement requirements;

    void produce(bool sync);

protected:
    // These methods are to be called from asyncRun
    bool async_canceled();
    void async_updateRequirement(const SerializationRequirement &n);
    void async_pushChunck(const MsgChunck &m);
    void async_done();

    // True if a producing thread is active
    bool isAsyncRunning();

protected:
    SerializationStatus asyncStatus;
    Msg *owner;

    MsgQueue *blockedProducer;

    std::set<MsgQueue *> awaiters;

private:
    std::vector<MsgChunck> chuncks;

protected:
    // Buffers malloced during asyncRun
    std::list<void *> ownBuffers;

    // This will notify awaiters and possibly release the owner
    void onDataReady();

    virtual bool generateContentAsync() const = 0;
    virtual void generateContent() = 0;

    void collectRequirements(SerializationRequirement &req);

    // The task will cancel itself if all owner release it
    void abort();

    // Make sure the given receiver will not be processed until this task complete
    // TODO : to implement + make sure the task start when it actually block something
    void blockReceiver(MsgQueue *toblock);

public:
    SerializedMsg(Msg *parent);
    virtual ~SerializedMsg();

    // Calling requestContent will start production
    // Return true if some content is available
    bool requestContent(const MsgChunckIterator &position);

    // Return true if some content is available
    // It is possible to have 0 to send, meaning end was actually reached
    bool getContent(MsgChunckIterator &position, void *&data, ssize_t &nsend, std::vector<int> &sharedBuffers);

    void advance(MsgChunckIterator &position, ssize_t s);

    // When a queue is done with sending this message
    void release(MsgQueue *from);

    void addAwaiter(MsgQueue *awaiter);

    ssize_t queueSize();
};

class SerializedMsgWithSharedBuffer : public SerializedMsg
{
    std::set<int> ownSharedBuffers;

protected:
    bool detectInlineBlobs();

public:
    SerializedMsgWithSharedBuffer(Msg *parent);
    virtual ~SerializedMsgWithSharedBuffer();

    virtual bool generateContentAsync() const;
    virtual void generateContent();
};

class SerializedMsgWithoutSharedBuffer : public SerializedMsg
{

public:
    SerializedMsgWithoutSharedBuffer(Msg *parent);
    virtual ~SerializedMsgWithoutSharedBuffer();

    virtual bool generateContentAsync() const;
    virtual void generateContent();
};