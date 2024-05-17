#include "short_string.hpp"

#include <stdexcept>

ShortString::ShortString(const std::string& s) {
    if (s.length() > MAX_LENGTH) {
        throw std::invalid_argument("String too long for ShortString");
    }
    str = s;
}

ShortString::ShortString(std::string_view s) {
    if (s.length() > MAX_LENGTH) {
        throw std::invalid_argument("String too long for ShortString");
    }
    str = s;
}

ShortString::ShortString(const char* s) : ShortString(std::string(s)) {}

ShortString::ShortString(const ShortString& other) : str(other.str) {}

ShortString& ShortString::operator=(const ShortString& other) {
    if (this != &other) {
        str = other.str;
    }
    return *this;
}

ShortString& ShortString::operator=(const std::string& s) {
    if (s.length() > MAX_LENGTH) {
        throw std::invalid_argument("String too long for ShortString");
    }
    str = s;
    return *this;
}

ShortString& ShortString::operator=(const char* s) {
    *this = ShortString(s);
    return *this;
}

ShortString& ShortString::operator=(std::string_view s) {
    if (s.length() > MAX_LENGTH) {
        throw std::invalid_argument("String too long for ShortString");
    }
    str = s;
    return *this;
}

std::ostream& operator<<(std::ostream& os, const ShortString& ss) {
    return os << ss.str;
}

ShortString ShortString::operator+(const ShortString& other) const {
    if (str.length() + other.str.length() > MAX_LENGTH) {
        throw std::invalid_argument(
            "Resulting string too long for ShortString");
    }
    return ShortString(str + other.str);
}

ShortString& ShortString::operator+=(const ShortString& other) {
    *this = *this + other;
    return *this;
}

ShortString& ShortString::operator+=(std::string_view other) {
    if (str.length() + other.length() > MAX_LENGTH) {
        throw std::invalid_argument(
            "Resulting string too long for ShortString");
    }
    str += other;
    return *this;
}

bool ShortString::operator==(const ShortString& other) const noexcept {
    return str == other.str;
}

bool ShortString::operator!=(const ShortString& other) const noexcept {
    return !(*this == other);
}

bool ShortString::operator<(const ShortString& other) const noexcept {
    return str < other.str;
}

bool ShortString::operator>(const ShortString& other) const noexcept {
    return str > other.str;
}

bool ShortString::operator<=(const ShortString& other) const noexcept {
    return !(*this > other);
}

bool ShortString::operator>=(const ShortString& other) const noexcept {
    return !(*this < other);
}

char& ShortString::operator[](size_t index) noexcept { return str[index]; }

const char& ShortString::operator[](size_t index) const noexcept {
    return str[index];
}

size_t ShortString::length() const noexcept { return str.length(); }

ShortString ShortString::substr(size_t pos = 0,
                                size_t count = std::string::npos) const {
    if (pos > str.length()) {
        throw std::out_of_range("Starting position out of range");
    }
    return ShortString(str.substr(pos, count));
}

void ShortString::clear() noexcept { str.clear(); }

void ShortString::swap(ShortString& other) noexcept { str.swap(other.str); }
