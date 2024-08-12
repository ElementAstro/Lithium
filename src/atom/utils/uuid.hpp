/*
 * uuid.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-5

Description: UUID Generator

**************************************************/

#ifndef ATOM_UTILS_UUID_HPP
#define ATOM_UTILS_UUID_HPP

#include <algorithm>
#include <array>
#include <cstdint>
#include <string>
#include "macro.hpp"

#include <openssl/evp.h>

namespace atom::utils {
/**
 * @class UUID
 * @brief Represents a Universally Unique Identifier (UUID).
 *
 * This class provides methods for generating, comparing, and manipulating
 * UUIDs.
 */
class UUID {
public:
    /**
     * @brief Constructs a new UUID with a random value.
     */
    UUID();

    /**
     * @brief Constructs a UUID from a given 16-byte array.
     * @param data An array of 16 bytes representing the UUID.
     */
    explicit UUID(const std::array<uint8_t, 16>& data);

    /**
     * @brief Converts the UUID to a string representation.
     * @return A string representation of the UUID.
     */
    ATOM_NODISCARD auto toString() const -> std::string;

    /**
     * @brief Creates a UUID from a string representation.
     * @param str A string representation of a UUID.
     * @return A UUID object.
     */
    static auto fromString(const std::string& str) -> UUID;

    /**
     * @brief Compares this UUID with another for equality.
     * @param other Another UUID to compare with.
     * @return True if both UUIDs are equal, otherwise false.
     */
    auto operator==(const UUID& other) const -> bool;

    /**
     * @brief Compares this UUID with another for inequality.
     * @param other Another UUID to compare with.
     * @return True if both UUIDs are not equal, otherwise false.
     */
    auto operator!=(const UUID& other) const -> bool;

    /**
     * @brief Defines a less-than comparison for UUIDs.
     * @param other Another UUID to compare with.
     * @return True if this UUID is less than the other, otherwise false.
     */
    auto operator<(const UUID& other) const -> bool;

    /**
     * @brief Writes the UUID to an output stream.
     * @param os The output stream to write to.
     * @param uuid The UUID to write.
     * @return The output stream.
     */
    friend auto operator<<(std::ostream& os, const UUID& uuid) -> std::ostream&;

    /**
     * @brief Reads a UUID from an input stream.
     * @param is The input stream to read from.
     * @param uuid The UUID to read into.
     * @return The input stream.
     */
    friend auto operator>>(std::istream& is, UUID& uuid) -> std::istream&;

    /**
     * @brief Retrieves the underlying data of the UUID.
     * @return An array of 16 bytes representing the UUID.
     */
    ATOM_NODISCARD auto getData() const -> std::array<uint8_t, 16>;

    /**
     * @brief Gets the version of the UUID.
     * @return The version number of the UUID.
     */
    ATOM_NODISCARD auto version() const -> uint8_t;

    /**
     * @brief Gets the variant of the UUID.
     * @return The variant number of the UUID.
     */
    ATOM_NODISCARD auto variant() const -> uint8_t;

    /**
     * @brief Generates a version 3 UUID using the MD5 hashing algorithm.
     * @param namespace_uuid The namespace UUID.
     * @param name The name from which to generate the UUID.
     * @return A version 3 UUID.
     */
    static auto generateV3(const UUID& namespace_uuid,
                           const std::string& name) -> UUID;

    /**
     * @brief Generates a version 5 UUID using the SHA-1 hashing algorithm.
     * @param namespace_uuid The namespace UUID.
     * @param name The name from which to generate the UUID.
     * @return A version 5 UUID.
     */
    static auto generateV5(const UUID& namespace_uuid,
                           const std::string& name) -> UUID;

    /**
     * @brief Generates a version 1, time-based UUID.
     * @return A version 1 UUID.
     */
    static auto generateV1() -> UUID;

private:
    /**
     * @brief Generates a random UUID.
     */
    void generateRandom();

    /**
     * @brief Template method that generates a name-based UUID using a hashing
     * algorithm.
     * @tparam CTX The context type for the hashing function.
     * @tparam INIT Function to initialize the hash context.
     * @tparam UPDATE Function to update the hash context with data.
     * @tparam FINAL Function to finalize the hashing process.
     * @param namespace_uuid The namespace UUID.
     * @param name The name from which to generate the UUID.
     * @param version The version of the UUID to be generated.
     * @return A UUID generated from the name.
     */
    template <const EVP_MD* (*DIGEST)()>
    static auto generateNameBased(const UUID& namespace_uuid,
                                  const std::string& name, int version) -> UUID;

    /**
     * @brief Generates a unique node identifier for version 1 UUIDs.
     * @return A 64-bit node identifier.
     */
    static auto generateNode() -> uint64_t;

    std::array<uint8_t, 16> data_;  ///< The internal storage of the UUID.
};

template <const EVP_MD* (*DIGEST)()>
auto UUID::generateNameBased(const UUID& namespace_uuid,
                             const std::string& name, int version) -> UUID {
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, DIGEST(), nullptr);
    EVP_DigestUpdate(ctx, namespace_uuid.data_.data(),
                     namespace_uuid.data_.size());
    EVP_DigestUpdate(ctx, name.data(), name.size());
    std::array<uint8_t, EVP_MAX_MD_SIZE> hash;
    unsigned int hash_len;
    EVP_DigestFinal_ex(ctx, hash.data(), &hash_len);
    EVP_MD_CTX_free(ctx);

    std::array<uint8_t, 16> uuid_data;
    std::copy_n(hash.begin(), 16, uuid_data.begin());

    uuid_data[6] = (uuid_data[6] & 0x0F) | (version << 4);  // Set version
    uuid_data[8] = (uuid_data[8] & 0x3F) | 0x80;            // Set variant

    return UUID(uuid_data);
}

/**
 * @brief Generates a unique UUID and returns it as a string.
 * @return A unique UUID as a string.
 */
ATOM_NODISCARD auto generateUniqueUUID() -> std::string;
}  // namespace atom::utils

#endif
