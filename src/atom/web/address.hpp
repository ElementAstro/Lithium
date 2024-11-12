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
 * @brief A base class representing a generic network address.
 */
class Address {
protected:
    std::string addressStr;  ///< Stores the address as a string.

public:
    Address() = default;

    /**
     * @brief Virtual destructor.
     */
    virtual ~Address() = default;

    /**
     * @brief Copy constructor.
     */
    Address(const Address& other) = default;

    /**
     * @brief Copy assignment operator.
     */
    Address& operator=(const Address& other) = default;

    /**
     * @brief Move constructor.
     */
    Address(Address&& other) noexcept = default;

    /**
     * @brief Move assignment operator.
     */
    Address& operator=(Address&& other) noexcept = default;

    /**
     * @brief Parses the address string.
     * @param address The address string to parse.
     * @return True if the address is successfully parsed, false otherwise.
     */
    virtual auto parse(const std::string& address) -> bool = 0;

    /**
     * @brief Prints the address type.
     */
    virtual void printAddressType() const = 0;

    /**
     * @brief Checks if the address is within the specified range.
     * @param start The start address of the range.
     * @param end The end address of the range.
     * @return True if the address is within the range, false otherwise.
     */
    virtual auto isInRange(const std::string& start,
                           const std::string& end) -> bool = 0;

    /**
     * @brief Converts the address to its binary representation.
     * @return The binary representation of the address as a string.
     */
    [[nodiscard]] virtual auto toBinary() const -> std::string = 0;

    /**
     * @brief Gets the address string.
     * @return The address as a string.
     */
    [[nodiscard]] auto getAddress() const -> std::string { return addressStr; }

    /**
     * @brief Checks if two addresses are equal.
     * @param other The other address to compare with.
     * @return True if the addresses are equal, false otherwise.
     */
    [[nodiscard]] virtual auto isEqual(const Address& other) const -> bool = 0;

    /**
     * @brief Gets the address type.
     * @return The address type as a string.
     */
    [[nodiscard]] virtual auto getType() const -> std::string = 0;

    /**
     * @brief Gets the network address given a subnet mask.
     * @param mask The subnet mask.
     * @return The network address as a string.
     */
    [[nodiscard]] virtual auto getNetworkAddress(const std::string& mask) const
        -> std::string = 0;

    /**
     * @brief Gets the broadcast address given a subnet mask.
     * @param mask The subnet mask.
     * @return The broadcast address as a string.
     */
    [[nodiscard]] virtual auto getBroadcastAddress(
        const std::string& mask) const -> std::string = 0;

    /**
     * @brief Checks if two addresses are in the same subnet.
     * @param other The other address to compare with.
     * @param mask The subnet mask.
     * @return True if the addresses are in the same subnet, false otherwise.
     */
    [[nodiscard]] virtual auto isSameSubnet(
        const Address& other, const std::string& mask) const -> bool = 0;

    /**
     * @brief Converts the address to its hexadecimal representation.
     * @return The hexadecimal representation of the address as a string.
     */
    [[nodiscard]] virtual auto toHex() const -> std::string = 0;
};

/**
 * @class IPv4
 * @brief A class representing an IPv4 address.
 */
class IPv4 : public Address {
public:
    IPv4() = default;

    /**
     * @brief Constructs an IPv4 address from a string.
     * @param address The IPv4 address as a string.
     */
    explicit IPv4(const std::string& address);

    /**
     * @brief Parses the IPv4 address string.
     * @param address The IPv4 address string to parse.
     * @return True if the address is successfully parsed, false otherwise.
     */
    auto parse(const std::string& address) -> bool override;

    /**
     * @brief Prints the address type.
     */
    void printAddressType() const override;

    /**
     * @brief Checks if the address is within the specified range.
     * @param start The start address of the range.
     * @param end The end address of the range.
     * @return True if the address is within the range, false otherwise.
     */
    auto isInRange(const std::string& start,
                   const std::string& end) -> bool override;

