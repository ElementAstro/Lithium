#include "remote_driver.hpp"

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <sys/socket.h>
#include <libgen.h>
#include <netdb.h>
#include <netinet/in.h>
#endif

#include "atom/log/loguru.hpp"

#include "hydrogen_server.hpp"

#include "atom/log/loguru.hpp"

RemoteDvrInfo::RemoteDvrInfo() : DvrInfo(false)
{
}

RemoteDvrInfo::RemoteDvrInfo(const RemoteDvrInfo &model) : DvrInfo(model),
                                                           host(model.host),
                                                           port(model.port)
{
}

RemoteDvrInfo::~RemoteDvrInfo()
{
}

RemoteDvrInfo *RemoteDvrInfo::clone() const
{
    return new RemoteDvrInfo(*this);
}

/*
    Old:
    void RemoteDvrInfo::extractRemoteId(const std::string &name, std::string &o_host, int &o_port, std::string &o_dev) const
    {
        char dev[MAXHYDROGENDEVICE] = {0};
        char host[MAXSBUF] = {0};

        int hydrogen_port = HYDROGENPORT;
        if (sscanf(name.c_str(), "%[^@]@%[^:]:%d", dev, host, &hydrogen_port) < 2)
        {
            // Device missing? Try a different syntax for all devices
            if (sscanf(name.c_str(), "@%[^:]:%d", host, &hydrogen_port) < 1)
            {
                log(fmt::format("Bad remote device syntax: %s\n", name.c_str()));
                // Bye();
            }
        }

        o_host = host;
        o_port = hydrogen_port;
        o_dev = dev;
    }
*/

void RemoteDvrInfo::extractRemoteId(const std::string &name, std::string &o_host, int &o_port, std::string &o_dev) const
{
    std::string dev;
    std::string host;

    // Extract host and port from name
    size_t atPos = name.find('@');
    size_t colonPos = name.find(':');
    size_t portEndPos = name.size();

    if (atPos != std::string::npos && colonPos != std::string::npos && colonPos > atPos + 1)
    {
        dev = name.substr(0, atPos);
        host = name.substr(atPos + 1, colonPos - atPos - 1);
        o_port = std::stoi(name.substr(colonPos + 1, portEndPos - colonPos - 1));
    }
    else if (atPos == std::string::npos && colonPos != std::string::npos && colonPos > 0)
    {
        host = name.substr(0, colonPos);
        o_port = std::stoi(name.substr(colonPos + 1, portEndPos - colonPos - 1));
    }

    o_host = host;
    o_dev = dev;
}

/* start the given remote HYDROGEN driver connection.
 * exit if trouble.
 */
void RemoteDvrInfo::start()
{
    int sockfd;
    std::string dev;
    extractRemoteId(name, host, port, dev);

    /* connect */
    sockfd = openHYDROGENServer();

    /* record flag pid, io channels, init lp and snoop list */

    this->setFds(sockfd, sockfd);

    if (verbose > 0)
        DLOG_F(INFO, "socket={}", sockfd);

    /* N.B. storing name now is key to limiting outbound traffic to this
     * dev.
     */
    if (!dev.empty())
        this->dev.insert(dev);

    /* Sending getProperties with device lets remote server limit its
     * outbound (and our inbound) traffic on this socket to this device.
     */
    XMLEle *root = addXMLEle(NULL, "getProperties");

    if (!dev.empty())
    {
        addXMLAtt(root, "device", dev.c_str());
        addXMLAtt(root, "version", TO_STRING(HYDROGENV));
    }
    else
    {
        // This informs downstream server that it is connecting to an upstream server
        // and not a regular client. The difference is in how it treats snooping properties
        // among properties.
        addXMLAtt(root, "device", "*");
        addXMLAtt(root, "version", TO_STRING(HYDROGENV));
    }

    Msg *mp = new Msg(nullptr, root);

    // pushmsg can kill this. do at end
    pushMsg(mp);
}

int RemoteDvrInfo::openHYDROGENServer()
{
    struct sockaddr_in serv_addr;
    struct hostent *hp;
    int sockfd;

    /* lookup host address */
    hp = gethostbyname(host.c_str());
    if (!hp)
    {
        LOG_F(ERROR, "gethostbyname({}): {}", host.c_str(), strerror(errno));
        // Bye();
    }

    /* create a socket to the HYDROGEN server */
    (void)memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr_list[0]))->s_addr;
    serv_addr.sin_port = htons(port);
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        LOG_F(ERROR, "socket({},{}): {}", host.c_str(), port, strerror(errno));
        // Bye();
    }

    /* connect */
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        LOG_F(ERROR, "connect({},{}): {}", host.c_str(), port, strerror(errno));
        // Bye();
    }

    /* ok */
    return (sockfd);
}