/*
 * slice.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-12

Description: Slice

**************************************************/

#include "slice.hpp"

#include <algorithm>

StringSlice::StringSlice(std::string_view sv, size_t start, size_t end)
    : sv(sv), start(start), end(end) {}

StringSlice::StringSlice(std::string_view sv)
    : sv(sv), start(0), end(sv.size()) {}

StringSlice::StringSlice(const std::string& s)
    : sv(s), start(0), end(s.size()) {}

StringSlice::StringSlice(const std::string& s, size_t start, size_t end)
    : sv(s), start(start), end(end) {}

char StringSlice::operator[](size_t index) const { return sv[start + index]; }

StringSlice StringSlice::operator()(size_t start, size_t end) const {
    return StringSlice(sv, this->start + start, this->start + end);
}

StringSlice& StringSlice::operator++() {
    if (start < end) {
        ++start;
    }
    return *this;
}

StringSlice StringSlice::operator++(int) {
    StringSlice temp = *this;
    ++(*this);
    return temp;
}

StringSlice& StringSlice::operator--() {
    if (start > 0) {
        --start;
    }
    return *this;
}

StringSlice StringSlice::operator--(int) {
    StringSlice temp = *this;
    --(*this);
    return temp;
}

StringSlice& StringSlice::operator+=(size_t n) {
    if (start + n <= end) {
        start += n;
    } else {
        start = end;
    }
    return *this;
}

StringSlice& StringSlice::operator-=(size_t n) {
    if (start >= n) {
        start -= n;
    } else {
        start = 0;
    }
    return *this;
}

StringSlice StringSlice::operator+(size_t n) const {
    StringSlice temp = *this;
    return temp += n;
}

StringSlice StringSlice::operator-(size_t n) const {
    StringSlice temp = *this;
    return temp -= n;
}

StringSlice::operator std::string() const {
    return std::string(sv.substr(start, end - start));
}

size_t StringSlice::size() const { return end - start; }

bool StringSlice::empty() const { return start == end; }

char StringSlice::front() const { return sv[start]; }

char StringSlice::back() const { return sv[end - 1]; }

void StringSlice::remove_prefix(size_t n) { start = std::min(start + n, end); }

void StringSlice::remove_suffix(size_t n) { end = std::max(end - n, start); }

void StringSlice::swap(StringSlice& other) {
    std::swap(sv, other.sv);
    std::swap(start, other.start);
    std::swap(end, other.end);
}

StringSlice& StringSlice::ltrim() {
    while (start < end && std::isspace(sv[start])) {
        ++start;
    }
    return *this;
}

StringSlice& StringSlice::rtrim() {
    while (end > start && std::isspace(sv[end - 1])) {
        --end;
    }
    return *this;
}

StringSlice& StringSlice::trim() { return ltrim().rtrim(); }

StringSlice StringSlice::substr(size_t pos = 0,
                                size_t count = std::string::npos) const {
    pos = std::min(pos, size());
    count = std::min(count, size() - pos);
    return StringSlice(sv, start + pos, start + pos + count);
}

bool StringSlice::equal(std::string_view other) const {
    return std::equal(sv.begin() + start, sv.begin() + end, other.begin(),
                      other.end());
}

bool StringSlice::equal(size_t pos, size_t count,
                        std::string_view other) const {
    return count == other.size() &&
           std::equal(sv.begin() + start + pos,
                      sv.begin() + start + pos + count, other.begin());
}

bool StringSlice::starts_with(std::string_view prefix) const {
    return size() >= prefix.size() && equal(0, prefix.size(), prefix);
}

bool StringSlice::ends_with(std::string_view suffix) const {
    return size() >= suffix.size() &&
           equal(size() - suffix.size(), size(), suffix);
}

size_t StringSlice::find(std::string_view target, size_t pos = 0) const {
    auto it = std::search(sv.begin() + start + pos, sv.begin() + end,
                          target.begin(), target.end());
    return it == sv.begin() + end ? std::string::npos
                                  : it - (sv.begin() + start);
}

size_t StringSlice::rfind(std::string_view target,
                          size_t pos = std::string::npos) const {
    pos = std::min(pos, size());
    auto it = std::find_end(sv.begin() + start, sv.begin() + start + pos,
                            target.begin(), target.end());
    return it == sv.begin() + start + pos ? std::string::npos
                                          : it - (sv.begin() + start);
}

std::ostream& operator<<(std::ostream& os, const StringSlice& slice) {
    os << static_cast<std::string>(slice);
    return os;
}

bool operator==(const StringSlice& lhs, const StringSlice& rhs) {
    return lhs.sv == rhs.sv && lhs.start == rhs.start && lhs.end == rhs.end;
}

bool operator==(const StringSlice& lhs, const std::string& rhs) {
    return lhs.sv.data() == rhs.data() && lhs.start == 0 &&
           lhs.end == lhs.sv.size();
}

bool operator==(const StringSlice& lhs, std::string_view rhs) {
    return lhs.sv == rhs && lhs.start == 0 && lhs.end == lhs.sv.size();
}

bool operator==(const StringSlice& lhs, const char* rhs) {
    return lhs.sv.data() == rhs && lhs.start == 0 && lhs.end == lhs.sv.size();
}

bool operator!=(const StringSlice& lhs, const StringSlice& rhs) {
    return lhs.sv != rhs.sv || lhs.start != rhs.start || lhs.end != rhs.end;
}

bool operator<=(const StringSlice& lhs, const StringSlice& rhs) {
    return lhs.sv.compare(lhs.start, lhs.end - lhs.start, rhs.sv, rhs.start,
                          rhs.end - rhs.start) <= 0;
}

bool operator<(const StringSlice& lhs, const StringSlice& rhs) {
    return lhs.sv.compare(lhs.start, lhs.end - lhs.start, rhs.sv, rhs.start,
                          rhs.end - rhs.start) < 0;
}

bool operator>=(const StringSlice& lhs, const StringSlice& rhs) {
    return lhs.sv.compare(lhs.start, lhs.end - lhs.start, rhs.sv, rhs.start,
                          rhs.end - rhs.start) >= 0;
}

bool operator>(const StringSlice& lhs, const StringSlice& rhs) {
    return lhs.sv.compare(lhs.start, lhs.end - lhs.start, rhs.sv, rhs.start,
                          rhs.end - rhs.start) > 0;
}

StringSlice operator+(const StringSlice& lhs, const StringSlice& rhs) {
    return StringSlice(lhs.sv, lhs.start, lhs.end + rhs.end - rhs.start);
}

StringSlice operator+(const StringSlice& lhs, const std::string& rhs) {
    return lhs + StringSlice(rhs);
}

StringSlice operator+(const StringSlice& lhs, std::string_view rhs) {
    return lhs + StringSlice(rhs);
}

StringSlice operator+(const std::string& lhs, const StringSlice& rhs) {
    return StringSlice(lhs) + rhs;
}

StringSlice operator+(std::string_view lhs, const StringSlice& rhs) {
    return StringSlice(lhs) + rhs;
}

StringSlice operator+(const StringSlice& lhs, const char* rhs) {
    return lhs + std::string(rhs);
}

StringSlice operator+(const char* lhs, const StringSlice& rhs) {
    return std::string(lhs) + rhs;
}