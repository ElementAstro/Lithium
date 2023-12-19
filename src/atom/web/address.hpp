#pragma once

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <iptypes.h>
#else
#include <netinet/in.h>
#endif

#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include <string>

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

class Address
{
public:
    typedef std::shared_ptr<Address> ptr;

    virtual ~Address() {}

    static Address::ptr Create(const sockaddr *addr, socklen_t addrlen);

    static Address::ptr Create(const std::string &address, uint16_t port);

    static bool Lookup(std::vector<Address::ptr> &result, const std::string &host,
                       int family = AF_INET, int type = 0, int protocol = 0);

    static Address::ptr LookupAny(const std::string &host,
                                  int family = AF_INET, int type = SOCK_STREAM, int protocol = 0);

    static std::shared_ptr<IPAddress> LookupAnyIPAddress(const std::string &host,
                                                         int family = AF_INET, int type = SOCK_STREAM, int protocol = 0);

    static bool GetInterfaceAddresses(std::multimap<std::string,
                                                    std::pair<Address::ptr, uint32_t>> &result,
                                      int family = AF_INET);

    static bool GetInterfaceAddresses(std::vector<std::pair<Address::ptr, uint32_t>> &result,
                                      const std::string &iface, int family = AF_INET);

    virtual const sockaddr *getAddr() const = 0;

    virtual sockaddr *getAddr() = 0;

    virtual socklen_t getAddrLen() const = 0;

    virtual std::ostream &insert(std::ostream &os) const = 0;

    int getFamily() const;

    std::string toString() const;

    bool operator<(const Address &rhs) const;

    bool operator==(const Address &rhs) const;

    bool operator!=(const Address &rhs) const;
};

class IPAddress : public Address
{
public:
    typedef std::shared_ptr<IPAddress> ptr;

    virtual ~IPAddress() {}

    static IPAddress::ptr Create(const char *address, uint16_t port = 0);

    virtual IPAddress::ptr broadcastAddress(uint32_t prefix_len) = 0;

    virtual IPAddress::ptr networdAddress(uint32_t prefix_len) = 0;

    virtual IPAddress::ptr subnetMask(uint32_t prefix_len) = 0;

    virtual uint16_t getPort() const = 0;

    virtual void setPort(uint16_t v) = 0;
};

#ifdef _WIN32
#pragma comment(lib, "ws2_32.lib")
#endif

class IPv4Address : public IPAddress
{
public:
    typedef std::shared_ptr<IPv4Address> ptr;

    IPv4Address(const sockaddr_in &address);

    IPv4Address(uint32_t address = INADDR_ANY, uint16_t port = 0);

    static IPv4Address::ptr Create(const char *ip, uint16_t port);

    const sockaddr *getAddr() const override;

    sockaddr *getAddr() override;

    socklen_t getAddrLen() const override;

    std::ostream &insert(std::ostream &os) const override;

    IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;

    IPAddress::ptr networdAddress(uint32_t prefix_len) override;

    IPAddress::ptr subnetMask(uint32_t prefix_len) override;

    uint16_t getPort() const override;

    void setPort(uint16_t v) override;

private:
    sockaddr_in m_addr;
};

class IPv6Address : public IPAddress
{
public:
    typedef std::shared_ptr<IPv6Address> ptr;

    static IPv6Address::ptr Create(const char *address, uint16_t port = 0);

    IPv6Address();

    IPv6Address(const sockaddr_in6 &address);

    IPv6Address(const uint8_t address[16], uint16_t port = 0);

    const sockaddr *getAddr() const override;

    sockaddr *getAddr() override;

    socklen_t getAddrLen() const override;

    std::ostream &insert(std::ostream &os) const override;

    IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;

    IPAddress::ptr networdAddress(uint32_t prefix_len) override;

    IPAddress::ptr subnetMask(uint32_t prefix_len) override;

    uint16_t getPort() const override;

    void setPort(uint16_t v) override;

private:
    sockaddr_in6 m_addr;
};