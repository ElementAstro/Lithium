/*
 * message.cpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-7-13

Description: Message

**************************************************/

#include "message.hpp"

#include <cassert>
#include <unistd.h>

#include "lilxml.hpp"

#include "message_queue.hpp"
#include "xml_util.hpp"

#include "atom/log/loguru.hpp"

MsgChunck::MsgChunck() : sharedBufferIdsToAttach()
{
    content = nullptr;
    contentLength = 0;
}

MsgChunck::MsgChunck(char *content, unsigned long length) : sharedBufferIdsToAttach()
{
    this->content = content;
    this->contentLength = length;
}

Msg::Msg(MsgQueue *from, XMLEle *ele) : sharedBuffers()
{
    this->from = from;
    xmlContent = ele;
    hasInlineBlobs = false;
    hasSharedBufferBlobs = false;

    convertionToSharedBuffer = nullptr;
    convertionToInline = nullptr;

    queueSize = sprlXMLEle(xmlContent, 0);
    for (auto blobContent : findBlobElements(xmlContent))
    {
        std::string attached = findXMLAttValu(blobContent, "attached");
        if (attached == "true")
        {
            hasSharedBufferBlobs = true;
        }
        else
        {
            hasInlineBlobs = true;
        }
    }
}

Msg::~Msg()
{
    // Assume convertionToSharedBlob and convertionToInlineBlob were already droped
    assert(convertionToSharedBuffer == nullptr);
    assert(convertionToInline == nullptr);

    releaseXmlContent();
    releaseSharedBuffers(std::set<int>());
}

void Msg::releaseSerialization(SerializedMsg *msg)
{
    if (msg == convertionToSharedBuffer)
    {
        convertionToSharedBuffer = nullptr;
    }

    if (msg == convertionToInline)
    {
        convertionToInline = nullptr;
    }

    delete (msg);
    prune();
}

void Msg::releaseXmlContent()
{
    if (xmlContent != nullptr)
    {
        delXMLEle(xmlContent);
        xmlContent = nullptr;
    }
}

void Msg::releaseSharedBuffers(const std::set<int> &keep)
{
    for (std::size_t i = 0; i < sharedBuffers.size(); ++i)
    {
        auto fd = sharedBuffers[i];
        if (fd != -1 && keep.find(fd) == keep.end())
        {
            if (close(fd) == -1)
            {
                perror("Releasing shared buffer");
            }
            sharedBuffers[i] = -1;
        }
    }
}

void Msg::prune()
{
    // Collect ressources required.
    SerializationRequirement req;
    if (convertionToSharedBuffer)
    {
        convertionToSharedBuffer->collectRequirements(req);
    }
    if (convertionToInline)
    {
        convertionToInline->collectRequirements(req);
    }
    // Free the resources.
    if (!req.xml)
    {
        releaseXmlContent();
    }

    releaseSharedBuffers(req.sharedBuffers);

    // Nobody cares anymore ?
    if (convertionToSharedBuffer == nullptr && convertionToInline == nullptr)
    {
        delete (this);
    }
}

bool parseBlobSize(XMLEle *blobWithAttachedBuffer, ssize_t &size)
{
    std::string sizeStr = findXMLAttValu(blobWithAttachedBuffer, "size");
    if (sizeStr == "")
    {
        return false;
    }
    std::size_t pos;
    size = std::stoll(sizeStr, &pos, 10);
    if (pos != sizeStr.size())
    {
        LOG_F(ERROR, "Invalid size attribute value %s", sizeStr.c_str());
        return false;
    }
    return true;
}

/** Init a message from xml content & additional incoming buffers */
bool Msg::fetchBlobs(std::list<int> &incomingSharedBuffers)
{
    /* Consume every buffers */
    for (auto blobContent : findBlobElements(xmlContent))
    {
        ssize_t blobSize;
        if (!parseBlobSize(blobContent, blobSize))
        {
            LOG_F(ERROR, "Attached blob misses the size attribute");
            return false;
        }

        std::string attached = findXMLAttValu(blobContent, "attached");
        if (attached == "true")
        {
            if (incomingSharedBuffers.empty())
            {
                LOG_F(ERROR, "Missing shared buffer...\n");
                return false;
            }

            queueSize += blobSize;
            // LOG_F(ERROR,"Found one fd !\n");
            int fd = *incomingSharedBuffers.begin();
            incomingSharedBuffers.pop_front();

            sharedBuffers.push_back(fd);
        }
        else
        {
            // Check cdata length vs blobSize ?
        }
    }
    return true;
}

void Msg::queuingDone()
{
    prune();
}

Msg *Msg::fromXml(MsgQueue *from, XMLEle *root, std::list<int> &incomingSharedBuffers)
{
    Msg *m = new Msg(from, root);
    if (!m->fetchBlobs(incomingSharedBuffers))
    {
        delete (m);
        return nullptr;
    }
    return m;
}

SerializedMsg *Msg::buildConvertionToSharedBuffer()
{
    if (convertionToSharedBuffer)
    {
        return convertionToSharedBuffer;
    }

    convertionToSharedBuffer = new SerializedMsgWithSharedBuffer(this);
    if (hasInlineBlobs && from)
    {
        convertionToSharedBuffer->blockReceiver(from);
    }
    return convertionToSharedBuffer;
}

SerializedMsg *Msg::buildConvertionToInline()
{
    if (convertionToInline)
    {
        return convertionToInline;
    }

    return convertionToInline = new SerializedMsgWithoutSharedBuffer(this);
}

SerializedMsg *Msg::serialize(MsgQueue *to)
{
    if (hasSharedBufferBlobs || hasInlineBlobs)
    {
        if (to->acceptSharedBuffers())
        {
            return buildConvertionToSharedBuffer();
        }
        else
        {
            return buildConvertionToInline();
        }
    }
    else
    {
        // Just serialize using copy
        return buildConvertionToInline();
    }
}