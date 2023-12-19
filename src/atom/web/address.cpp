#include "address.hpp"

Address::ptr Address::Create(const std::string &address, uint16_t port)
{
    addrinfo hints, *result = nullptr;
    memset(&hints, 0, sizeof(addrinfo));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICHOST | AI_NUMERICSERV;

    char service[32];
    snprintf(service, sizeof(service), "%u", port);

    int ret = getaddrinfo(address.c_str(), service, &hints, &result);
    if (ret != 0)
    {
        return nullptr;
    }

    Address::ptr addr;
    for (addrinfo *rp = result; rp != nullptr; rp = rp->ai_next)
    {
        addr = Create(rp->ai_addr, rp->ai_addrlen);
        if (addr != nullptr)
        {
            break;
        }
    }

    freeaddrinfo(result);
    return addr;
}

Address::ptr Address::Create(const sockaddr *addr, socklen_t addrlen)
{
    if (addr == nullptr)
    {
        return nullptr;
    }
    if (addr->sa_family == AF_INET)
    {
        return std::make_shared<IPv4Address>(*(const sockaddr_in *)addr);
    }
    else if (addr->sa_family == AF_INET6)
    {
        return std::make_shared<IPv6Address>(*(const sockaddr_in6 *)addr);
    }
    else
    {
        return nullptr;
    }
}

bool Address::Lookup(std::vector<Address::ptr> &result, const std::string &host,
                     int family, int type, int protocol)
{
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = family;
    hints.ai_socktype = type;
    hints.ai_protocol = protocol;
    if (getaddrinfo(host.c_str(), nullptr, &hints, &res) != 0)
    {
        return false;
    }

    char addrbuf[INET6_ADDRSTRLEN] = {0};
    for (struct addrinfo *p = res; p != nullptr; p = p->ai_next)
    {
        result.push_back(Create(p->ai_addr, p->ai_addrlen));
    }
    freeaddrinfo(res);
    return true;
}

Address::ptr Address::LookupAny(const std::string &host,
                                int family, int type, int protocol)
{
    std::vector<Address::ptr> result;
    if (Lookup(result, host, family, type, protocol))
    {
        if (!result.empty())
        {
            return result.front();
        }
    }
    return nullptr;
}

std::shared_ptr<IPAddress> Address::LookupAnyIPAddress(const std::string &host,
                                                       int family, int type, int protocol)
{
    auto addr = LookupAny(host, family, type, protocol);
    return std::dynamic_pointer_cast<IPAddress>(addr);
}

#ifdef _WIN32

bool Address::GetInterfaceAddresses(std::multimap<std::string,
                                                  std::pair<Address::ptr, uint32_t>> &result,
                                    int family)
{
    PIP_ADAPTER_ADDRESSES pAddresses = nullptr;
    ULONG outBufLen = 0;
    ULONG flags = GAA_FLAG_INCLUDE_PREFIX | GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST;

    DWORD ret = GetAdaptersAddresses(family, flags, nullptr, nullptr, &outBufLen);
    if (ret != ERROR_BUFFER_OVERFLOW)
    {
        return false;
    }
    pAddresses = (PIP_ADAPTER_ADDRESSES)malloc(outBufLen);
    ret = GetAdaptersAddresses(family, flags, nullptr, pAddresses, &outBufLen);
    if (ret != ERROR_SUCCESS)
    {
        free(pAddresses);
        return false;
    }

    for (PIP_ADAPTER_ADDRESSES adapter = pAddresses; adapter != nullptr; adapter = adapter->Next)
    {
        for (IP_ADAPTER_UNICAST_ADDRESS *unicastAddr = adapter->FirstUnicastAddress;
             unicastAddr != nullptr;
             unicastAddr = unicastAddr->Next)
        {
            Address::ptr addr;
            uint32_t prefix_len = 0;
            if (family == AF_INET && unicastAddr->Address.lpSockaddr->sa_family == AF_INET)
            {
                addr = Create(unicastAddr->Address.lpSockaddr, sizeof(sockaddr_in));
                sockaddr_in *sai = (sockaddr_in *)unicastAddr->Address.lpSockaddr;
                prefix_len = CountBytes(sai->sin_addr.s_addr);
            }
            else if (family == AF_INET6 && unicastAddr->Address.lpSockaddr->sa_family == AF_INET6)
            {
                addr = Create(unicastAddr->Address.lpSockaddr, sizeof(sockaddr_in6));
                sockaddr_in6 *sai6 = (sockaddr_in6 *)unicastAddr->Address.lpSockaddr;
                prefix_len = CountBytes(sai6->sin6_addr.u.Byte);
            }
            if (addr)
            {
                result.insert(std::make_pair(adapter->AdapterName,
                                             std::make_pair(addr, prefix_len)));
            }
        }
    }

    free(pAddresses);
    return !result.empty();
}

