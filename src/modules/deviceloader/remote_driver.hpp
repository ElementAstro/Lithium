#include "driver_info.hpp"

class RemoteDvrInfo : public DvrInfo
{
    /* open a connection to the given host and port or die.
     * return socket fd.
     */
    int openHYDROGENServer();

    void extractRemoteId(const std::string &name, std::string &o_host, int &o_port, std::string &o_dev) const;

protected:
    RemoteDvrInfo(const RemoteDvrInfo &model);

public:
    std::string host;
    int port;

    RemoteDvrInfo();
    virtual ~RemoteDvrInfo();

    virtual void start();

    virtual RemoteDvrInfo *clone() const;

    virtual const std::string remoteServerUid() const
    {
        return std::string(host) + ":" + std::to_string(port);
    }
};