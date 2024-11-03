#ifndef ATOM_SYSTEM_NETWORK_MANAGER_HPP
#define ATOM_SYSTEM_NETWORK_MANAGER_HPP

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "atom/macro.hpp"

namespace atom::system {

/**
 * @struct NetworkConnection
 * @brief Represents a network connection.
 */
struct NetworkConnection {
    std::string protocol;       ///< Protocol (TCP or UDP).
    std::string localAddress;   ///< Local IP address.
    std::string remoteAddress;  ///< Remote IP address.
    int localPort;              ///< Local port number.
    int remotePort;             ///< Remote port number.
} ATOM_ALIGNAS(128);


class NetworkInterface {
public:
    NetworkInterface(std::string name, std::vector<std::string> addresses,
                     std::string mac, bool isUp);

    [[nodiscard]] auto getName() const -> const std::string&;
    [[nodiscard]] auto getAddresses() const -> const std::vector<std::string>&;
    auto getAddresses() -> std::vector<std::string>&;
    [[nodiscard]] auto getMac() const -> const std::string&;
    [[nodiscard]] auto isUp() const -> bool;

private:
    class NetworkInterfaceImpl;
    std::shared_ptr<NetworkInterfaceImpl> impl_;
};

class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();

    auto getNetworkInterfaces() -> std::vector<NetworkInterface>;
    static void enableInterface(const std::string& interfaceName);
    static void disableInterface(const std::string& interfaceName);
    static auto resolveDNS(const std::string& hostname) -> std::string;
    void monitorConnectionStatus();
    auto getInterfaceStatus(const std::string& interfaceName) -> std::string;
    static auto getDNSServers() -> std::vector<std::string>;
    static void setDNSServers(const std::vector<std::string>& dnsServers);
    static void addDNSServer(const std::string& dns);
    static void removeDNSServer(const std::string& dns);

private:
    class NetworkManagerImpl;
    std::unique_ptr<NetworkManagerImpl> impl_;
    static auto getMacAddress(const std::string& interfaceName)
        -> std::optional<std::string>;
    auto isInterfaceUp(const std::string& interfaceName) -> bool;
    void statusCheckLoop();
};


/**
 * @brief Gets the network connections of a process by its PID.
 * @param pid The process ID.
 * @return A vector of NetworkConnection structs representing the network
 * connections.
 */
auto getNetworkConnections(int pid) -> std::vector<NetworkConnection>;
}  // namespace atom::system

#endif