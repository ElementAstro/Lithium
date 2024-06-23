#include "utf.hpp"

#include <algorithm>
#include <cassert>
#include <codecvt>
#include <cstdint>
#include <locale>
#include <vector>

#include "atom/error/exception.hpp"

namespace atom::utils {

std::string toUTF8(std::wstring_view wstr) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
    return convert.to_bytes(wstr.data(), wstr.data() + wstr.size());
}

std::wstring fromUTF8(std::string_view str) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
    return convert.from_bytes(str.data(), str.data() + str.size());
}

std::u16string UTF8toUTF16(std::string_view str) {
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
    return convert.from_bytes(str.data(), str.data() + str.size());
}

std::u32string UTF8toUTF32(std::string_view str) {
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
    return convert.from_bytes(str.data(), str.data() + str.size());
}

std::string UTF16toUTF8(std::u16string_view str) {
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
    return convert.to_bytes(str.data(), str.data() + str.size());
}

bool is_high_surrogate(char16_t c) { return (c & 0xFC00) == 0xD800; }

bool is_low_surrogate(char16_t c) { return (c & 0xFC00) == 0xDC00; }

char32_t surrogate_to_codepoint(char16_t high, char16_t low) {
    return ((high - 0xD800) << 10) + (low - 0xDC00) + 0x10000;
}

std::u32string UTF16toUTF32(std::u16string_view str) {
    std::u32string result;
    auto it = str.begin();
    while (it != str.end()) {
        char16_t code_unit = *it++;
        if (is_high_surrogate(code_unit)) {
            if (it == str.end() || !is_low_surrogate(*it)) {
                THROW_INVALID_ARGUMENT(
                    "Invalid UTF-16 string: incomplete surrogate pair");
            }
            char32_t code_point = surrogate_to_codepoint(code_unit, *it++);
            result.push_back(code_point);
        } else if (is_low_surrogate(code_unit)) {
            THROW_INVALID_ARGUMENT(
                "Invalid UTF-16 string: unexpected low surrogate");
        } else {
            result.push_back(static_cast<char32_t>(code_unit));
        }
    }
    return result;
}

std::string UTF32toUTF8(std::u32string_view str) {
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
    return convert.to_bytes(str.data(), str.data() + str.size());
}

std::u16string UTF32toUTF16(std::u32string_view str) {
    std::wstring_convert<std::codecvt_utf16<char32_t>, char32_t> convert;
    std::string bytes = convert.to_bytes(
        reinterpret_cast<const char32_t *>(str.data()),
        reinterpret_cast<const char32_t *>(str.data() + str.size()));
    std::u16string result;
    for (size_t i = 0; i < bytes.size(); i += 2) {
        char16_t codepoint = (uint8_t)bytes[i] << 8 | (uint8_t)bytes[i + 1];
        result.push_back(codepoint);
    }
    return result;
}

bool isValidUTF8(std::string_view str) {
    int continuation_bytes = 0;
    for (unsigned char c : str) {
        if (continuation_bytes == 0) {
            if ((c >> 5) == 0x6)
                continuation_bytes = 1;
            else if ((c >> 4) == 0xe)
                continuation_bytes = 2;
            else if ((c >> 3) == 0x1e)
                continuation_bytes = 3;
            else if (c >> 7)
                return false;
        } else {
            if ((c >> 6) != 0x2)
                return false;
            --continuation_bytes;
        }
    }
    return continuation_bytes == 0;
}

}  // namespace atom::utils