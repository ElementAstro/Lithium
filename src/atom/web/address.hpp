/*
 * address.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-1-4

Description: Address class for IPv4, IPv6, and Unix domain sockets.

**************************************************/

#pragma once

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <iptypes.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <netinet/in.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#endif

#include <vector>
#include <map>
#include <memory>
#include <string>

namespace Atom::Web
{

    template <class T>
    static T CreateMask(uint32_t bits)
    {
        return (1 << (sizeof(T) * 8 - bits)) - 1;
    }

    template <class T>
    static uint32_t CountBytes(T value)
    {
        uint32_t result = 0;
        uint8_t *ptr = reinterpret_cast<uint8_t *>(&value);
        for (size_t i = 0; i < sizeof(T); ++i)
        {
            uint8_t byte = ptr[i];
            for (; byte; ++result)
            {
                byte &= byte - 1;
            }
        }
        return result;
    }

    class IPAddress;

    /**
     * @brief The base class for all socket addresses.
     *
     * This class provides a common interface for all socket address
     * types, including IPv4, IPv6, and Unix domain sockets. It provides
     * functions for creating, looking up, and manipulating socket addresses.
     *
     * @note This class is an abstract base class and cannot be instantiated
     * directly. Instead, use one of the derived classes to create or manipulate
     * socket addresses.
     */
    class Address
    {
    public:
        /**
         * Shared pointer to an Address.
         */
        typedef std::shared_ptr<Address> ptr;

        /**
         * Destructor.
         */
        virtual ~Address() {}

        /**
         * Creates an Address from a socket address.
         *
         * @param addr The socket address.
         * @param addrlen The length of the socket address.
         * @return The Address.
         */
        static Address::ptr Create(const sockaddr *addr, socklen_t addrlen);

        /**
         * Creates an Address from a string representation.
         *
         * @param address The string representation.
         * @param port The port.
         * @return The Address.
         */
        static Address::ptr Create(const std::string &address, uint16_t port);

        /**
         * Looks up the addresses of a host.
         *
         * @param result The vector to store the addresses.
         * @param host The host name.
         * @param family The address family.
         * @param type The socket type.
         * @param protocol The protocol.
         * @return True if the lookup succeeded, false otherwise.
         */
        static bool Lookup(std::vector<Address::ptr> &result, const std::string &host,
                           int family = AF_INET, int type = 0, int protocol = 0);

        /**
         * Looks up the address of a host.
         *
         * @param host The host name.
         * @param family The address family.
         * @param type The socket type.
         * @param protocol The protocol.
         * @return The address.
         */
        static Address::ptr LookupAny(const std::string &host,
                                      int family = AF_INET, int type = SOCK_STREAM, int protocol = 0);

        /**
         * Looks up the address of a host.
         *
         * @param host The host name.
         * @param family The address family.
         * @param type The socket type.
         * @param protocol The protocol.
         * @return The address.
         */
        static std::shared_ptr<IPAddress> LookupAnyIPAddress(const std::string &host,
                                                             int family = AF_INET, int type = SOCK_STREAM, int protocol = 0);

        /**
         * Gets the addresses of all network interfaces.
         *
         * @param result The map to store the addresses.
         * @param family The address family.
         * @return True if the lookup succeeded, false otherwise.
         */
        static bool GetInterfaceAddresses(std::multimap<std::string,
                                                        std::pair<Address::ptr, uint32_t>> &result,
                                          int family = AF_INET);

        /**
         * Gets the addresses of all network interfaces.
         *
         * @param result The vector to store the addresses.
         * @param iface The interface name.
         * @param family The address family.
         * @return True if the lookup succeeded, false otherwise.
         */
        static bool GetInterfaceAddresses(std::vector<std::pair<Address::ptr, uint32_t>> &result,
                                          const std::string &iface, int family = AF_INET);

        /**
         * Gets the address of the local host.
         *
         * @param family The address family.
         * @return The address.
         */
        virtual const sockaddr *getAddr() const = 0;

        /**
         * Gets the address of the local host.
         *
         * @param family The address family.
         * @return The address.
         */
        virtual sockaddr *getAddr() = 0;

        /**
         * Gets the length of the address.
         *
         * @return The length of the address.
         */
        virtual socklen_t getAddrLen() const = 0;

        /**
         * Inserts the address into a stream.
         *
         * @param os The stream.
         * @return The stream.
         */
        virtual std::ostream &insert(std::ostream &os) const = 0;

        /**
         * Gets the address family.
         *
         * @return The address family.
         */
        int getFamily() const;

        /**
         * Gets the string representation of the address.
         *
         * @return The string representation.
         */
        std::string toString() const;

        /**
         * Compares two addresses.
         *
         * @param rhs The address to compare to.
         * @return True if the addresses are equal, false otherwise.
         */
        bool operator<(const Address &rhs) const;

        /**
         * Compares two addresses.
         *
         * @param rhs The address to compare to.
         * @return True if the addresses are equal, false otherwise.
         */
        bool operator==(const Address &rhs) const;

        /**
         * Compares two addresses.
         *
         * @param rhs The address to compare to.
         * @return True if the addresses are not equal, false otherwise.
         */
        bool operator!=(const Address &rhs) const;
    };

    /**
     * @brief The IPv4 address.
     *
     * This class represents an IPv4 address.
     */
    class IPAddress : public Address
    {
    public:
        /**
         * Shared pointer to an IPv4Address.
         */
        typedef std::shared_ptr<IPAddress> ptr;

        /**
         * Destructor.
         */
        virtual ~IPAddress() {}

