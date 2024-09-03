#ifndef ALPACADISCOVERY_H
#define ALPACADISCOVERY_H

#include <memory>
#include <string>
#include <vector>

/**
 * @class AlpacaDiscovery
 * @brief This class handles the discovery of Alpaca servers on the local
 * network via UDP broadcast.
 *
 * The AlpacaDiscovery class searches for Alpaca servers by broadcasting a
 * discovery message on the local network. It listens for responses that contain
 * the server's IP address and port.
 */
class AlpacaDiscovery {
public:
    /**
     * @brief Constructor
     */
    AlpacaDiscovery();

    /**
     * @brief Destructor
     */
    ~AlpacaDiscovery();

    /**
     * @brief Searches for Alpaca servers on the local network.
     *
     * @param numQuery The number of broadcast queries to send.
     * @param timeout The timeout in seconds for waiting on responses.
     * @return A vector of strings containing the IP addresses and ports of
     * discovered servers.
     * @throw std::runtime_error If socket operations fail.
     */
    std::vector<std::string> searchIPv4(int numQuery = 2, int timeout = 2);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl; /**< Pointer to the implementation class */
};

#endif  // ALPACADISCOVERY_H
