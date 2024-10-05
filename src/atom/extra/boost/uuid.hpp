#ifndef ATOM_EXTRA_BOOST_UUID_HPP
#define ATOM_EXTRA_BOOST_UUID_HPP

#include <boost/functional/hash.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <chrono>
#include <compare>
#include <format>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

namespace atom::extra::boost {
constexpr size_t UUID_SIZE = 16;
constexpr int BASE64_RESERVE_SIZE = 22;
constexpr int SHIFT_40 = 40;
constexpr int SHIFT_32 = 32;
constexpr int SHIFT_24 = 24;
constexpr int SHIFT_16 = 16;
constexpr int SHIFT_8 = 8;
constexpr int BASE64_MASK = 63;
constexpr int BASE64_SHIFT_18 = 18;
constexpr int BASE64_SHIFT_12 = 12;
constexpr int BASE64_SHIFT_6 = 6;
constexpr uint64_t TIMESTAMP_DIVISOR = 10000000;
constexpr uint64_t UUID_EPOCH = 0x01B21DD213814000L;

/**
 * @class UUID
 * @brief A wrapper class for Boost.UUID providing various UUID operations.
 */
class UUID {
private:
    ::boost::uuids::uuid uuid_;  ///< The Boost.UUID object.

public:
    /**
     * @brief Default constructor that generates a random UUID (v4).
     */
    UUID() : uuid_(::boost::uuids::random_generator()()) {}

    /**
     * @brief Constructs a UUID from a string representation.
     * @param str The string representation of the UUID.
     */
    explicit UUID(const std::string& str)
        : uuid_(::boost::uuids::string_generator()(str)) {}

    /**
     * @brief Constructs a UUID from a Boost.UUID object.
     * @param uuid The Boost.UUID object.
     */
    explicit UUID(const ::boost::uuids::uuid& uuid) : uuid_(uuid) {}

    /**
     * @brief Converts the UUID to a string representation.
     * @return The string representation of the UUID.
     */
    [[nodiscard]] auto toString() const -> std::string {
        return ::boost::uuids::to_string(uuid_);
    }

    /**
     * @brief Checks if the UUID is nil (all zeros).
     * @return True if the UUID is nil, false otherwise.
     */
    [[nodiscard]] auto isNil() const -> bool { return uuid_.is_nil(); }

    /**
     * @brief Compares this UUID with another UUID.
     * @param other The other UUID to compare.
     * @return The result of the comparison.
     */
    auto operator<=>(const UUID& other) const -> std::strong_ordering {
        return uuid_ <=> other.uuid_;
    }

    /**
     * @brief Checks if this UUID is equal to another UUID.
     * @param other The other UUID to compare.
     * @return True if the UUIDs are equal, false otherwise.
     */
    auto operator==(const UUID& other) const -> bool {
        return uuid_ == other.uuid_;
    }

    /**
     * @brief Formats the UUID as a string enclosed in curly braces.
     * @return The formatted string.
     */
    [[nodiscard]] auto format() const -> std::string {
        return std::format("{{{}}}", toString());
    }

    /**
     * @brief Converts the UUID to a vector of bytes.
     * @return The vector of bytes representing the UUID.
     */
    [[nodiscard]] auto toBytes() const -> std::vector<uint8_t> {
        return {uuid_.begin(), uuid_.end()};
    }

    /**
     * @brief Constructs a UUID from a span of bytes.
     * @param bytes The span of bytes.
     * @return The constructed UUID.
     * @throws std::invalid_argument if the span size is not 16 bytes.
     */
    static auto fromBytes(const std::span<const uint8_t>& bytes) -> UUID {
        if (bytes.size() != UUID_SIZE) {
            throw std::invalid_argument("UUID must be exactly 16 bytes");
        }
        ::boost::uuids::uuid uuid;
        std::copy(bytes.begin(), bytes.end(), uuid.begin());
        return UUID(uuid);
    }

    /**
     * @brief Converts the UUID to a 64-bit unsigned integer.
     * @return The 64-bit unsigned integer representation of the UUID.
     */
    [[nodiscard]] auto toUint64() const -> uint64_t {
        return ::boost::lexical_cast<uint64_t>(uuid_);
    }

    /**
     * @brief Gets the DNS namespace UUID.
     * @return The DNS namespace UUID.
     */
    static auto namespaceDNS() -> UUID {
        return UUID(::boost::uuids::ns::dns());
    }

    /**
     * @brief Gets the URL namespace UUID.
     * @return The URL namespace UUID.
     */
    static auto namespaceURL() -> UUID {
        return UUID(::boost::uuids::ns::url());
    }

    /**
     * @brief Gets the OID namespace UUID.
     * @return The OID namespace UUID.
     */
    static auto namespaceOID() -> UUID {
        return UUID(::boost::uuids::ns::oid());
    }

