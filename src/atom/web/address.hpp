/*
 * address.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-1-4

Description: Address class for IPv4, IPv6, and Unix domain sockets.

**************************************************/

#ifndef ATOM_WEB_ADDRESS_HPP
#define ATOM_WEB_ADDRESS_HPP

#include <string>
#include <vector>

/**
 * @class Address
 * @brief Base class representing a generic network address.
 */
class Address {
protected:
    std::string addressStr;  ///< Stores the address as a string.

public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~Address() = default;

    /**
     * @brief Parses the address from a string.
     * @param address The address string to parse.
     * @return True if the address is successfully parsed, false otherwise.
     */
    virtual auto parse(const std::string& address) -> bool = 0;

    /**
     * @brief Prints the type of the address.
     */
    virtual void printAddressType() const = 0;

    /**
     * @brief Checks if the address is within a given range.
     * @param start The start of the range.
     * @param end The end of the range.
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
     * @brief Gets the address as a string.
     * @return The address as a string.
     */
    [[nodiscard]] auto getAddress() const -> std::string { return addressStr; }

    /**
     * @brief Compares the address with another address for equality.
     * @param other The other address to compare with.
     * @return True if the addresses are equal, false otherwise.
     */
    [[nodiscard]] virtual auto isEqual(const Address& other) const -> bool = 0;

    /**
     * @brief Gets the type of the address.
     * @return The type of the address as a string.
     */
    [[nodiscard]] virtual auto getType() const -> std::string = 0;
};

/**
 * @class IPv4
 * @brief Class representing an IPv4 address.
 */
class IPv4 : public Address {
public:
    /**
     * @brief Parses the IPv4 address from a string.
     * @param address The IPv4 address string to parse.
     * @return True if the address is successfully parsed, false otherwise.
     */
    auto parse(const std::string& address) -> bool override;

    /**
     * @brief Prints the type of the address (IPv4).
     */
    void printAddressType() const override;

    /**
     * @brief Checks if the IPv4 address is within a given range.
     * @param start The start of the range.
     * @param end The end of the range.
     * @return True if the address is within the range, false otherwise.
     */
    auto isInRange(const std::string& start,
                   const std::string& end) -> bool override;

    /**
     * @brief Converts the IPv4 address to its binary representation.
     * @return The binary representation of the IPv4 address as a string.
     */
    [[nodiscard]] auto toBinary() const -> std::string override;

    /**
     * @brief Compares the IPv4 address with another address for equality.
     * @param other The other address to compare with.
     * @return True if the addresses are equal, false otherwise.
     */
    [[nodiscard]] auto isEqual(const Address& other) const -> bool override;

    /**
     * @brief Gets the type of the address (IPv4).
     * @return The type of the address as a string.
     */
    [[nodiscard]] auto getType() const -> std::string override;

private:
    /**
     * @brief Converts an IPv4 address string to an integer.
     * @param ip The IPv4 address string.
     * @return The integer representation of the IPv4 address.
     */
    [[nodiscard]] auto ipToInteger(const std::string& ip) const -> unsigned int;
};

/**
 * @class IPv6
 * @brief Class representing an IPv6 address.
 */
class IPv6 : public Address {
public:
    /**
     * @brief Parses the IPv6 address from a string.
     * @param address The IPv6 address string to parse.
     * @return True if the address is successfully parsed, false otherwise.
     */
    auto parse(const std::string& address) -> bool override;

    /**
     * @brief Prints the type of the address (IPv6).
     */
    void printAddressType() const override;

    /**
     * @brief Checks if the IPv6 address is within a given range.
     * @param start The start of the range.
     * @param end The end of the range.
     * @return True if the address is within the range, false otherwise.
     */
    auto isInRange(const std::string& start,
                   const std::string& end) -> bool override;

    /**
     * @brief Converts the IPv6 address to its binary representation.
     * @return The binary representation of the IPv6 address as a string.
     */
    [[nodiscard]] auto toBinary() const -> std::string override;

    /**
     * @brief Compares the IPv6 address with another address for equality.
     * @param other The other address to compare with.
     * @return True if the addresses are equal, false otherwise.
     */
    [[nodiscard]] auto isEqual(const Address& other) const -> bool override;

    /**
     * @brief Gets the type of the address (IPv6).
     * @return The type of the address as a string.
     */
    [[nodiscard]] auto getType() const -> std::string override;

private:
    /**
     * @brief Converts an IPv6 address string to a vector of unsigned shorts.
     * @param ip The IPv6 address string.
     * @return The vector representation of the IPv6 address.
     */
    [[nodiscard]] auto ipToVector(const std::string& ip) const
        -> std::vector<unsigned short>;
};

#endif  // ATOM_WEB_ADDRESS_HPP
