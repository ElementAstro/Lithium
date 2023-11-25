#include "serialize.hpp"

#include <thread>
#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

#include <unistd.h>

#include "base64.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

#include "io.hpp"
#include "message.hpp"
#include "message_queue.hpp"
#include "xml_util.hpp"
#include "hydrogen_server.hpp"

#include "atom/log/loguru.hpp"

SerializedMsg::SerializedMsg(Msg *parent) : asyncProgress(), owner(parent), awaiters(), chuncks(), ownBuffers()
{
    blockedProducer = nullptr;
    // At first, everything is required.
    for (auto fd : parent->sharedBuffers)
    {
        if (fd != -1)
        {
            requirements.sharedBuffers.insert(fd);
        }
    }
    requirements.xml = true;
    asyncStatus = PENDING;
#ifdef USE_LIBUV
    uv_async_init(loop, &asyncProgress, (uv_async_cb)&SerializedMsg::async_progressed);
    asyncProgress.data = this;
#else
    asyncProgress.set<SerializedMsg, &SerializedMsg::async_progressed>(this);
#endif
}

// Delete occurs when no async task is running and no awaiters are left
SerializedMsg::~SerializedMsg()
{
    for (auto buff : ownBuffers)
    {
        free(buff);
    }
}

bool SerializedMsg::async_canceled()
{
    std::lock_guard<std::recursive_mutex> guard(lock);
    return asyncStatus == CANCELING;
}

void SerializedMsg::async_updateRequirement(const SerializationRequirement &req)
{
    std::lock_guard<std::recursive_mutex> guard(lock);
    if (this->requirements == req)
    {
        return;
    }
    this->requirements = req;
#ifdef USE_LIBUV
    uv_async_send(&asyncProgress);
#else
    asyncProgress.send();
#endif
}

void SerializedMsg::async_pushChunck(const MsgChunck &m)
{
    std::lock_guard<std::recursive_mutex> guard(lock);

    this->chuncks.push_back(m);
#ifdef USE_LIBUV
    uv_async_send(&asyncProgress);
#else
    asyncProgress.send();
#endif
}

void SerializedMsg::async_done()
{
    std::lock_guard<std::recursive_mutex> guard(lock);
    asyncStatus = TERMINATED;
#ifdef USE_LIBUV
    uv_async_send(&asyncProgress);
#else
    asyncProgress.send();
#endif
}

void SerializedMsg::async_start()
{
    std::lock_guard<std::recursive_mutex> guard(lock);
    if (asyncStatus != PENDING)
    {
        return;
    }

    asyncStatus = RUNNING;
    if (generateContentAsync())
    {
#ifdef USE_LIBUV
        uv_async_send(&asyncProgress);
#else
        asyncProgress.start();
#endif

        std::thread t([this]()
                      { generateContent(); });
        t.detach();
    }
    else
    {
        generateContent();
    }
}

void SerializedMsg::async_progressed()
{
    std::lock_guard<std::recursive_mutex> guard(lock);

    if (asyncStatus == TERMINATED)
    {
// FIXME: unblock ?
#ifdef USE_LIBUV
        uv_close((uv_handle_t *)&this->asyncProgress, NULL);
#else
        asyncProgress.stop();
#endif
    }

    // Update ios of awaiters
    for (auto awaiter : awaiters)
    {
        awaiter->messageMayHaveProgressed(this);
    }

    // Then prune
    owner->prune();
}

bool SerializedMsg::isAsyncRunning()
{
    std::lock_guard<std::recursive_mutex> guard(lock);

    return (asyncStatus == RUNNING) || (asyncStatus == CANCELING);
}

bool SerializedMsg::requestContent(const MsgChunckIterator &position)
{
    std::lock_guard<std::recursive_mutex> guard(lock);

    if (asyncStatus == PENDING)
    {
        async_start();
    }

    if (asyncStatus == TERMINATED)
    {
        return true;
    }

    // Not reached the last chunck
    if (position.chunckId < chuncks.size())
    {
        return true;
    }

    return false;
}

