#ifndef ATOM_EXTRA_INICPP_CONVERT_HPP
#define ATOM_EXTRA_INICPP_CONVERT_HPP

#include <stdexcept>
#include <string>
#include "common.hpp"

namespace inicpp {

/**
 * @brief Template structure for converting between types and strings.
 * @tparam T The type to convert.
 */
template <typename T>
struct Convert {};

/**
 * @brief Specialization of Convert for bool type.
 */
template <>
struct Convert<bool> {
    /**
     * @brief Decodes a string view to a bool.
     * @param value The string view to decode.
     * @param result The resulting bool.
     * @throws std::invalid_argument if the string is not "TRUE" or "FALSE".
     */
    void decode(std::string_view value, bool &result) {
        std::string str(value);
        std::ranges::transform(str, str.begin(), [](char c) {
            return static_cast<char>(::toupper(c));
        });

        if (str == "TRUE")
            result = true;
        else if (str == "FALSE")
            result = false;
        else
            throw std::invalid_argument("field is not a bool");
    }

    /**
     * @brief Encodes a bool to a string.
     * @param value The bool to encode.
     * @param result The resulting string.
     */
    void encode(const bool value, std::string &result) {
        result = value ? "true" : "false";
    }
};

/**
 * @brief Specialization of Convert for char type.
 */
template <>
struct Convert<char> {
    /**
     * @brief Decodes a string view to a char.
     * @param value The string view to decode.
     * @param result The resulting char.
     * @throws std::invalid_argument if the string is empty.
     */
    void decode(std::string_view value, char &result) {
        if (value.empty())
            throw std::invalid_argument("field is empty");
        result = value.front();
    }

    /**
     * @brief Encodes a char to a string.
     * @param value The char to encode.
     * @param result The resulting string.
     */
    void encode(const char value, std::string &result) { result = value; }
};

/**
 * @brief Specialization of Convert for unsigned char type.
 */
template <>
struct Convert<unsigned char> {
    /**
     * @brief Decodes a string view to an unsigned char.
     * @param value The string view to decode.
     * @param result The resulting unsigned char.
     * @throws std::invalid_argument if the string is empty.
     */
    void decode(std::string_view value, unsigned char &result) {
        if (value.empty())
            throw std::invalid_argument("field is empty");
        result = value.front();
    }

    /**
     * @brief Encodes an unsigned char to a string.
     * @param value The unsigned char to encode.
     * @param result The resulting string.
     */
    void encode(const unsigned char value, std::string &result) {
        result = value;
    }
};

/**
 * @brief Specialization of Convert for short type.
 */
template <>
struct Convert<short> {
    /**
     * @brief Decodes a string view to a short.
     * @param value The string view to decode.
     * @param result The resulting short.
     * @throws std::invalid_argument if the string cannot be converted to a
     * short.
     */
    void decode(std::string_view value, short &result) {
        if (auto tmp = strToLong(value); tmp.has_value())
            result = static_cast<short>(tmp.value());
        else
            throw std::invalid_argument("field is not a short");
    }

    /**
     * @brief Encodes a short to a string.
     * @param value The short to encode.
     * @param result The resulting string.
     */
    void encode(const short value, std::string &result) {
        result = std::to_string(value);
    }
};

/**
 * @brief Specialization of Convert for unsigned short type.
 */
template <>
struct Convert<unsigned short> {
    /**
     * @brief Decodes a string view to an unsigned short.
     * @param value The string view to decode.
     * @param result The resulting unsigned short.
     * @throws std::invalid_argument if the string cannot be converted to an
     * unsigned short.
     */
    void decode(std::string_view value, unsigned short &result) {
        if (auto tmp = strToULong(value); tmp.has_value())
            result = static_cast<unsigned short>(tmp.value());
        else
            throw std::invalid_argument("field is not an unsigned short");
    }

    /**
     * @brief Encodes an unsigned short to a string.
     * @param value The unsigned short to encode.
     * @param result The resulting string.
     */
    void encode(const unsigned short value, std::string &result) {
        result = std::to_string(value);
    }
};

/**
 * @brief Specialization of Convert for int type.
 */
template <>
struct Convert<int> {
    /**
     * @brief Decodes a string view to an int.
     * @param value The string view to decode.
     * @param result The resulting int.
     * @throws std::invalid_argument if the string cannot be converted to an
     * int.
     */
    void decode(std::string_view value, int &result) {
        if (auto tmp = strToLong(value); tmp.has_value())
            result = static_cast<int>(tmp.value());
        else
            throw std::invalid_argument("field is not an int");
    }

    /**
     * @brief Encodes an int to a string.
     * @param value The int to encode.
     * @param result The resulting string.
     */
    void encode(const int value, std::string &result) {
        result = std::to_string(value);
    }
};

/**
 * @brief Specialization of Convert for unsigned int type.
 */
template <>
struct Convert<unsigned int> {
    /**
     * @brief Decodes a string view to an unsigned int.
     * @param value The string view to decode.
     * @param result The resulting unsigned int.
     * @throws std::invalid_argument if the string cannot be converted to an
     * unsigned int.
     */
    void decode(std::string_view value, unsigned int &result) {
        if (auto tmp = strToULong(value); tmp.has_value())
            result = static_cast<unsigned int>(tmp.value());
        else
            throw std::invalid_argument("field is not an unsigned int");
    }

