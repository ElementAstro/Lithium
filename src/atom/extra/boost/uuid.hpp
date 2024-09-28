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

class UUID {
private:
    boost::uuids::uuid uuid_;

public:
    UUID() : uuid_(boost::uuids::random_generator()()) {}

    explicit UUID(const std::string& str)
        : uuid_(boost::uuids::string_generator()(str)) {}

    explicit UUID(const boost::uuids::uuid& uuid) : uuid_(uuid) {}

    [[nodiscard]] auto toString() const -> std::string {
        return boost::uuids::to_string(uuid_);
    }

    [[nodiscard]] auto isNil() const -> bool { return uuid_.is_nil(); }

    auto operator<=>(const UUID& other) const -> std::strong_ordering {
        return uuid_ <=> other.uuid_;
    }

    auto operator==(const UUID& other) const -> bool {
        return uuid_ == other.uuid_;
    }

    [[nodiscard]] auto format() const -> std::string {
        return std::format("{{{}}}", toString());
    }

    [[nodiscard]] auto toBytes() const -> std::vector<uint8_t> {
        return {uuid_.begin(), uuid_.end()};
    }

    static auto fromBytes(const std::span<const uint8_t>& bytes) -> UUID {
        if (bytes.size() != UUID_SIZE) {
            throw std::invalid_argument("UUID must be exactly 16 bytes");
        }
        boost::uuids::uuid uuid;
        std::copy(bytes.begin(), bytes.end(), uuid.begin());
        return UUID(uuid);
    }

    [[nodiscard]] auto toUint64() const -> uint64_t {
        return boost::lexical_cast<uint64_t>(uuid_);
    }

    static auto namespaceDNS() -> UUID { return UUID(boost::uuids::ns::dns()); }
    static auto namespaceURL() -> UUID { return UUID(boost::uuids::ns::url()); }
    static auto namespaceOID() -> UUID { return UUID(boost::uuids::ns::oid()); }

    static auto v3(const UUID& namespace_uuid,
                   const std::string& name) -> UUID {
        return UUID(boost::uuids::name_generator(namespace_uuid.uuid_)(name));
    }

    static auto v5(const UUID& namespace_uuid,
                   const std::string& name) -> UUID {
        boost::uuids::name_generator_sha1 gen(namespace_uuid.uuid_);
        return UUID(gen(name));
    }

    [[nodiscard]] auto version() const -> int { return uuid_.version(); }
    [[nodiscard]] auto variant() const -> int { return uuid_.variant(); }

    [[nodiscard]] static auto v1() -> UUID {
        static boost::uuids::basic_random_generator<std::mt19937> gen;
        return UUID(gen());
    }

    [[nodiscard]] static auto v4() -> UUID {
        return {};  // Default constructor already generates v4 UUID
    }

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

    template <typename H>
    friend auto abslHashValue(H h, const UUID& uuid) -> H {
        return H::combine(std::move(h), uuid.uuid_);
    }

    [[nodiscard]] auto getUUID() const -> const boost::uuids::uuid& {
        return uuid_;
    }
};

namespace std {
template <>
struct hash<UUID> {
    auto operator()(const UUID& uuid) const -> size_t {
        return boost::hash<boost::uuids::uuid>()(uuid.getUUID());
    }
};
}  // namespace std