#else

bool Address::GetInterfaceAddresses(std::multimap<std::string,
                                                  std::pair<Address::ptr, uint32_t>> &result,
                                    int family)
{
    struct ifaddrs *res = nullptr, *ifa = nullptr;
    if (getifaddrs(&res) == -1)
    {
        return false;
    }

    for (ifa = res; ifa != nullptr; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == nullptr)
        {
            continue;
        }
        Address::ptr addr;
        uint32_t prefix_len = 0;
        if (family == AF_INET && ifa->ifa_addr->sa_family == AF_INET)
        {
            addr = Create(ifa->ifa_addr, sizeof(sockaddr_in));
            sockaddr_in *sai = (sockaddr_in *)ifa->ifa_addr;
            prefix_len = CountBytes(sai->sin_addr.s_addr);
        }
        else if (family == AF_INET6 && ifa->ifa_addr->sa_family == AF_INET6)
        {
            addr = Create(ifa->ifa_addr, sizeof(sockaddr_in6), false);
            sockaddr_in6 *sai6 = (sockaddr_in6 *)ifa->ifa_addr;
            prefix_len = CountBytes(sai6->sin6_addr.__in6_u.__u6_addr8);
        }
        else
        {
            continue;
        }
        if (addr)
        {
            result.insert(std::make_pair(ifa->ifa_name,
                                         std::make_pair(addr, prefix_len)));
        }
    }

    freeifaddrs(res);
    return !result.empty();
}

#endif

bool Address::GetInterfaceAddresses(std::vector<std::pair<Address::ptr, uint32_t>> &result,
                                    const std::string &iface, int family)
{
    if (iface.empty() || iface == "*")
    {
        if (family == AF_INET || family == AF_UNSPEC)
        {
            result.push_back(std::make_pair(Create("0.0.0.0", sizeof(sockaddr_in)), 0));
        }
        if (family == AF_INET6 || family == AF_UNSPEC)
        {
            result.push_back(std::make_pair(Create("::0", sizeof(sockaddr_in)), 0));
        }
        return true;
    }

    std::multimap<std::string, std::pair<Address::ptr, uint32_t>> addrs;
    if (!GetInterfaceAddresses(addrs, family))
    {
        return false;
    }

    auto its = addrs.equal_range(iface);
    for (; its.first != its.second; ++its.first)
    {
        result.push_back(its.first->second);
    }
    return !result.empty();
}

IPv4Address::IPv4Address(const sockaddr_in &address)
{
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = address.sin_port;
    m_addr.sin_addr.s_addr = address.sin_addr.s_addr;
}

IPv4Address::IPv4Address(uint32_t address, uint16_t port)
{
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons(port);
    m_addr.sin_addr.s_addr = htonl(address);
}

IPv4Address::ptr IPv4Address::Create(const char *ip, uint16_t port)
{
    IPv4Address::ptr ret(new IPv4Address);
    ret->m_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &ret->m_addr.sin_addr) <= 0)
    {
        return nullptr;
    }
    return ret;
}

const sockaddr *IPv4Address::getAddr() const
{
    return (sockaddr *)&m_addr;
}

sockaddr *IPv4Address::getAddr()
{
    return (sockaddr *)&m_addr;
}

socklen_t IPv4Address::getAddrLen() const
{
    return sizeof(m_addr);
}

std::ostream &IPv4Address::insert(std::ostream &os) const
{
    uint32_t addr = ntohl(m_addr.sin_addr.s_addr);
    os << ((addr >> 24) & 0xff) << "."
       << ((addr >> 16) & 0xff) << "."
       << ((addr >> 8) & 0xff) << "."
       << (addr & 0xff);
    return os;
}

IPAddress::ptr IPv4Address::broadcastAddress(uint32_t prefix_len)
{
    if (prefix_len > 32)
    {
        return nullptr;
    }
    sockaddr_in baddr(m_addr);
    baddr.sin_addr.s_addr |= htonl((1 << prefix_len) - 1);
    return std::make_shared<IPv4Address>(baddr);
}