    /**
     * @brief Encodes an unsigned int to a string.
     * @param value The unsigned int to encode.
     * @param result The resulting string.
     */
    void encode(const unsigned int value, std::string &result) {
        result = std::to_string(value);
    }
};

/**
 * @brief Specialization of Convert for long type.
 */
template <>
struct Convert<long> {
    /**
     * @brief Decodes a string view to a long.
     * @param value The string view to decode.
     * @param result The resulting long.
     * @throws std::invalid_argument if the string cannot be converted to a
     * long.
     */
    void decode(std::string_view value, long &result) {
        if (auto tmp = strToLong(value); tmp.has_value())
            result = tmp.value();
        else
            throw std::invalid_argument("field is not a long");
    }

    /**
     * @brief Encodes a long to a string.
     * @param value The long to encode.
     * @param result The resulting string.
     */
    void encode(const long value, std::string &result) {
        result = std::to_string(value);
    }
};

/**
 * @brief Specialization of Convert for unsigned long type.
 */
template <>
struct Convert<unsigned long> {
    /**
     * @brief Decodes a string view to an unsigned long.
     * @param value The string view to decode.
     * @param result The resulting unsigned long.
     * @throws std::invalid_argument if the string cannot be converted to an
     * unsigned long.
     */
    void decode(std::string_view value, unsigned long &result) {
        if (auto tmp = strToULong(value); tmp.has_value())
            result = tmp.value();
        else
            throw std::invalid_argument("field is not an unsigned long");
    }

    /**
     * @brief Encodes an unsigned long to a string.
     * @param value The unsigned long to encode.
     * @param result The resulting string.
     */
    void encode(const unsigned long value, std::string &result) {
        result = std::to_string(value);
    }
};

/**
 * @brief Specialization of Convert for double type.
 */
template <>
struct Convert<double> {
    /**
     * @brief Decodes a string view to a double.
     * @param value The string view to decode.
     * @param result The resulting double.
     */
    void decode(std::string_view value, double &result) {
        result = std::stod(std::string(value));
    }

    /**
     * @brief Encodes a double to a string.
     * @param value The double to encode.
     * @param result The resulting string.
     */
    void encode(const double value, std::string &result) {
        result = std::to_string(value);
    }
};

/**
 * @brief Specialization of Convert for float type.
 */
template <>
struct Convert<float> {
    /**
     * @brief Decodes a string view to a float.
     * @param value The string view to decode.
     * @param result The resulting float.
     */
    void decode(std::string_view value, float &result) {
        result = std::stof(std::string(value));
    }

    /**
     * @brief Encodes a float to a string.
     * @param value The float to encode.
     * @param result The resulting string.
     */
    void encode(const float value, std::string &result) {
        result = std::to_string(value);
    }
};

/**
 * @brief Specialization of Convert for std::string type.
 */
template <>
struct Convert<std::string> {
    /**
     * @brief Decodes a string view to a std::string.
     * @param value The string view to decode.
     * @param result The resulting std::string.
     */
    void decode(std::string_view value, std::string &result) { result = value; }

    /**
     * @brief Encodes a std::string to a string.
     * @param value The std::string to encode.
     * @param result The resulting string.
     */
    void encode(const std::string &value, std::string &result) {
        result = value;
    }
};

#ifdef __cpp_lib_string_view
/**
 * @brief Specialization of Convert for std::string_view type.
 */
template <>
struct Convert<std::string_view> {
    /**
     * @brief Decodes a string view to a std::string_view.
     * @param value The string view to decode.
     * @param result The resulting std::string_view.
     */
    void decode(std::string_view value, std::string_view &result) {
        result = value;
    }

    /**
     * @brief Encodes a std::string_view to a string.
     * @param value The std::string_view to encode.
     * @param result The resulting string.
     */
    void encode(std::string_view value, std::string &result) { result = value; }
};
#endif

/**
 * @brief Specialization of Convert for const char* type.
 */
template <>
struct Convert<const char *> {
    /**
     * @brief Encodes a const char* to a string.
     * @param value The const char* to encode.
     * @param result The resulting string.
     */
    void encode(const char *const &value, std::string &result) {
        result = value;
    }

    /**
     * @brief Decodes a string view to a const char*.
     * @param value The string view to decode.
     * @param result The resulting const char*.
     */
    void decode(std::string_view value, const char *&result) {
        result = value.data();
    }
};

/**
 * @brief Specialization of Convert for char arrays.
 * @tparam N The size of the char array.
 */
template <size_t N>
struct Convert<char[N]> {
    /**
     * @brief Decodes a string to a char array.
     * @param value The string to decode.
     * @param result The resulting char array.
     * @throws std::invalid_argument if the string is too large for the char
     * array.
     */
    void decode(const std::string &value, char (&result)[N]) {
        if (value.size() >= N)
            throw std::invalid_argument(
                "field value is too large for the char array");
        std::copy(value.begin(), value.end(), result);
        result[value.size()] = '\0';  // Null-terminate the char array
    }

    /**
     * @brief Encodes a char array to a string.
     * @param value The char array to encode.
     * @param result The resulting string.
     */
    void encode(const char (&value)[N], std::string &result) { result = value; }
};

}  // namespace inicpp

#endif  // ATOM_EXTRA_INICPP_CONVERT_HPP