bool SerializedMsg::getContent(MsgChunckIterator &from, void *&data, ssize_t &size,
                               std::vector<int, std::allocator<int>> &sharedBuffers)
{
    std::lock_guard<std::recursive_mutex> guard(lock);

    if (asyncStatus != TERMINATED && from.chunckId >= chuncks.size())
    {
        // Not ready yet
        return false;
    }

    if (from.chunckId == chuncks.size())
    {
        // Done
        data = 0;
        size = 0;
        from.endReached = true;
        return true;
    }

    const MsgChunck &ck = chuncks[from.chunckId];

    if (from.chunckOffset == 0)
    {
        sharedBuffers = ck.sharedBufferIdsToAttach;
    }
    else
    {
        sharedBuffers.clear();
    }

    data = ck.content + from.chunckOffset;
    size = ck.contentLength - from.chunckOffset;
    return true;
}

void SerializedMsg::advance(MsgChunckIterator &iter, ssize_t s)
{
    std::lock_guard<std::recursive_mutex> guard(lock);

    MsgChunck &cur = chuncks[iter.chunckId];
    iter.chunckOffset += s;
    if (iter.chunckOffset >= cur.contentLength)
    {
        iter.chunckId++;
        iter.chunckOffset = 0;
        if (iter.chunckId >= chuncks.size() && asyncStatus == TERMINATED)
        {
            iter.endReached = true;
        }
    }
}

void SerializedMsg::addAwaiter(MsgQueue *q)
{
    awaiters.insert(q);
}

void SerializedMsg::release(MsgQueue *q)
{
    awaiters.erase(q);
    if (awaiters.empty() && !isAsyncRunning())
    {
        owner->releaseSerialization(this);
    }
}

void SerializedMsg::collectRequirements(SerializationRequirement &sr)
{
    sr.add(requirements);
}

// This is called when a received message require additional // work, to avoid overflow
void SerializedMsg::blockReceiver(MsgQueue *receiver)
{
    // TODO : implement or remove
    (void)receiver;
}

ssize_t SerializedMsg::queueSize()
{
    return owner->queueSize;
}

SerializedMsgWithoutSharedBuffer::SerializedMsgWithoutSharedBuffer(Msg *parent) : SerializedMsg(parent)
{
}

SerializedMsgWithoutSharedBuffer::~SerializedMsgWithoutSharedBuffer()
{
}

SerializedMsgWithSharedBuffer::SerializedMsgWithSharedBuffer(Msg *parent) : SerializedMsg(parent), ownSharedBuffers()
{
}

SerializedMsgWithSharedBuffer::~SerializedMsgWithSharedBuffer()
{
    for (auto id : ownSharedBuffers)
    {
        close(id);
    }
}

bool SerializedMsgWithSharedBuffer::detectInlineBlobs()
{
    for (auto blobContent : findBlobElements(owner->xmlContent))
    {
        // C'est pas trivial, dans ce cas, car il faut les réattacher
        std::string attached = findXMLAttValu(blobContent, "attached");
        if (attached != "true")
        {
            return true;
        }
    }
    return false;
}

bool SerializedMsgWithoutSharedBuffer::generateContentAsync() const
{
    return owner->hasInlineBlobs || owner->hasSharedBufferBlobs;
}