    /**
     * @brief Converts the address to its binary representation.
     * @return The binary representation of the address as a string.
     */
    [[nodiscard]] auto toBinary() const -> std::string override;

    /**
     * @brief Checks if two addresses are equal.
     * @param other The other address to compare with.
     * @return True if the addresses are equal, false otherwise.
     */
    [[nodiscard]] auto isEqual(const Address& other) const -> bool override;

    /**
     * @brief Gets the address type.
     * @return The address type as a string.
     */
    [[nodiscard]] auto getType() const -> std::string override;

    /**
     * @brief Gets the network address given a subnet mask.
     * @param mask The subnet mask.
     * @return The network address as a string.
     */
    [[nodiscard]] auto getNetworkAddress(const std::string& mask) const
        -> std::string override;

    /**
     * @brief Gets the broadcast address given a subnet mask.
     * @param mask The subnet mask.
     * @return The broadcast address as a string.
     */
    [[nodiscard]] auto getBroadcastAddress(const std::string& mask) const
        -> std::string override;

    /**
     * @brief Checks if two addresses are in the same subnet.
     * @param other The other address to compare with.
     * @param mask The subnet mask.
     * @return True if the addresses are in the same subnet, false otherwise.
     */
    [[nodiscard]] auto isSameSubnet(
        const Address& other, const std::string& mask) const -> bool override;

    /**
     * @brief Converts the address to its hexadecimal representation.
     * @return The hexadecimal representation of the address as a string.
     */
    [[nodiscard]] auto toHex() const -> std::string override;

    /**
     * @brief Parses an IPv4 address in CIDR notation.
     * @param cidr The CIDR notation string.
     * @return True if the CIDR notation is successfully parsed, false
     * otherwise.
     */
    auto parseCIDR(const std::string& cidr) -> bool;

private:
    uint32_t ipValue{0};  ///< Stores the IP address as an integer.

    /**
     * @brief Converts an IP address string to an integer.
     * @param ipAddr The IP address string.
     * @return The IP address as an integer.
     */
    [[nodiscard]] auto ipToInteger(const std::string& ipAddr) const -> uint32_t;

    /**
     * @brief Converts an integer to an IP address string.
     * @param ipAddr The IP address as an integer.
     * @return The IP address string.
     */
    [[nodiscard]] auto integerToIp(uint32_t ipAddr) const -> std::string;
};

/**
 * @class IPv6
 * @brief A class representing an IPv6 address.
 */
class IPv6 : public Address {
public:
    IPv6() = default;

    /**
     * @brief Constructs an IPv6 address from a string.
     * @param address The IPv6 address as a string.
     */
    explicit IPv6(const std::string& address);

    /**
     * @brief Parses the IPv6 address string.
     * @param address The IPv6 address string to parse.
     * @return True if the address is successfully parsed, false otherwise.
     */
    auto parse(const std::string& address) -> bool override;

    /**
     * @brief Prints the address type.
     */
    void printAddressType() const override;

    /**
     * @brief Checks if the address is within the specified range.
     * @param start The start address of the range.
     * @param end The end address of the range.
     * @return True if the address is within the range, false otherwise.
     */
    auto isInRange(const std::string& start,
                   const std::string& end) -> bool override;

    /**
     * @brief Converts the address to its binary representation.
     * @return The binary representation of the address as a string.
     */
    [[nodiscard]] auto toBinary() const -> std::string override;

    /**
     * @brief Checks if two addresses are equal.
     * @param other The other address to compare with.
     * @return True if the addresses are equal, false otherwise.
     */
    [[nodiscard]] auto isEqual(const Address& other) const -> bool override;

    /**
     * @brief Gets the address type.
     * @return The address type as a string.
     */
    [[nodiscard]] auto getType() const -> std::string override;

    /**
     * @brief Gets the network address given a subnet mask.
     * @param mask The subnet mask.
     * @return The network address as a string.
     */
    [[nodiscard]] auto getNetworkAddress(const std::string& mask) const
        -> std::string override;

