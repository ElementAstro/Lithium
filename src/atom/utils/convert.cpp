/*
 * convert.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-18

Description: Convert Utils for Windows

**************************************************/

#ifdef _WIN32

#include "convert.hpp"

#include <stdexcept>
#include <vector>

namespace Atom::Utils {
LPWSTR CharToLPWSTR(std::string_view charString) {
    const int size =
        MultiByteToWideChar(CP_ACP, 0, charString.data(),
                            static_cast<int>(charString.size()), nullptr, 0);
    if (size == 0) {
        throw std::runtime_error("Error converting char string to LPWSTR");
    }
    std::vector<wchar_t> buffer(size + 1);
    MultiByteToWideChar(CP_ACP, 0, charString.data(),
                        static_cast<int>(charString.size()), buffer.data(),
                        size);
    buffer[size] = L'\0';
    return buffer.data();
}

LPWSTR StringToLPWSTR(const std::string& str) { return CharToLPWSTR(str); }

std::string LPWSTRToString(LPWSTR lpwstr) {
    const int size = WideCharToMultiByte(CP_ACP, 0, lpwstr, -1, nullptr, 0,
                                         nullptr, nullptr);
    if (size == 0) {
        throw std::runtime_error("Error converting LPWSTR to std::string");
    }
    std::string str(size - 1, '\0');
    WideCharToMultiByte(CP_ACP, 0, lpwstr, -1, str.data(), size, nullptr,
                        nullptr);
    return str;
}

std::string LPCWSTRToString(LPCWSTR lpcwstr) {
    return LPWSTRToString(const_cast<LPWSTR>(lpcwstr));
}

LPWSTR WStringToLPWSTR(const std::wstring& wstr) {
    std::vector<wchar_t> buffer(wstr.size() + 1);
    std::copy(wstr.begin(), wstr.end(), buffer.begin());
    buffer[wstr.size()] = L'\0';
    return buffer.data();
}

std::wstring LPWSTRToWString(LPWSTR lpwstr) { return std::wstring(lpwstr); }

std::wstring LPCWSTRToWString(LPCWSTR lpcwstr) { return std::wstring(lpcwstr); }

}  // namespace Atom::Utils

#endif