IPAddress::ptr IPv4Address::networdAddress(uint32_t prefix_len)
{
    if (prefix_len > 32)
    {
        return nullptr;
    }
    sockaddr_in naddr(m_addr);
    naddr.sin_addr.s_addr &= htonl(((1 << prefix_len) - 1) << (32 - prefix_len));
    return std::make_shared<IPv4Address>(naddr);
}

IPAddress::ptr IPv4Address::subnetMask(uint32_t prefix_len)
{
    sockaddr_in subnet;
    memset(&subnet, 0, sizeof(subnet));
    subnet.sin_family = AF_INET;
    subnet.sin_addr.s_addr = htonl(((1 << prefix_len) - 1) << (32 - prefix_len));
    return std::make_shared<IPv4Address>(subnet);
}

uint16_t IPv4Address::getPort() const
{
    return ntohs(m_addr.sin_port);
}

void IPv4Address::setPort(uint16_t v)
{
    m_addr.sin_port = htons(v);
}

IPv6Address::IPv6Address()
{
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin6_family = AF_INET6;
}

IPv6Address::IPv6Address(const sockaddr_in6 &address)
{
    memcpy(&m_addr, &address, sizeof(m_addr));
}

IPv6Address::IPv6Address(const uint8_t address[16], uint16_t port)
{
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin6_family = AF_INET6;
    memcpy(&m_addr.sin6_addr.s6_addr, address, 16);
    m_addr.sin6_port = htons(port);
}

IPv6Address::ptr IPv6Address::Create(const char *address, uint16_t port)
{
    struct in6_addr addr;
    if (inet_pton(AF_INET6, address, &addr) <= 0)
    {
        return nullptr;
    }
    return std::make_shared<IPv6Address>(addr.s6_addr, port);
}

const sockaddr *IPv6Address::getAddr() const
{
    return reinterpret_cast<const sockaddr *>(&m_addr);
}

sockaddr *IPv6Address::getAddr()
{
    return reinterpret_cast<sockaddr *>(&m_addr);
}

socklen_t IPv6Address::getAddrLen() const
{
    return static_cast<socklen_t>(sizeof(m_addr));
}

std::ostream &IPv6Address::insert(std::ostream &os) const
{
    char buf[INET6_ADDRSTRLEN] = {0};
    inet_ntop(AF_INET6, &(m_addr.sin6_addr), buf, sizeof(buf));
    os << "[" << buf << "]:" << ntohs(m_addr.sin6_port);
    return os;
}

IPAddress::ptr IPv6Address::broadcastAddress(uint32_t prefix_len)
{
    if (prefix_len > 128)
    {
        return nullptr;
    }
    sockaddr_in6 baddr(m_addr);
    uint32_t mask = prefix_len == 0 ? 0 : (~0ull << (128 - prefix_len));
    for (int i = 0; i < 16; ++i)
    {
        baddr.sin6_addr.s6_addr[i] |= ((mask >> (120 - i * 8)) & 0xff);
    }
    return std::make_shared<IPv6Address>(baddr);
}

IPAddress::ptr IPv6Address::networdAddress(uint32_t prefix_len)
{
    if (prefix_len > 128)
    {
        return nullptr;
    }
    sockaddr_in6 addr(m_addr);
    uint32_t mask = prefix_len == 0 ? 0 : (~0ull << (128 - prefix_len));
    for (int i = 0; i < 16; ++i)
    {
        addr.sin6_addr.s6_addr[i] &= ((mask >> (120 - i * 8)) & 0xff);
    }
    return std::make_shared<IPv6Address>(addr);
}

IPAddress::ptr IPv6Address::subnetMask(uint32_t prefix_len)
{
    sockaddr_in6 subnet;
    memset(&subnet, 0, sizeof(subnet));
    subnet.sin6_family = AF_INET6;
    uint32_t mask = prefix_len == 0 ? 0 : (~0ull << (128 - prefix_len));
    for (int i = 0; i < 16; ++i)
    {
        subnet.sin6_addr.s6_addr[i] = (mask >> (120 - i * 8)) & 0xff;
    }
    return std::make_shared<IPv6Address>(subnet);
}

uint16_t IPv6Address::getPort() const
{
    return ntohs(m_addr.sin6_port);
}

void IPv6Address::setPort(uint16_t v)
{
    m_addr.sin6_port = htons(v);
}

/*
int main()
{
    // 示例代码
    std::vector<Address::ptr> addrs;
    if (Address::Lookup(addrs, "www.baidu.com"))
    {
        for (auto &addr : addrs)
        {
            std::cout << addr->toString() << std::endl;
        }
    }

    return 0;
}
*/