    /**
     * @brief Gets the broadcast address given a subnet mask.
     * @param mask The subnet mask.
     * @return The broadcast address as a string.
     */
    [[nodiscard]] auto getBroadcastAddress(const std::string& mask) const
        -> std::string override;

    /**
     * @brief Checks if two addresses are in the same subnet.
     * @param other The other address to compare with.
     * @param mask The subnet mask.
     * @return True if the addresses are in the same subnet, false otherwise.
     */
    [[nodiscard]] auto isSameSubnet(
        const Address& other, const std::string& mask) const -> bool override;

    /**
     * @brief Converts the address to its hexadecimal representation.
     * @return The hexadecimal representation of the address as a string.
     */
    [[nodiscard]] auto toHex() const -> std::string override;

    /**
     * @brief Parses an IPv6 address in CIDR notation.
     * @param cidr The CIDR notation string.
     * @return True if the CIDR notation is successfully parsed, false
     * otherwise.
     */
    auto parseCIDR(const std::string& cidr) -> bool;

private:
    std::vector<uint16_t> ipSegments;  ///< Stores the IP address segments.

    /**
     * @brief Converts an IP address string to a vector of segments.
     * @param ipAddr The IP address string.
     * @return The IP address as a vector of segments.
     */
    [[nodiscard]] auto ipToVector(const std::string& ipAddr) const
        -> std::vector<uint16_t>;

    /**
     * @brief Converts a vector of segments to an IP address string.
     * @param segments The IP address segments.
     * @return The IP address string.
     */
    [[nodiscard]] auto vectorToIp(const std::vector<uint16_t>& segments) const
        -> std::string;
};

/**
 * @class UnixDomain
 * @brief A class representing a Unix domain socket address.
 */
class UnixDomain : public Address {
public:
    UnixDomain() = default;

    /**
     * @brief Constructs a Unix domain socket address from a path.
     * @param path The Unix domain socket path.
     */
    explicit UnixDomain(const std::string& path);

    /**
     * @brief Parses the Unix domain socket path.
     * @param path The Unix domain socket path to parse.
     * @return True if the path is successfully parsed, false otherwise.
     */
    auto parse(const std::string& path) -> bool override;

    /**
     * @brief Prints the address type.
     */
    void printAddressType() const override;

    /**
     * @brief Checks if the address is within the specified range.
     * @param start The start address of the range.
     * @param end The end address of the range.
     * @return True if the address is within the range, false otherwise.
     */
    auto isInRange(const std::string& start,
                   const std::string& end) -> bool override;

    /**
     * @brief Converts the address to its binary representation.
     * @return The binary representation of the address as a string.
     */
    [[nodiscard]] auto toBinary() const -> std::string override;

    /**
     * @brief Checks if two addresses are equal.
     * @param other The other address to compare with.
     * @return True if the addresses are equal, false otherwise.
     */
    [[nodiscard]] auto isEqual(const Address& other) const -> bool override;

    /**
     * @brief Gets the address type.
     * @return The address type as a string.
     */
    [[nodiscard]] auto getType() const -> std::string override;

    /**
     * @brief Gets the network address given a subnet mask.
     * @param mask The subnet mask.
     * @return The network address as a string.
     */
    [[nodiscard]] auto getNetworkAddress(const std::string& mask) const
        -> std::string override;

    /**
     * @brief Gets the broadcast address given a subnet mask.
     * @param mask The subnet mask.
     * @return The broadcast address as a string.
     */
    [[nodiscard]] auto getBroadcastAddress(const std::string& mask) const
        -> std::string override;

    /**
     * @brief Checks if two addresses are in the same subnet.
     * @param other The other address to compare with.
     * @param mask The subnet mask.
     * @return True if the addresses are in the same subnet, false otherwise.
     */
    [[nodiscard]] auto isSameSubnet(
        const Address& other, const std::string& mask) const -> bool override;

    /**
     * @brief Converts the address to its hexadecimal representation.
     * @return The hexadecimal representation of the address as a string.
     */
    [[nodiscard]] auto toHex() const -> std::string override;
};
}  // namespace atom::web

#endif  // ATOM_WEB_ADDRESS_HPP