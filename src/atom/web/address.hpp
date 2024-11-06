/*
 * address.hpp
 *
 * Copyright (C)
 */

/*************************************************

Date: 2024-1-4

Description: Enhanced Address class for IPv4, IPv6, and Unix domain sockets.

**************************************************/

#ifndef ATOM_WEB_ADDRESS_HPP
#define ATOM_WEB_ADDRESS_HPP

#include <cstdint>
#include <string>
#include <vector>

namespace atom::web {
/**
 * @class Address
 * @brief 基础类，表示通用的网络地址。
 */
class Address {
protected:
    std::string addressStr;  ///< 存储地址的字符串形式。

public:
    Address() = default;
    /**
     * @brief 虚析构函数。
     */
    virtual ~Address() = default;

    /**
     * @brief 拷贝构造函数。
     */
    Address(const Address& other) = default;

    /**
     * @brief 拷贝赋值运算符。
     */
    Address& operator=(const Address& other) = default;

    /**
     * @brief 移动构造函数。
     */
    Address(Address&& other) noexcept = default;

    /**
     * @brief 移动赋值运算符。
     */
    Address& operator=(Address&& other) noexcept = default;

    /**
     * @brief 解析地址字符串。
     * @param address 要解析的地址字符串。
     * @return 如果成功解析返回 true，否则返回 false。
     */
    virtual auto parse(const std::string& address) -> bool = 0;

    /**
     * @brief 打印地址类型。
     */
    virtual void printAddressType() const = 0;

    /**
     * @brief 判断地址是否在指定范围内。
     * @param start 范围的起始地址。
     * @param end 范围的结束地址。
     * @return 如果在范围内返回 true，否则返回 false。
     */
    virtual auto isInRange(const std::string& start,
                           const std::string& end) -> bool = 0;

    /**
     * @brief 将地址转换为二进制表示形式。
     * @return 地址的二进制字符串。
     */
    [[nodiscard]] virtual auto toBinary() const -> std::string = 0;

    /**
     * @brief 获取地址字符串。
     * @return 地址的字符串形式。
     */
    [[nodiscard]] auto getAddress() const -> std::string { return addressStr; }

    /**
     * @brief 判断两个地址是否相等。
     * @param other 要比较的另一个地址。
     * @return 如果相等返回 true，否则返回 false。
     */
    [[nodiscard]] virtual auto isEqual(const Address& other) const -> bool = 0;

    /**
     * @brief 获取地址类型。
     * @return 地址类型的字符串。
     */
    [[nodiscard]] virtual auto getType() const -> std::string = 0;

    /**
     * @brief 获取网络地址。
     * @param mask 子网掩码。
     * @return 网络地址的字符串。
     */
    [[nodiscard]] virtual auto getNetworkAddress(const std::string& mask) const
        -> std::string = 0;

    /**
     * @brief 获取广播地址。
     * @param mask 子网掩码。
     * @return 广播地址的字符串。
     */
    [[nodiscard]] virtual auto getBroadcastAddress(
        const std::string& mask) const -> std::string = 0;

    /**
     * @brief 判断两个地址是否在同一子网内。
     * @param other 要比较的另一个地址。
     * @param mask 子网掩码。
     * @return 如果在同一子网内返回 true，否则返回 false。
     */
    [[nodiscard]] virtual auto isSameSubnet(
        const Address& other, const std::string& mask) const -> bool = 0;

    /**
     * @brief 将地址转换为十六进制字符串。
     * @return 地址的十六进制字符串。
     */
    [[nodiscard]] virtual auto toHex() const -> std::string = 0;
};

/**
 * @class IPv4
 * @brief 表示 IPv4 地址的类。
 */
class IPv4 : public Address {
public:
    IPv4() = default;
    explicit IPv4(const std::string& address);
    auto parse(const std::string& address) -> bool override;
    void printAddressType() const override;
    auto isInRange(const std::string& start,
                   const std::string& end) -> bool override;
    [[nodiscard]] auto toBinary() const -> std::string override;
    [[nodiscard]] auto isEqual(const Address& other) const -> bool override;
    [[nodiscard]] auto getType() const -> std::string override;
    [[nodiscard]] auto getNetworkAddress(const std::string& mask) const
        -> std::string override;
    [[nodiscard]] auto getBroadcastAddress(const std::string& mask) const
        -> std::string override;
    [[nodiscard]] auto isSameSubnet(
        const Address& other, const std::string& mask) const -> bool override;
    [[nodiscard]] auto toHex() const -> std::string override;

    /**
     * @brief 解析 CIDR 格式的 IP 地址。
     * @param cidr CIDR 格式的字符串。
     * @return 如果成功解析返回 true，否则返回 false。
     */
    auto parseCIDR(const std::string& cidr) -> bool;

private:
    uint32_t ipValue{0};  ///< 以整数形式存储 IP 地址。

    [[nodiscard]] auto ipToInteger(const std::string& ipAddr) const -> uint32_t;
    [[nodiscard]] auto integerToIp(uint32_t ipAddr) const -> std::string;
};

/**
 * @class IPv6
 * @brief 表示 IPv6 地址的类。
 */
class IPv6 : public Address {
public:
    IPv6() = default;
    explicit IPv6(const std::string& address);
    auto parse(const std::string& address) -> bool override;
    void printAddressType() const override;
    auto isInRange(const std::string& start,
                   const std::string& end) -> bool override;
    [[nodiscard]] auto toBinary() const -> std::string override;
    [[nodiscard]] auto isEqual(const Address& other) const -> bool override;
    [[nodiscard]] auto getType() const -> std::string override;
    [[nodiscard]] auto getNetworkAddress(const std::string& mask) const
        -> std::string override;
    [[nodiscard]] auto getBroadcastAddress(const std::string& mask) const
        -> std::string override;
    [[nodiscard]] auto isSameSubnet(
        const Address& other, const std::string& mask) const -> bool override;
    [[nodiscard]] auto toHex() const -> std::string override;

    /**
     * @brief 解析 CIDR 格式的 IPv6 地址。
     * @param cidr CIDR 格式的字符串。
     * @return 如果成功解析返回 true，否则返回 false。
     */
    auto parseCIDR(const std::string& cidr) -> bool;

private:
    std::vector<uint16_t> ipSegments;  ///< 存储 IP 地址的段。

    [[nodiscard]] auto ipToVector(const std::string& ipAddr) const
        -> std::vector<uint16_t>;
    [[nodiscard]] auto vectorToIp(const std::vector<uint16_t>& segments) const
        -> std::string;
};

/**
 * @class UnixDomain
 * @brief 表示 Unix 域套接字地址的类。
 */
class UnixDomain : public Address {
public:
    UnixDomain() = default;
    explicit UnixDomain(const std::string& path);
    auto parse(const std::string& path) -> bool override;
    void printAddressType() const override;
    auto isInRange(const std::string& start,
                   const std::string& end) -> bool override;
    [[nodiscard]] auto toBinary() const -> std::string override;
    [[nodiscard]] auto isEqual(const Address& other) const -> bool override;
    [[nodiscard]] auto getType() const -> std::string override;
    [[nodiscard]] auto getNetworkAddress(const std::string& mask) const
        -> std::string override;
    [[nodiscard]] auto getBroadcastAddress(const std::string& mask) const
        -> std::string override;
    [[nodiscard]] auto isSameSubnet(
        const Address& other, const std::string& mask) const -> bool override;
    [[nodiscard]] auto toHex() const -> std::string override;
};
}  // namespace atom::web

#endif  // ATOM_WEB_ADDRESS_HPP
