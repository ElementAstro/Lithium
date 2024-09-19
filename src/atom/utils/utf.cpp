#include "utf.hpp"

#include <string>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

#include "atom/error/exception.hpp"

namespace atom::utils {
// UTF-16 to UTF-8 using Windows API or manual conversion for non-Windows
// platforms
auto utF16toUtF8(std::u16string_view str) -> std::string {
#if defined(_WIN32) || defined(_WIN64)
    if (str.empty()) {
        return {};
    }
    int sizeNeeded = WideCharToMultiByte(
        CP_UTF8, 0, reinterpret_cast<const wchar_t *>(str.data()), str.size(),
        nullptr, 0, nullptr, nullptr);
    std::string result(sizeNeeded, 0);
    WideCharToMultiByte(
        CP_UTF8, 0, reinterpret_cast<const wchar_t *>(str.data()), str.size(),
        result.data(), sizeNeeded, nullptr, nullptr);
    return result;
#else
    std::string result;
    result.reserve(str.size() * 3);  // Reserve a reasonable initial size
    for (char16_t c : str) {
        if (c <= 0x7F) {
            result.push_back(static_cast<char>(c));
        } else if (c <= 0x7FF) {
            result.push_back(static_cast<char>(0xC0 | ((c >> 6) & 0x1F)));
            result.push_back(static_cast<char>(0x80 | (c & 0x3F)));
        } else {
            result.push_back(static_cast<char>(0xE0 | ((c >> 12) & 0x0F)));
            result.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | (c & 0x3F)));
        }
    }
    return result;
#endif
}

// UTF-8 to UTF-16 using Windows API or manual conversion for non-Windows
// platforms
auto utF8toUtF16(std::string_view str) -> std::u16string {
#if defined(_WIN32) || defined(_WIN64)
    if (str.empty()) {
        return {};
    }
    int sizeNeeded =
        MultiByteToWideChar(CP_UTF8, 0, str.data(), str.size(), nullptr, 0);
    std::u16string result(sizeNeeded, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.data(), str.size(),
                        reinterpret_cast<wchar_t *>(result.data()), sizeNeeded);
    return result;
#else
    std::u16string result;
    size_t i = 0;
    while (i < str.size()) {
        unsigned char c = static_cast<unsigned char>(str[i++]);
        if (c <= 0x7F) {
            result.push_back(static_cast<char16_t>(c));
        } else if (c <= 0xDF) {
            if (i >= str.size())
                THROW_INVALID_ARGUMENT(
                    "Invalid UTF-8 string: unexpected end of input");
            unsigned char c2 = static_cast<unsigned char>(str[i++]);
            if ((c2 & 0xC0) != 0x80)
                THROW_INVALID_ARGUMENT(
                    "Invalid UTF-8 string: invalid continuation byte");
            result.push_back(((c & 0x1F) << 6) | (c2 & 0x3F));
        } else if (c <= 0xEF) {
            if (i + 1 >= str.size())
                THROW_INVALID_ARGUMENT(
                    "Invalid UTF-8 string: unexpected end of input");
            unsigned char c2 = static_cast<unsigned char>(str[i++]);
            unsigned char c3 = static_cast<unsigned char>(str[i++]);
            if ((c2 & 0xC0) != 0x80 || (c3 & 0xC0) != 0x80)
                THROW_INVALID_ARGUMENT(
                    "Invalid UTF-8 string: invalid continuation byte");
            result.push_back(((c & 0x0F) << 12) | ((c2 & 0x3F) << 6) |
                             (c3 & 0x3F));
        } else {
            THROW_INVALID_ARGUMENT(
                "Invalid UTF-8 string: invalid starting byte");
        }
    }
    return result;
#endif
}

// UTF-32 to UTF-8 conversion
auto utF32toUtF8(std::u32string_view str) -> std::string {
    std::string result;
    result.reserve(str.size() *
                   4);  // UTF-8 could be up to 4 bytes per UTF-32 code point
    for (char32_t c : str) {
        if (c <= 0x7F) {
            result.push_back(static_cast<char>(c));
        } else if (c <= 0x7FF) {
            result.push_back(static_cast<char>(0xC0 | ((c >> 6) & 0x1F)));
            result.push_back(static_cast<char>(0x80 | (c & 0x3F)));
        } else if (c <= 0xFFFF) {
            result.push_back(static_cast<char>(0xE0 | ((c >> 12) & 0x0F)));
            result.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | (c & 0x3F)));
        } else {
            result.push_back(static_cast<char>(0xF0 | ((c >> 18) & 0x07)));
            result.push_back(static_cast<char>(0x80 | ((c >> 12) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | (c & 0x3F)));
        }
    }
    return result;
}

