#pragma once

#include <string>
#include <vector>
#include <list>

#include "lilxml.h"

#include "serialize.hpp"

class MsgChunckIterator;
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

bool parseBlobSize(XMLEle *blobWithAttachedBuffer, ssize_t &size);