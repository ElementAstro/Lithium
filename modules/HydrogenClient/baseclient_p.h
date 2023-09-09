#pragma once

#include "abstractbaseclient_p.h"
#include "hydrogenlilxml.hpp"

#include <tcpsocket.h>

namespace HYDROGEN
{

#ifdef ENABLE_HYDROGEN_SHARED_MEMORY
class ClientSharedBlobs
{
    public:
        class Blobs : public std::vector<std::string>
        {
            public:
                ~Blobs();
        };

    public:
        void enableDirectBlobAccess(const char * dev, const char * prop);
        void disableDirectBlobAccess();

        bool parseAttachedBlobs(const HYDROGEN::LilXmlElement &root, Blobs &blobs);
        bool isDirectBlobAccess(const std::string &dev, const std::string &prop) const;

        static bool hasDirectBlobAccessEntry(const std::map<std::string, std::set<std::string>> &directBlobAccess,
                                             const std::string &dev, const std::string &prop);

        void addIncomingSharedBuffer(int fd);

        void clear();

    private:
        std::list<int> incomingSharedBuffers;
        std::map<std::string, std::set<std::string>> directBlobAccess;
};

class TcpSocketSharedBlobs : public TcpSocket
{
    public:
        void readyRead() override;

        ClientSharedBlobs sharedBlobs;
};
#endif

class BaseDevice;


class BaseClientPrivate : public AbstractBaseClientPrivate
{
    public:
        BaseClientPrivate(BaseClient *parent);
        virtual ~BaseClientPrivate();

    public:
        bool connectToHostAndWait(std::string hostname, unsigned short port);

    public:
        ssize_t sendData(const void *data, size_t size) override;

#ifdef ENABLE_HYDROGEN_SHARED_MEMORY
        TcpSocketSharedBlobs clientSocket;
#else
        TcpSocket clientSocket;
#endif
        LilXmlParser xmlParser;
};

}