    /**
     * @brief Generates a version 3 (MD5) UUID based on a namespace UUID and a
     * name.
     * @param namespace_uuid The namespace UUID.
     * @param name The name.
     * @return The generated UUID.
     */
    static auto v3(const UUID& namespace_uuid,
                   const std::string& name) -> UUID {
        return UUID(::boost::uuids::name_generator(namespace_uuid.uuid_)(name));
    }

    /**
     * @brief Generates a version 5 (SHA-1) UUID based on a namespace UUID and a
     * name.
     * @param namespace_uuid The namespace UUID.
     * @param name The name.
     * @return The generated UUID.
     */
    static auto v5(const UUID& namespace_uuid,
                   const std::string& name) -> UUID {
        ::boost::uuids::name_generator_sha1 gen(namespace_uuid.uuid_);
        return UUID(gen(name));
    }

    /**
     * @brief Gets the version of the UUID.
     * @return The version of the UUID.
     */
    [[nodiscard]] auto version() const -> int { return uuid_.version(); }

    /**
     * @brief Gets the variant of the UUID.
     * @return The variant of the UUID.
     */
    [[nodiscard]] auto variant() const -> int { return uuid_.variant(); }

    /**
     * @brief Generates a version 1 (timestamp-based) UUID.
     * @return The generated UUID.
     */
    [[nodiscard]] static auto v1() -> UUID {
        static ::boost::uuids::basic_random_generator<std::mt19937> gen;
        return UUID(gen());
    }

    /**
     * @brief Generates a version 4 (random) UUID.
     * @return The generated UUID.
     */
    [[nodiscard]] static auto v4() -> UUID {
        return {};  // Default constructor already generates v4 UUID
    }

    /**
     * @brief Converts the UUID to a Base64 string representation.
     * @return The Base64 string representation of the UUID.
     */
    [[nodiscard]] auto toBase64() const -> std::string {
        static const char* basE64Chars =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string result;
        result.reserve(BASE64_RESERVE_SIZE);

        auto bytes = toBytes();
        for (size_t i = 0; i < bytes.size(); i += 3) {
            uint32_t num =
                (bytes[i] << SHIFT_16) |
                (i + 1 < bytes.size() ? bytes[i + 1] << SHIFT_8 : 0) |
                (i + 2 < bytes.size() ? bytes[i + 2] : 0);
            result += basE64Chars[(num >> BASE64_SHIFT_18) & BASE64_MASK];
            result += basE64Chars[(num >> BASE64_SHIFT_12) & BASE64_MASK];
            result += basE64Chars[(num >> BASE64_SHIFT_6) & BASE64_MASK];
            result += basE64Chars[num & BASE64_MASK];
        }
        result.resize(BASE64_RESERVE_SIZE);  // Remove padding
        return result;
    }

    /**
     * @brief Gets the timestamp from a version 1 UUID.
     * @return The timestamp as a std::chrono::system_clock::time_point.
     * @throws std::runtime_error if the UUID is not version 1.
     */
    [[nodiscard]] auto getTimestamp() const
        -> std::chrono::system_clock::time_point {
        if (version() != 1) {
            throw std::runtime_error(
                "Timestamp is only available for version 1 UUIDs");
        }
        uint64_t timestamp = ((uint64_t)uuid_.data[6] << SHIFT_40) |
                             ((uint64_t)uuid_.data[7] << SHIFT_32) |
                             ((uint64_t)uuid_.data[4] << SHIFT_24) |
                             ((uint64_t)uuid_.data[5] << SHIFT_16) |
                             ((uint64_t)uuid_.data[0] << SHIFT_8) |
                             (uint64_t)uuid_.data[1];
        return std::chrono::system_clock::from_time_t(static_cast<std::time_t>(
            timestamp / TIMESTAMP_DIVISOR - UUID_EPOCH / TIMESTAMP_DIVISOR));
    }

    /**
     * @brief Hash function for UUIDs.
     * @tparam H The hash function type.
     * @param h The hash function.
     * @param uuid The UUID to hash.
     * @return The hash value.
     */
    template <typename H>
    friend auto abslHashValue(H h, const UUID& uuid) -> H {
        return H::combine(std::move(h), uuid.uuid_);
    }

    /**
     * @brief Gets the underlying Boost.UUID object.
     * @return The Boost.UUID object.
     */
    [[nodiscard]] auto getUUID() const -> const ::boost::uuids::uuid& {
        return uuid_;
    }
};
}  // namespace atom::extra::boost

namespace std {
/**
 * @brief Specialization of std::hash for UUID.
 */
template <>
struct hash<atom::extra::boost::UUID> {
    /**
     * @brief Hash function for UUIDs.
     * @param uuid The UUID to hash.
     * @return The hash value.
     */
    auto operator()(const atom::extra::boost::UUID& uuid) const -> size_t {
        return ::boost::hash<::boost::uuids::uuid>()(uuid.getUUID());
    }
};
}  // namespace std

#endif