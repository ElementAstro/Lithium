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
#include <format>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

// IPv4 类的实现
auto IPv4::parse(const std::string& address) -> bool {
    std::regex ipv4Pattern(
        R"(^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$)");
    if (std::regex_match(address, ipv4Pattern)) {
        addressStr = address;
        std::cout << std::format("Valid IPv4 address: {}", address)
                  << std::endl;
        return true;
    }          std::cerr << std::format("Invalid IPv4 address: {}", address)
                  << std::endl;
        return false;

}

void IPv4::printAddressType() const {
    std::cout << "Address type: IPv4" << std::endl;
}

bool IPv4::isInRange(const std::string& start, const std::string& end) {
    unsigned int ip = ipToInteger(addressStr);
    unsigned int startIp = ipToInteger(start);
    unsigned int endIp = ipToInteger(end);
    return (ip >= startIp && ip <= endIp);
}

std::string IPv4::toBinary() const {
    std::stringstream binaryStream;
    std::istringstream iss(addressStr);
    std::string segment;
    while (std::getline(iss, segment, '.')) {
        int num = std::stoi(segment);
        binaryStream << std::bitset<8>(num);  // 每个八位段转换为二进制
    }
    return binaryStream.str();
}

bool IPv4::isEqual(const Address& other) const {
    if (other.getType() != "IPv4") {
        return false;
    }
    return addressStr == other.getAddress();
}

std::string IPv4::getType() const { return "IPv4"; }

unsigned int IPv4::ipToInteger(const std::string& ip) const {
    std::istringstream iss(ip);
    std::string segment;
    unsigned int result = 0;
    int shift = 24;
    while (std::getline(iss, segment, '.')) {
        result |= (std::stoi(segment) << shift);
        shift -= 8;
    }
    return result;
}

// IPv6 类的实现
bool IPv6::parse(const std::string& address) {
    std::regex ipv6_pattern(R"(^([A-Fa-f0-9]{1,4}:){7}[A-Fa-f0-9]{1,4}$)");
    if (std::regex_match(address, ipv6_pattern)) {
        addressStr = address;
        std::cout << std::format("Valid IPv6 address: {}", address)
                  << std::endl;
        return true;
    } else {
        std::cerr << std::format("Invalid IPv6 address: {}", address)
                  << std::endl;
        return false;
    }
}

void IPv6::printAddressType() const {
    std::cout << "Address type: IPv6" << std::endl;
}

bool IPv6::isInRange(const std::string& start, const std::string& end) {
    std::vector<unsigned short> ip = ipToVector(addressStr);
    std::vector<unsigned short> startIp = ipToVector(start);
    std::vector<unsigned short> endIp = ipToVector(end);

    for (int i = 0; i < 8; ++i) {
        if (ip[i] < startIp[i] || ip[i] > endIp[i]) {
            return false;
        }
    }
    return true;
}

std::string IPv6::toBinary() const {
    std::stringstream binaryStream;
    std::istringstream iss(addressStr);
    std::string segment;
    while (std::getline(iss, segment, ':')) {
        unsigned short num =
            static_cast<unsigned short>(std::stoi(segment, nullptr, 16));
        binaryStream << std::bitset<16>(num);  // 每个十六位段转换为二进制
    }
    return binaryStream.str();
}

bool IPv6::isEqual(const Address& other) const {
    if (other.getType() != "IPv6") {
        return false;
    }
    return addressStr == other.getAddress();
}

std::string IPv6::getType() const { return "IPv6"; }

std::vector<unsigned short> IPv6::ipToVector(const std::string& ip) const {
    std::vector<unsigned short> result(8, 0);
    std::istringstream iss(ip);
    std::string segment;
    int index = 0;
    while (std::getline(iss, segment, ':') && index < 8) {
        result[index++] =
            static_cast<unsigned short>(std::stoi(segment, nullptr, 16));
    }
    return result;
}

// 使用示例
int main() {
    // IPv4测试
    Address* ipv4 = new IPv4();
    ipv4->printAddressType();
    ipv4->parse("192.168.1.1");
    std::cout << "Binary format: " << ipv4->toBinary() << std::endl;
    std::cout << "Is in range 192.168.0.0 - 192.168.255.255: " << std::boolalpha
              << ipv4->isInRange("192.168.0.0", "192.168.255.255") << std::endl;

    // IPv6测试
    Address* ipv6 = new IPv6();
    ipv6->printAddressType();
    ipv6->parse("2001:0db8:85a3:0000:0000:8a2e:0370:7334");
    std::cout << "Binary format: " << ipv6->toBinary() << std::endl;
    std::cout << "Is in range 2001:0db8:85a3:0000:0000:8a2e:0370:7330 - "
                 "2001:0db8:85a3:0000:0000:8a2e:0370:733f: "
              << ipv6->isInRange("2001:0db8:85a3:0000:0000:8a2e:0370:7330",
                                 "2001:0db8:85a3:0000:0000:8a2e:0370:733f")
              << std::endl;

    // 比较两个地址是否相等
    Address* ipv4_2 = new IPv4();
    ipv4_2->parse("192.168.1.1");
    std::cout << "IPv4 addresses are equal: " << std::boolalpha
              << ipv4->isEqual(*ipv4_2) << std::endl;

    // 释放内存
    delete ipv4;
    delete ipv4_2;
    delete ipv6;

    return 0;
}
