#ifndef LITHIUM_IMAGE_FITSKEYWORD_HPP
#define LITHIUM_IMAGE_FITSKEYWORD_HPP

#include <fitsio.h>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>

// Concepts for type constraints
template <typename T>
concept FitsValueType = std::is_same_v<T, std::string> ||
                        std::is_same_v<T, int64_t> || std::is_same_v<T, double>;

class FITSRecord {
public:
    enum class Type {
        VOID,
        COMMENT,
        STRING = TSTRING,
        LONGLONG = TLONGLONG,
        DOUBLE = TDOUBLE
    };

    // Constructors
    constexpr FITSRecord() noexcept = default;

    FITSRecord(std::string_view key, std::string_view value,
               std::string_view comment = {}) noexcept
        : m_key(key),
          m_value(std::string(value)),
          m_comment(comment),
          m_type(Type::STRING) {}

    FITSRecord(std::string_view key, int64_t value,
               std::string_view comment = {}) noexcept
        : m_key(key),
          m_value(value),
          m_comment(comment),
          m_type(Type::LONGLONG) {}

    FITSRecord(std::string_view key, double value, int decimal = 6,
               std::string_view comment = {}) noexcept
        : m_key(key),
          m_value(value),
          m_comment(comment),
          m_type(Type::DOUBLE),
          m_decimal(decimal) {}

    explicit FITSRecord(std::string_view comment) noexcept
        : m_comment(comment), m_type(Type::COMMENT) {}

    // Accessors
    [[nodiscard]] constexpr Type type() const noexcept { return m_type; }
    [[nodiscard]] const std::string& key() const& noexcept { return m_key; }
    [[nodiscard]] const std::string& comment() const& noexcept {
        return m_comment;
    }
    [[nodiscard]] constexpr int decimal() const noexcept { return m_decimal; }

    // Value accessors with type safety
    [[nodiscard]] std::string valueString() const {
        if (const auto* str = std::get_if<std::string>(&m_value)) {
            return *str;
        }
        throw std::runtime_error("Value is not a string");
    }

    [[nodiscard]] int64_t valueInt() const {
        if (const auto* val = std::get_if<int64_t>(&m_value)) {
            return *val;
        }
        throw std::runtime_error("Value is not an integer");
    }

    [[nodiscard]] double valueDouble() const {
        if (const auto* val = std::get_if<double>(&m_value)) {
            return *val;
        }
        throw std::runtime_error("Value is not a double");
    }

    // Generic value setter with type constraints
    template <FitsValueType T>
    void setValue(const T& value) {
        m_value = value;
        if constexpr (std::is_same_v<T, std::string>) {
            m_type = Type::STRING;
        } else if constexpr (std::is_same_v<T, int64_t>) {
            m_type = Type::LONGLONG;
        } else if constexpr (std::is_same_v<T, double>) {
            m_type = Type::DOUBLE;
        }
    }

private:
    std::string m_key;
    std::variant<std::monostate, std::string, int64_t, double> m_value;
    std::string m_comment;
    Type m_type{Type::VOID};
    int m_decimal{6};
};

inline namespace fits_literals {
[[nodiscard]] inline auto operator""_fits_comment(const char* str,
                                                  size_t) -> FITSRecord {
    return FITSRecord(str);
}
}  // namespace fits_literals

#endif