        /**
         * Creates an IPv4Address from a string representation.
         *
         * @param address The string representation.
         * @param port The port.
         * @return The IPv4Address.
         */
        static IPAddress::ptr Create(const char *address, uint16_t port = 0);

        /**
         * Creates an IPv4Address from a string representation.
         *
         * @param address The string representation.
         * @param port The port.
         * @return The IPv4Address.
         */
        virtual IPAddress::ptr broadcastAddress(uint32_t prefix_len) = 0;

        /**
         * Creates an IPv4Address from a string representation.
         *
         * @param address The string representation.
         * @param port The port.
         * @return The IPv4Address.
         */
        virtual IPAddress::ptr networdAddress(uint32_t prefix_len) = 0;

        /**
         * Creates an IPv4Address from a string representation.
         *
         * @param address The string representation.
         * @param port The port.
         * @return The IPv4Address.
         */
        virtual IPAddress::ptr subnetMask(uint32_t prefix_len) = 0;

        /**
         * Gets the port.
         *
         * @return The port.
         */
        virtual uint16_t getPort() const = 0;

        /**
         * Sets the port.
         *
         * @param v The port.
         */
        virtual void setPort(uint16_t v) = 0;
    };

    /**
     * @brief The IPv4 address.
     *
     * This class represents an IPv4 address.
     */
    class IPv4Address : public IPAddress
    {
    public:
        /**
         * Shared pointer to an IPv4Address.
         */
        typedef std::shared_ptr<IPv4Address> ptr;

        /**
         * Constructor.
         * @param address
         */
        IPv4Address(const sockaddr_in &address);

        /**
         * Constructor.
         * @param address
         */
        IPv4Address(uint32_t address = INADDR_ANY, uint16_t port = 0);

        /**
         * Creates an IPv4Address from a string representation.
         *
         * @param ip The string representation.
         * @param port The port.
         * @return The IPv4Address.
         */
        static IPv4Address::ptr Create(const char *ip, uint16_t port);

        /**
         * Gets the address.
         *
         * @return The address.
         */
        const sockaddr *getAddr() const override;

        /**
         * Gets the address.
         *
         * @return The address.
         */
        sockaddr *getAddr() override;

        /**
         * Gets the length of the address.
         *
         * @return The length of the address.
         */
        socklen_t getAddrLen() const override;

        /**
         * Inserts the address into a stream.
         *
         * @param os The stream.
         * @return The stream.
         */
        std::ostream &insert(std::ostream &os) const override;

        /**
         * Gets the broadcast address.
         *
         * @param prefix_len The prefix length.
         * @return The broadcast address.
         */
        IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;

        /**
         * Gets the network address.
         *
         * @param prefix_len The prefix length.
         * @return The network address.
         */
        IPAddress::ptr networdAddress(uint32_t prefix_len) override;

        /**
         * Gets the subnet mask.
         *
         * @param prefix_len The prefix length.
         * @return The subnet mask.
         */
        IPAddress::ptr subnetMask(uint32_t prefix_len) override;

        /**
         * Gets the port.
         *
         * @return The port.
         */
        uint16_t getPort() const override;

        /**
         * Sets the port.
         *
         * @param v The port.
         */
        void setPort(uint16_t v) override;

    private:
        sockaddr_in m_addr;
    };

    /**
     * @brief The IPv6 address.
     *
     * This class represents an IPv6 address.
     */
    class IPv6Address : public IPAddress
    {
    public:
        /**
         * Shared pointer to an IPv6Address.
         */
        typedef std::shared_ptr<IPv6Address> ptr;

        /**
         * Creates an IPv6Address from a string representation.
         *
         * @param address The string representation.
         * @param port The port.
         * @return The IPv6Address.
         */
        static IPv6Address::ptr Create(const char *address, uint16_t port = 0);

        /**
         * Constructor.
         */
        IPv6Address();

        /**
         * Constructor.
         * @param address
         */
        IPv6Address(const sockaddr_in6 &address);

        /**
         * Constructor.
         * @param address
         */
        IPv6Address(const uint8_t address[16], uint16_t port = 0);

        /**
         * Gets the address.
         *
         * @return The address.
         */
        const sockaddr *getAddr() const override;

        /**
         * Gets the address.
         *
         * @return The address.
         */
        sockaddr *getAddr() override;

        /**
         * Gets the length of the address.
         *
         * @return The length of the address.
         */
        socklen_t getAddrLen() const override;

        /**
         * Inserts the address into a stream.
         *
         * @param os The stream.
         * @return The stream.
         */
        std::ostream &insert(std::ostream &os) const override;

        /**
         * Gets the broadcast address.
         *
         * @param prefix_len The prefix length.
         * @return The broadcast address.
         */
        IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;

        /**
         * Gets the network address.
         *
         * @param prefix_len The prefix length.
         * @return The network address.
         */
        IPAddress::ptr networdAddress(uint32_t prefix_len) override;

        /**
         * Gets the subnet mask.
         *
         * @param prefix_len The prefix length.
         * @return The subnet mask.
         */
        IPAddress::ptr subnetMask(uint32_t prefix_len) override;

        /**
         * Gets the port.
         *
         * @return The port.
         */
        uint16_t getPort() const override;

        /**
         * Sets the port.
         *
         * @param v The port.
         */
        void setPort(uint16_t v) override;

    private:
        sockaddr_in6 m_addr;
    };

    /**
     * @brief 获取IPv4地址
     * @return std::vector<std::string>
     */
    [[nodiscard]] std::vector<std::string> getIPv4Addresses();

    /**
     * @brief 获取IPv6地址
     * @return std::vector<std::string>
     */
    [[nodiscard]] std::vector<std::string> getIPv6Addresses();
}
