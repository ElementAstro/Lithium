/*
 * address.cpp
 *
 */

/*************************************************

Date: 2024-1-4

Description: Enhanced Address class for IPv4, IPv6, and Unix domain sockets.

**************************************************/

#include "address.hpp"

#include <arpa/inet.h>
#include <bitset>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>

#include "atom/log/loguru.hpp"

namespace atom::web {
constexpr int IPV4_BIT_LENGTH = 32;
constexpr int IPV6_SEGMENT_COUNT = 8;
constexpr int IPV6_SEGMENT_BIT_LENGTH = 16;
constexpr int UNIX_DOMAIN_PATH_MAX_LENGTH = 108;
constexpr uint32_t BYTE_MASK = 0xFF;

// IPv4 类的实现

IPv4::IPv4(const std::string& address) { parse(address); }

auto IPv4::parse(const std::string& address) -> bool {
    if (inet_pton(AF_INET, address.c_str(), &ipValue) == 1) {
        addressStr = address;
        return true;
    }
    LOG_F(ERROR, "Invalid IPv4 address: %s", address.c_str());
    return false;
}

auto IPv4::parseCIDR(const std::string& cidr) -> bool {
    size_t pos = cidr.find('/');
    if (pos == std::string::npos) {
        return parse(cidr);
    }
    std::string ipAddr = cidr.substr(0, pos);
    int prefixLength = std::stoi(cidr.substr(pos + 1));
    if (prefixLength < 0 || prefixLength > IPV4_BIT_LENGTH) {
        LOG_F(ERROR, "Invalid CIDR prefix length: %d", prefixLength);
        return false;
    }
    if (!parse(ipAddr)) {
        return false;
    }
    uint32_t mask =
        (prefixLength == 0) ? 0 : (~0U << (IPV4_BIT_LENGTH - prefixLength));
    ipValue &= htonl(mask);
    addressStr = integerToIp(ipValue) + "/" + std::to_string(prefixLength);
    return true;
}

void IPv4::printAddressType() const { LOG_F(INFO, "Address type: IPv4"); }

auto IPv4::isInRange(const std::string& start, const std::string& end) -> bool {
    uint32_t startIp = ipToInteger(start);
    uint32_t endIp = ipToInteger(end);
    uint32_t currentIp = ntohl(ipValue);
    return currentIp >= startIp && currentIp <= endIp;
}

auto IPv4::toBinary() const -> std::string {
    std::bitset<IPV4_BIT_LENGTH> bits(ntohl(ipValue));
    return bits.to_string();
}

auto IPv4::toHex() const -> std::string {
    std::stringstream stringStream;
    stringStream << std::hex << ntohl(ipValue);
    return stringStream.str();
}

auto IPv4::isEqual(const Address& other) const -> bool {
    if (other.getType() != "IPv4") {
        return false;
    }
    const IPv4* ipv4Other = dynamic_cast<const IPv4*>(&other);
    return ipValue == ipv4Other->ipValue;
}

auto IPv4::getType() const -> std::string { return "IPv4"; }

auto IPv4::getNetworkAddress(const std::string& mask) const -> std::string {
    uint32_t maskValue = ipToInteger(mask);
    uint32_t netAddr = ntohl(ipValue) & maskValue;
    return integerToIp(htonl(netAddr));
}

auto IPv4::getBroadcastAddress(const std::string& mask) const -> std::string {
    uint32_t maskValue = ipToInteger(mask);
    uint32_t broadcastAddr = (ntohl(ipValue) & maskValue) | (~maskValue);
    return integerToIp(htonl(broadcastAddr));
}

auto IPv4::isSameSubnet(const Address& other,
                        const std::string& mask) const -> bool {
    if (other.getType() != "IPv4") {
        return false;
    }
    uint32_t maskValue = ipToInteger(mask);
    uint32_t netAddr1 = ntohl(ipValue) & maskValue;
    uint32_t netAddr2 =
        ntohl(dynamic_cast<const IPv4*>(&other)->ipValue) & maskValue;
    return netAddr1 == netAddr2;
}

auto IPv4::ipToInteger(const std::string& ipAddr) const -> uint32_t {
    uint32_t result = 0;
    if (inet_pton(AF_INET, ipAddr.c_str(), &result) == 1) {
        return ntohl(result);
    }
    LOG_F(ERROR, "Invalid IPv4 address: %s", ipAddr.c_str());
    return 0;
}

auto IPv4::integerToIp(uint32_t ipAddr) const -> std::string {
    struct in_addr addr {};
    addr.s_addr = ipAddr;
    return inet_ntoa(addr);
}

// IPv6 类的实现

IPv6::IPv6(const std::string& address) { parse(address); }

auto IPv6::parse(const std::string& address) -> bool {
    std::array<uint8_t, 16> addrBuf{};
    if (inet_pton(AF_INET6, address.c_str(), addrBuf.data()) == 1) {
        addressStr = address;
        ipSegments.resize(IPV6_SEGMENT_COUNT);
        for (int i = 0; i < IPV6_SEGMENT_COUNT; ++i) {
            ipSegments[i] = (addrBuf[i * 2] << 8) | addrBuf[i * 2 + 1];
        }
        return true;
    }
    LOG_F(ERROR, "Invalid IPv6 address: %s", address.c_str());
    return false;
}

auto IPv6::parseCIDR(const std::string& cidr) -> bool {
    size_t pos = cidr.find('/');
    if (pos == std::string::npos) {
        return parse(cidr);
    }
    std::string ipAddr = cidr.substr(0, pos);
    int prefixLength = std::stoi(cidr.substr(pos + 1));
    if (prefixLength < 0 || prefixLength > 128) {
        LOG_F(ERROR, "Invalid CIDR prefix length: %d", prefixLength);
        return false;
    }
    if (!parse(ipAddr)) {
        return false;
    }
    // 应用掩码（需要详细实现）
    addressStr = ipAddr + "/" + std::to_string(prefixLength);
    return true;
}

void IPv6::printAddressType() const { LOG_F(INFO, "Address type: IPv6"); }

auto IPv6::isInRange(const std::string& start, const std::string& end) -> bool {
    auto startIp = ipToVector(start);
    auto endIp = ipToVector(end);
    for (size_t i = 0; i < ipSegments.size(); ++i) {
        if (ipSegments[i] < startIp[i] || ipSegments[i] > endIp[i]) {
            return false;
        }
    }
    return true;
}

auto IPv6::toBinary() const -> std::string {
    std::string binaryStr;
    for (uint16_t segment : ipSegments) {
        binaryStr += std::bitset<IPV6_SEGMENT_BIT_LENGTH>(segment).to_string();
    }
    return binaryStr;
}

auto IPv6::toHex() const -> std::string {
    std::stringstream stringStream;
    for (uint16_t segment : ipSegments) {
        stringStream << std::hex << segment;
    }
    return stringStream.str();
}

auto IPv6::isEqual(const Address& other) const -> bool {
    if (other.getType() != "IPv6") {
        return false;
    }
    const IPv6* ipv6Other = dynamic_cast<const IPv6*>(&other);
    return ipSegments == ipv6Other->ipSegments;
}

auto IPv6::getType() const -> std::string { return "IPv6"; }

auto IPv6::getNetworkAddress([[maybe_unused]] const std::string& mask) const
    -> std::string {
    // 需要实现
    return "";
}

auto IPv6::getBroadcastAddress([[maybe_unused]] const std::string& mask) const
    -> std::string {
    // 需要实现
    return "";
}

auto IPv6::isSameSubnet([[maybe_unused]] const Address& other,
                        [[maybe_unused]] const std::string& mask) const
    -> bool {
    // 需要实现
    return false;
}

auto IPv6::ipToVector(const std::string& ipAddr) const
    -> std::vector<uint16_t> {
    std::vector<uint16_t> segments(IPV6_SEGMENT_COUNT, 0);
    std::array<uint8_t, 16> addrBuf{};
    if (inet_pton(AF_INET6, ipAddr.c_str(), addrBuf.data()) == 1) {
        for (int i = 0; i < IPV6_SEGMENT_COUNT; ++i) {
            segments[i] = (addrBuf[i * 2] << 8) | addrBuf[i * 2 + 1];
        }
    }
    return segments;
}

auto IPv6::vectorToIp(const std::vector<uint16_t>& segments) const
    -> std::string {
    std::array<char, INET6_ADDRSTRLEN> str{};
    std::array<uint8_t, 16> addrBuf{};
    for (int i = 0; i < IPV6_SEGMENT_COUNT; ++i) {
        addrBuf[i * 2] = segments[i] >> 8;
        addrBuf[i * 2 + 1] = segments[i] & BYTE_MASK;
    }
    inet_ntop(AF_INET6, addrBuf.data(), str.data(), INET6_ADDRSTRLEN);
    return std::string(str.data());
}

// UnixDomain 类的实现

UnixDomain::UnixDomain(const std::string& path) { parse(path); }

auto UnixDomain::parse(const std::string& path) -> bool {
    if (path.empty() || path.length() >= UNIX_DOMAIN_PATH_MAX_LENGTH) {
        LOG_F(ERROR, "Invalid Unix domain socket path: %s", path.c_str());
        return false;
    }
    addressStr = path;
    return true;
}

void UnixDomain::printAddressType() const {
    LOG_F(INFO, "Address type: Unix Domain Socket");
}

auto UnixDomain::isInRange([[maybe_unused]] const std::string& start,
                           [[maybe_unused]] const std::string& end) -> bool {
    // 对于 Unix 域套接字，不适用
    return false;
}

auto UnixDomain::toBinary() const -> std::string {
    std::string binaryStr;
    for (char character : addressStr) {
        binaryStr += std::bitset<8>(character).to_string();
    }
    return binaryStr;
}

auto UnixDomain::toHex() const -> std::string {
    std::stringstream stringStream;
    for (char character : addressStr) {
        stringStream << std::hex << static_cast<int>(character);
    }
    return stringStream.str();
}

auto UnixDomain::isEqual(const Address& other) const -> bool {
    if (other.getType() != "UnixDomain") {
        return false;
    }
    return addressStr == other.getAddress();
}

auto UnixDomain::getType() const -> std::string { return "UnixDomain"; }

auto UnixDomain::getNetworkAddress(
    [[maybe_unused]] const std::string& mask) const -> std::string {
    // 不适用
    return "";
}

auto UnixDomain::getBroadcastAddress(
    [[maybe_unused]] const std::string& mask) const -> std::string {
    // 不适用
    return "";
}

auto UnixDomain::isSameSubnet([[maybe_unused]] const Address& other,
                              [[maybe_unused]] const std::string& mask) const
    -> bool {
    // 不适用
    return false;
}
}  // namespace atom::web
