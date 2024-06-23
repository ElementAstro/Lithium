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

#include <array>
#include <cstdint>
#include <random>
#include <sstream>
#include <string>

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
    std::string to_string() const;

    /**
     * @brief Creates a UUID from a string representation.
     * @param str A string representation of a UUID.
     * @return A UUID object.
     */
    static UUID from_string(const std::string& str);

    /**
     * @brief Compares this UUID with another for equality.
     * @param other Another UUID to compare with.
     * @return True if both UUIDs are equal, otherwise false.
     */
    bool operator==(const UUID& other) const;

    /**
     * @brief Compares this UUID with another for inequality.
     * @param other Another UUID to compare with.
     * @return True if both UUIDs are not equal, otherwise false.
     */
    bool operator!=(const UUID& other) const;

    /**
     * @brief Defines a less-than comparison for UUIDs.
     * @param other Another UUID to compare with.
     * @return True if this UUID is less than the other, otherwise false.
     */
    bool operator<(const UUID& other) const;

    /**
     * @brief Writes the UUID to an output stream.
     * @param os The output stream to write to.
     * @param uuid The UUID to write.
     * @return The output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, const UUID& uuid);

    /**
     * @brief Reads a UUID from an input stream.
     * @param is The input stream to read from.
     * @param uuid The UUID to read into.
     * @return The input stream.
     */
    friend std::istream& operator>>(std::istream& is, UUID& uuid);

    /**
     * @brief Retrieves the underlying data of the UUID.
     * @return An array of 16 bytes representing the UUID.
     */
    std::array<uint8_t, 16> get_data() const;

    /**
     * @brief Gets the version of the UUID.
     * @return The version number of the UUID.
     */
    uint8_t version() const;

    /**
     * @brief Gets the variant of the UUID.
     * @return The variant number of the UUID.
     */
    uint8_t variant() const;

    /**
     * @brief Generates a version 3 UUID using the MD5 hashing algorithm.
     * @param namespace_uuid The namespace UUID.
     * @param name The name from which to generate the UUID.
     * @return A version 3 UUID.
     */
    static UUID generate_v3(const UUID& namespace_uuid,
                            const std::string& name);

    /**
     * @brief Generates a version 5 UUID using the SHA-1 hashing algorithm.
     * @param namespace_uuid The namespace UUID.
     * @param name The name from which to generate the UUID.
     * @return A version 5 UUID.
     */
    static UUID generate_v5(const UUID& namespace_uuid,
                            const std::string& name);

    /**
     * @brief Generates a version 1, time-based UUID.
     * @return A version 1 UUID.
     */
    static UUID generate_v1();

private:
    /**
     * @brief Generates a random UUID.
     */
    void generate_random();

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
    template <typename CTX, int (*INIT)(CTX*),
              int (*UPDATE)(CTX*, const void*, size_t),
              int (*FINAL)(unsigned char*, CTX*)>
    static UUID generate_name_based(const UUID& namespace_uuid,
                                    const std::string& name, int version);

    /**
     * @brief Generates a unique node identifier for version 1 UUIDs.
     * @return A 64-bit node identifier.
     */
    static uint64_t generate_node();

    std::array<uint8_t, 16> data;  ///< The internal storage of the UUID.
};

/**
 * @brief Generates a unique UUID and returns it as a string.
 * @return A unique UUID as a string.
 */
[[nodiscard]] std::string generateUniqueUUID();
}  // namespace atom::utils

#endif
