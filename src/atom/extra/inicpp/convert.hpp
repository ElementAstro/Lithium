#ifndef ATOM_EXTRA_INICPP_CONVERT_HPP
#define ATOM_EXTRA_INICPP_CONVERT_HPP

#include <stdexcept>
#include <string>
#include "common.hpp"

namespace inicpp {

template <typename T>
struct Convert {};

template <>
struct Convert<bool> {
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

    void encode(const bool value, std::string &result) {
        result = value ? "true" : "false";
    }
};

template <>
struct Convert<char> {
    void decode(std::string_view value, char &result) {
        if (value.empty())
            throw std::invalid_argument("field is empty");
        result = value.front();
    }

    void encode(const char value, std::string &result) { result = value; }
};

template <>
struct Convert<unsigned char> {
    void decode(std::string_view value, unsigned char &result) {
        if (value.empty())
            throw std::invalid_argument("field is empty");
        result = value.front();
    }

    void encode(const unsigned char value, std::string &result) {
        result = value;
    }
};

template <>
struct Convert<short> {
    void decode(std::string_view value, short &result) {
        if (auto tmp = strToLong(value); tmp.has_value())
            result = static_cast<short>(tmp.value());
        else
            throw std::invalid_argument("field is not a short");
    }

    void encode(const short value, std::string &result) {
        result = std::to_string(value);
    }
};

template <>
struct Convert<unsigned short> {
    void decode(std::string_view value, unsigned short &result) {
        if (auto tmp = strToULong(value); tmp.has_value())
            result = static_cast<unsigned short>(tmp.value());
        else
            throw std::invalid_argument("field is not an unsigned short");
    }

    void encode(const unsigned short value, std::string &result) {
        result = std::to_string(value);
    }
};

template <>
struct Convert<int> {
    void decode(std::string_view value, int &result) {
        if (auto tmp = strToLong(value); tmp.has_value())
            result = static_cast<int>(tmp.value());
        else
            throw std::invalid_argument("field is not an int");
    }

    void encode(const int value, std::string &result) {
        result = std::to_string(value);
    }
};

template <>
struct Convert<unsigned int> {
    void decode(std::string_view value, unsigned int &result) {
        if (auto tmp = strToULong(value); tmp.has_value())
            result = static_cast<unsigned int>(tmp.value());
        else
            throw std::invalid_argument("field is not an unsigned int");
    }

    void encode(const unsigned int value, std::string &result) {
        result = std::to_string(value);
    }
};

template <>
struct Convert<long> {
    void decode(std::string_view value, long &result) {
        if (auto tmp = strToLong(value); tmp.has_value())
            result = tmp.value();
        else
            throw std::invalid_argument("field is not a long");
    }

    void encode(const long value, std::string &result) {
        result = std::to_string(value);
    }
};

template <>
struct Convert<unsigned long> {
    void decode(std::string_view value, unsigned long &result) {
        if (auto tmp = strToULong(value); tmp.has_value())
            result = tmp.value();
        else
            throw std::invalid_argument("field is not an unsigned long");
    }

    void encode(const unsigned long value, std::string &result) {
        result = std::to_string(value);
    }
};

template <>
struct Convert<double> {
    void decode(std::string_view value, double &result) {
        result = std::stod(std::string(value));
    }

    void encode(const double value, std::string &result) {
        result = std::to_string(value);
    }
};

template <>
struct Convert<float> {
    void decode(std::string_view value, float &result) {
        result = std::stof(std::string(value));
    }

    void encode(const float value, std::string &result) {
        result = std::to_string(value);
    }
};

template <>
struct Convert<std::string> {
    void decode(std::string_view value, std::string &result) { result = value; }

    void encode(const std::string &value, std::string &result) {
        result = value;
    }
};

#ifdef __cpp_lib_string_view
template <>
struct Convert<std::string_view> {
    void decode(std::string_view value, std::string_view &result) {
        result = value;
    }

    void encode(std::string_view value, std::string &result) { result = value; }
};
#endif

template <>
struct Convert<const char *> {
    void encode(const char *const &value, std::string &result) {
        result = value;
    }

    void decode(std::string_view value, const char *&result) {
        result = value.data();
    }
};

// 对 char[n] 进行模板特化的支持
template <size_t N>
struct Convert<char[N]> {
    void decode(const std::string &value, char (&result)[N]) {
        if (value.size() >= N)
            throw std::invalid_argument(
                "field value is too large for the char array");
        std::copy(value.begin(), value.end(), result);
        result[value.size()] = '\0';  // Null-terminate the char array
    }

    void encode(const char (&value)[N], std::string &result) { result = value; }
};
}  // namespace inicpp

#endif  // ATOM_EXTRA_INICPP_CONVERT_HPP