void SerializedMsgWithoutSharedBuffer::generateContent()
{
    // Convert every shared buffer into an inline base64
    auto xmlContent = owner->xmlContent;

    std::vector<XMLEle *> cdata;
    // Every cdata will have either sharedBuffer or sharedCData
    std::vector<int> sharedBuffers;
    std::vector<ssize_t> xmlSizes;
    std::vector<XMLEle *> sharedCData;

    std::unordered_map<XMLEle *, XMLEle *> replacement;

    int ownerSharedBufferId = 0;

    // Identify shared buffer blob to base64 them
    // Identify base64 blob to avoid copying them (we'll copy the cdata)
    for (auto blobContent : findBlobElements(xmlContent))
    {
        std::string attached = findXMLAttValu(blobContent, "attached");

        if (attached != "true" && pcdatalenXMLEle(blobContent) == 0)
        {
            continue;
        }

        XMLEle *clone = shallowCloneXMLEle(blobContent);
        rmXMLAtt(clone, "attached");
        editXMLEle(clone, "_");

        replacement[blobContent] = clone;
        cdata.push_back(clone);

        if (attached == "true")
        {
            rmXMLAtt(clone, "enclen");

            // Get the size if present
            ssize_t size = -1;
            parseBlobSize(clone, size);

            // FIXME: we could add enclen there

            // Put something here for later replacement
            sharedBuffers.push_back(owner->sharedBuffers[ownerSharedBufferId++]);
            xmlSizes.push_back(size);
            sharedCData.push_back(nullptr);
        }
        else
        {
            sharedBuffers.push_back(-1);
            xmlSizes.push_back(-1);
            sharedCData.push_back(blobContent);
        }
    }

    if (replacement.empty())
    {
        // Just print the content as is...

        char *model = (char *)malloc(sprlXMLEle(xmlContent, 0) + 1);
        int modelSize = sprXMLEle(model, xmlContent, 0);

        ownBuffers.push_back(model);

        async_pushChunck(MsgChunck(model, modelSize));

        // FIXME: lower requirements asap... how to do that ?
        // requirements.xml = false;
        // requirements.sharedBuffers.clear();
    }
    else
    {
        // Create a replacement that shares original CData buffers
        xmlContent = cloneXMLEleWithReplacementMap(xmlContent, replacement);

        std::vector<size_t> modelCdataOffset(cdata.size());

        char *model = (char *)malloc(sprlXMLEle(xmlContent, 0) + 1);
        int modelSize = sprXMLEle(model, xmlContent, 0);

        ownBuffers.push_back(model);

        // Get the element offset
        for (std::size_t i = 0; i < cdata.size(); ++i)
        {
            modelCdataOffset[i] = sprXMLCDataOffset(xmlContent, cdata[i], 0);
        }
        delXMLEle(xmlContent);

        std::vector<int> fds(cdata.size());
        std::vector<void *> blobs(cdata.size());
        std::vector<size_t> sizes(cdata.size());
        std::vector<size_t> attachedSizes(cdata.size());

        // Attach all blobs
        for (std::size_t i = 0; i < cdata.size(); ++i)
        {
            if (sharedBuffers[i] != -1)
            {
                fds[i] = sharedBuffers[i];

                size_t dataSize;
#ifdef _WIN32
                blobs[i] = attachSharedBuffer((HANDLE)_get_osfhandle(fds[i]), dataSize);
#else
                blobs[i] = attachSharedBuffer(fds[i], dataSize);
#endif
                attachedSizes[i] = dataSize;

                // check dataSize is compatible with the blob element's size
                // It's mandatory for attached blob to give their size
                if (xmlSizes[i] != -1 && ((size_t)xmlSizes[i]) <= dataSize)
                {
                    dataSize = xmlSizes[i];
                }
                sizes[i] = dataSize;
            }
            else
            {
                fds[i] = -1;
            }
        }

        // Copy from model or blob (streaming base64 encode)
        int modelOffset = 0;
        for (std::size_t i = 0; i < cdata.size(); ++i)
        {
            int cdataOffset = modelCdataOffset[i];
            if (cdataOffset > modelOffset)
            {
                async_pushChunck(MsgChunck(model + modelOffset, cdataOffset - modelOffset));
            }
            // Skip the dummy cdata completly
            modelOffset = cdataOffset + 1;

            // Perform inplace base64
            // FIXME: could be streamed/splitted

            if (fds[i] != -1)
            {
                // Add a binary chunck. This needs base64 convertion
                // FIXME: the size here should be the size of the blob element
                unsigned long buffSze = sizes[i];
                const unsigned char *src = (const unsigned char *)blobs[i];

                // split here in smaller chuncks for faster startup
                // This allow starting write before the whole blob is converted
                while (buffSze > 0)
                {
                    // We need a block size multiple of 24 bits (3 bytes)
                    unsigned long sze = buffSze > 3 * 16384 ? 3 * 16384 : buffSze;

                    char *buffer = (char *)malloc(4 * sze / 3 + 4);
                    ownBuffers.push_back(buffer);
                    int base64Count = to64frombits_s((unsigned char *)buffer, src, sze, (4 * sze / 3 + 4));

                    async_pushChunck(MsgChunck(buffer, base64Count));

                    buffSze -= sze;
                    src += sze;
                }

                // Dettach blobs ASAP
#ifdef _WIN32
                detachSharedBuffer(blobs[i]);
#else
                detachSharedBuffer(fds[i], blobs[i], attachedSizes[i]);
#endif
                // requirements.sharedBuffers.erase(fds[i]);
            }
            else
            {
                // Add an already ready cdata section

                auto len = pcdatalenXMLEle(sharedCData[i]);
                auto data = pcdataXMLEle(sharedCData[i]);
                async_pushChunck(MsgChunck(data, len));
            }
        }

        if (modelOffset < modelSize)
        {
            async_pushChunck(MsgChunck(model + modelOffset, modelSize - modelOffset));
            modelOffset = modelSize;
        }
    }
    async_done();
}