// UTF-8 to UTF-32 conversion
auto utF8toUtF32(std::string_view str) -> std::u32string {
    std::u32string result;
    size_t i = 0;
    while (i < str.size()) {
        auto c = static_cast<unsigned char>(str[i++]);
        if (c <= 0x7F) {
            result.push_back(static_cast<char32_t>(c));
        } else if (c <= 0xDF) {
            if (i >= str.size()) {
                THROW_INVALID_ARGUMENT(
                    "Invalid UTF-8 string: unexpected end of input");
            }
            auto c2 = static_cast<unsigned char>(str[i++]);
            if ((c2 & 0xC0) != 0x80) {
                THROW_INVALID_ARGUMENT(
                    "Invalid UTF-8 string: invalid continuation byte");
            }
            result.push_back(((c & 0x1F) << 6) | (c2 & 0x3F));
        } else if (c <= 0xEF) {
            if (i + 1 >= str.size()) {
                THROW_INVALID_ARGUMENT(
                    "Invalid UTF-8 string: unexpected end of input");
            }
            auto c2 = static_cast<unsigned char>(str[i++]);
            auto c3 = static_cast<unsigned char>(str[i++]);
            if ((c2 & 0xC0) != 0x80 || (c3 & 0xC0) != 0x80) {
                THROW_INVALID_ARGUMENT(
                    "Invalid UTF-8 string: invalid continuation byte");
            }
            result.push_back(((c & 0x0F) << 12) | ((c2 & 0x3F) << 6) |
                             (c3 & 0x3F));
        } else if (c <= 0xF7) {
            if (i + 2 >= str.size()) {
                THROW_INVALID_ARGUMENT(
                    "Invalid UTF-8 string: unexpected end of input");
            }
            auto c2 = static_cast<unsigned char>(str[i++]);
            auto c3 = static_cast<unsigned char>(str[i++]);
            auto c4 = static_cast<unsigned char>(str[i++]);
            if ((c2 & 0xC0) != 0x80 || (c3 & 0xC0) != 0x80 ||
                (c4 & 0xC0) != 0x80) {
                THROW_INVALID_ARGUMENT(
                    "Invalid UTF-8 string: invalid continuation byte");
            }
            result.push_back(((c & 0x07) << 18) | ((c2 & 0x3F) << 12) |
                             ((c3 & 0x3F) << 6) | (c4 & 0x3F));
        } else {
            THROW_INVALID_ARGUMENT(
                "Invalid UTF-8 string: invalid starting byte");
        }
    }
    return result;
}

auto isHighSurrogate(char16_t c) -> bool { return (c & 0xFC00) == 0xD800; }

auto isLowSurrogate(char16_t c) -> bool { return (c & 0xFC00) == 0xDC00; }

auto surrogateToCodepoint(char16_t high, char16_t low) -> char32_t {
    return ((high - 0xD800) << 10) + (low - 0xDC00) + 0x10000;
}

// UTF-16 to UTF-32 conversion
auto utF16toUtF32(std::u16string_view str) -> std::u32string {
    std::u32string result;
    auto iterator = str.begin();  // Renamed 'it' to 'iterator' and fixed type
    while (iterator != str.end()) {
        char16_t codeUnit = *iterator++;
        if (isHighSurrogate(codeUnit)) {
            if (iterator == str.end() || !isLowSurrogate(*iterator)) {
                THROW_INVALID_ARGUMENT(
                    "Invalid UTF-16 string: incomplete surrogate pair");
            }
            char32_t codePoint = surrogateToCodepoint(codeUnit, *iterator++);
            result.push_back(codePoint);
        } else if (isLowSurrogate(codeUnit)) {
            THROW_INVALID_ARGUMENT(
                "Invalid UTF-16 string: unexpected low surrogate");
        } else {
            result.push_back(static_cast<char32_t>(codeUnit));
        }
    }
    return result;
}

// UTF-32 to UTF-16 conversion
auto utF32toUtF16(std::u32string_view str) -> std::u16string {
    std::u16string result;
    for (char32_t codePoint : str) {
        if (codePoint <= 0xFFFF) {
            result.push_back(static_cast<char16_t>(codePoint));
        } else if (codePoint <= 0x10FFFF) {
            char16_t highSurrogate =
                static_cast<char16_t>((codePoint - 0x10000) >> 10) + 0xD800;
            char16_t lowSurrogate =
                static_cast<char16_t>((codePoint - 0x10000) & 0x3FF) + 0xDC00;
            result.push_back(highSurrogate);
            result.push_back(lowSurrogate);
        } else {
            THROW_INVALID_ARGUMENT(
                "Invalid UTF-32 code point: out of Unicode range");
        }
    }
    return result;
}

// UTF-8 validation
auto isValidUTF8(std::string_view str) -> bool {
    int continuationBytes = 0;
    for (unsigned char c : str) {
        if (continuationBytes == 0) {
            if ((c >> 5) == 0x6) {  // 110xxxxx, 2-byte sequence
                continuationBytes = 1;
            } else if ((c >> 4) == 0xe) {  // 1110xxxx, 3-byte sequence
                continuationBytes = 2;
            } else if ((c >> 3) == 0x1e) {  // 11110xxx, 4-byte sequence
                continuationBytes = 3;
            } else if (c >> 7) {  // 10xxxxxx, invalid as initial byte
                return false;
            }
        } else {
            if ((c >> 6) != 0x2) {  // Not a continuation byte
                return false;
            }
            --continuationBytes;
        }
    }
    return continuationBytes == 0;
}
}  // namespace atom::utils
