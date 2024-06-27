/*
 * short_string.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-5-21

Description: ShortString for Atom

**************************************************/

#include "short_string.hpp"

#include "atom/error/exception.hpp"

namespace atom::type {
ShortString::ShortString(const std::string& s) {
    if (s.length() > MAX_LENGTH) {
        THROW_INVALID_ARGUMENT("String too long for ShortString");
    }
    str_ = s;
}

ShortString::ShortString(std::string_view s) {
    if (s.length() > MAX_LENGTH) {
        THROW_INVALID_ARGUMENT("String too long for ShortString");
    }
    str_ = s;
}

ShortString::ShortString(const char* s) : ShortString(std::string_view(s)) {}

auto ShortString::operator=(const std::string& s) -> ShortString& {
    if (s.length() > MAX_LENGTH) {
        THROW_INVALID_ARGUMENT("String too long for ShortString");
    }
    str_ = s;
    return *this;
}

auto ShortString::operator=(const char* s) -> ShortString& {
    *this = std::string_view(s);
    return *this;
}

auto ShortString::operator=(std::string_view s) -> ShortString& {
    if (s.length() > MAX_LENGTH) {
        THROW_INVALID_ARGUMENT("String too long for ShortString");
    }
    str_ = s;
    return *this;
}

auto operator<<(std::ostream& os, const ShortString& ss) -> std::ostream& {
    return os << ss.str_;
}

auto ShortString::operator+(const ShortString& other) const -> ShortString {
    if (str_.length() + other.str_.length() > MAX_LENGTH) {
        THROW_INVALID_ARGUMENT("Resulting string too long for ShortString");
    }
    return ShortString(str_ + other.str_);
}

auto ShortString::operator+=(const ShortString& other) -> ShortString& {
    *this = *this + other;
    return *this;
}

auto ShortString::operator+=(std::string_view other) -> ShortString& {
    if (str_.length() + other.length() > MAX_LENGTH) {
        THROW_INVALID_ARGUMENT("Resulting string too long for ShortString");
    }
    str_ += other;
    return *this;
}

auto ShortString::operator==(const ShortString& other) const noexcept -> bool {
    return str_ == other.str_;
}

auto ShortString::operator!=(const ShortString& other) const noexcept -> bool {
    return !(*this == other);
}

auto ShortString::operator<(const ShortString& other) const noexcept -> bool {
    return str_ < other.str_;
}

auto ShortString::operator>(const ShortString& other) const noexcept -> bool {
    return str_ > other.str_;
}

auto ShortString::operator<=(const ShortString& other) const noexcept -> bool {
    return !(*this > other);
}

auto ShortString::operator>=(const ShortString& other) const noexcept -> bool {
    return !(*this < other);
}

auto ShortString::operator[](size_t index) noexcept -> char& {
    return str_[index];
}

auto ShortString::operator[](size_t index) const noexcept -> const char& {
    return str_[index];
}

auto ShortString::length() const noexcept -> size_t { return str_.length(); }

auto ShortString::substr(size_t pos, size_t count) const -> ShortString {
    if (pos > str_.length()) {
        THROW_OUT_OF_RANGE("Starting position out of range");
    }
    return ShortString(str_.substr(pos, count));
}

void ShortString::clear() noexcept { str_.clear(); }

void ShortString::swap(ShortString& other) noexcept { str_.swap(other.str_); }

}  // namespace atom::type