bool SerializedMsgWithSharedBuffer::generateContentAsync() const
{
    return owner->hasInlineBlobs;
}

void SerializedMsgWithSharedBuffer::generateContent()
{
    // Convert every inline base64 blob from xml into an attached buffer
    auto xmlContent = owner->xmlContent;

    std::vector<int> sharedBuffers = owner->sharedBuffers;

    std::unordered_map<XMLEle *, XMLEle *> replacement;
    int blobPos = 0;
    for (auto blobContent : findBlobElements(owner->xmlContent))
    {
        if (!pcdatalenXMLEle(blobContent))
        {
            continue;
        }
        std::string attached = findXMLAttValu(blobContent, "attached");
        if (attached != "true")
        {
            // We need to replace.
            XMLEle *clone = shallowCloneXMLEle(blobContent);
            rmXMLAtt(clone, "enclen");
            rmXMLAtt(clone, "attached");
            addXMLAtt(clone, "attached", "true");

            replacement[blobContent] = clone;

            int base64datalen = pcdatalenXMLEle(blobContent);
            char *base64data = pcdataXMLEle(blobContent);
            // Shall we really trust the size here ?

            ssize_t size;
            if (!parseBlobSize(blobContent, size))
            {
                DLOG_F(WARNING, "Missing size value for blob");
                size = 1;
            }

            void *blob = IDSharedBlobAlloc(size);
            if (blob == nullptr)
            {
                LOG_F(ERROR, "Unable to allocate shared buffer of size %ld : %s\n", size, strerror(errno));
                ::exit(1);
            }
            DLOG_F(INFO, "Blob allocated at %p\n", blob);

            int actualLen = from64tobits_fast((char *)blob, base64data, base64datalen);

            if (actualLen != size)
            {
                // FIXME: WTF ? at least prevent overflow ???
                DLOG_F(INFO, "Blob size mismatch after base64dec: %lld vs %lld\n", (long long int)actualLen, (long long int)size);
            }

            int newFd = IDSharedBlobGetFd(blob);
            ownSharedBuffers.insert(newFd);

            IDSharedBlobDettach(blob);

            sharedBuffers.insert(sharedBuffers.begin() + blobPos, newFd);
        }
        blobPos++;
    }

    if (!replacement.empty())
    {
        // Work on a copy --- but we don't want to copy the blob !!!
        xmlContent = cloneXMLEleWithReplacementMap(xmlContent, replacement);
    }

    // Now create a Chunk from xmlContent
    MsgChunck chunck;

    chunck.content = (char *)malloc(sprlXMLEle(xmlContent, 0) + 1);
    ownBuffers.push_back(chunck.content);
    chunck.contentLength = sprXMLEle(chunck.content, xmlContent, 0);
    chunck.sharedBufferIdsToAttach = sharedBuffers;

    async_pushChunck(chunck);

    if (!replacement.empty())
    {
        delXMLEle(xmlContent);
    }
    async_done();
}
