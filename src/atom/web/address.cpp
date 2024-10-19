/*
 * address.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-1-4

Description: Address class for IPv4, IPv6, and Unix domain sockets.

**************************************************/

#include "address.hpp"

#include <bitset>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "atom/log/loguru.hpp"

namespace {
constexpr int K_I_PV4_SEGMENT_BITS = 8;
constexpr int K_I_PV4_SHIFT_START = 24;
constexpr int K_I_PV4_SHIFT_STEP = 8;
constexpr int K_I_PV6_SEGMENT_BITS = 16;
constexpr int K_I_PV6_SEGMENTS = 8;
}  // namespace

// IPv4 类的实现
auto IPv4::parse(const std::string& address) -> bool {
    LOG_F(INFO, "IPv4::parse called with address: {}", address);
    std::regex ipv4Pattern(
        R"(^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$)");
    if (std::regex_match(address, ipv4Pattern)) {
        addressStr = address;
        LOG_F(INFO, "Valid IPv4 address: {}", address);
        return true;
    }
    LOG_F(WARNING, "Invalid IPv4 address: {}", address);
    return false;
}

auto IPv4::printAddressType() const -> void {
    LOG_F(INFO, "IPv4::printAddressType called");
    std::cout << "Address type: IPv4" << std::endl;
}

auto IPv4::isInRange(const std::string& start, const std::string& end) -> bool {
    LOG_F(INFO, "IPv4::isInRange called with start: {}, end: {}", start, end);
    unsigned int ipAddress = ipToInteger(addressStr);
    unsigned int startIpAddress = ipToInteger(start);
    unsigned int endIpAddress = ipToInteger(end);
    bool result = (ipAddress >= startIpAddress && ipAddress <= endIpAddress);
    LOG_F(INFO, "IPv4::isInRange returning: {}", result);
    return result;
}

auto IPv4::toBinary() const -> std::string {
    LOG_F(INFO, "IPv4::toBinary called");
    std::stringstream binaryStream;
    std::istringstream iss(addressStr);
    std::string segment;
    while (std::getline(iss, segment, '.')) {
        int num = std::stoi(segment);
        binaryStream << std::bitset<K_I_PV4_SEGMENT_BITS>(
            num);  // 每个八位段转换为二进制
    }
    std::string result = binaryStream.str();
    LOG_F(INFO, "IPv4::toBinary returning: {}", result);
    return result;
}

auto IPv4::isEqual(const Address& other) const -> bool {
    LOG_F(INFO, "IPv4::isEqual called with other address type: {}",
          other.getType());
    if (other.getType() != "IPv4") {
        LOG_F(INFO, "IPv4::isEqual returning: false (different types)");
        return false;
    }
    bool result = addressStr == other.getAddress();
    LOG_F(INFO, "IPv4::isEqual returning: {}", result);
    return result;
}

auto IPv4::getType() const -> std::string {
    LOG_F(INFO, "IPv4::getType called");
    return "IPv4";
}

auto IPv4::ipToInteger(const std::string& ipAddress) const -> unsigned int {
    LOG_F(INFO, "IPv4::ipToInteger called with ipAddress: {}", ipAddress);
    std::istringstream iss(ipAddress);
    std::string segment;
    unsigned int result = 0;
    int shift = K_I_PV4_SHIFT_START;
    while (std::getline(iss, segment, '.')) {
        result |= (std::stoi(segment) << shift);
        shift -= K_I_PV4_SHIFT_STEP;
    }
    LOG_F(INFO, "IPv4::ipToInteger returning: {}", result);
    return result;
}

// IPv6 类的实现
auto IPv6::parse(const std::string& address) -> bool {
    LOG_F(INFO, "IPv6::parse called with address: {}", address);
    std::regex ipv6Pattern(R"(^([A-Fa-f0-9]{1,4}:){7}[A-Fa-f0-9]{1,4}$)");
    if (std::regex_match(address, ipv6Pattern)) {
        addressStr = address;
        LOG_F(INFO, "Valid IPv6 address: {}", address);
        return true;
    }
    LOG_F(WARNING, "Invalid IPv6 address: {}", address);
    return false;
}

auto IPv6::printAddressType() const -> void {
    LOG_F(INFO, "IPv6::printAddressType called");
    std::cout << "Address type: IPv6" << std::endl;
}

auto IPv6::isInRange(const std::string& start, const std::string& end) -> bool {
    LOG_F(INFO, "IPv6::isInRange called with start: {}, end: {}", start, end);
    std::vector<unsigned short> ipAddress = ipToVector(addressStr);
    std::vector<unsigned short> startIpAddress = ipToVector(start);
    std::vector<unsigned short> endIpAddress = ipToVector(end);

    for (int i = 0; i < K_I_PV6_SEGMENTS; ++i) {
        if (ipAddress[i] < startIpAddress[i] ||
            ipAddress[i] > endIpAddress[i]) {
            LOG_F(INFO, "IPv6::isInRange returning: false");
            return false;
        }
    }
    LOG_F(INFO, "IPv6::isInRange returning: true");
    return true;
}

auto IPv6::toBinary() const -> std::string {
    LOG_F(INFO, "IPv6::toBinary called");
    std::stringstream binaryStream;
    std::istringstream iss(addressStr);
    std::string segment;
    while (std::getline(iss, segment, ':')) {
        auto num = static_cast<unsigned short>(std::stoi(segment, nullptr, 16));
        binaryStream << std::bitset<K_I_PV6_SEGMENT_BITS>(
            num);  // 每个十六位段转换为二进制
    }
    std::string result = binaryStream.str();
    LOG_F(INFO, "IPv6::toBinary returning: {}", result);
    return result;
}

auto IPv6::isEqual(const Address& other) const -> bool {
    LOG_F(INFO, "IPv6::isEqual called with other address type: {}",
          other.getType());
    if (other.getType() != "IPv6") {
        LOG_F(INFO, "IPv6::isEqual returning: false (different types)");
        return false;
    }
    bool result = addressStr == other.getAddress();
    LOG_F(INFO, "IPv6::isEqual returning: {}", result);
    return result;
}

auto IPv6::getType() const -> std::string {
    LOG_F(INFO, "IPv6::getType called");
    return "IPv6";
}

auto IPv6::ipToVector(const std::string& ipAddress) const
    -> std::vector<unsigned short> {
    LOG_F(INFO, "IPv6::ipToVector called with ipAddress: {}", ipAddress);
    std::vector<unsigned short> result(K_I_PV6_SEGMENTS, 0);
    std::istringstream iss(ipAddress);
    std::string segment;
    int index = 0;
    while (std::getline(iss, segment, ':') && index < K_I_PV6_SEGMENTS) {
        result[index++] =
            static_cast<unsigned short>(std::stoi(segment, nullptr, 16));
    }
    LOG_F(INFO, "IPv6::ipToVector returning vector of size: {}", result.size());
    return result;
}