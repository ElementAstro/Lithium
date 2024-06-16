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

namespace atom::utils {
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

std::string WCharArrayToString(const WCHAR* wCharArray) {
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wCharArray, -1, NULL, 0, NULL, NULL);
    std::string str(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wCharArray, -1, &str[0], size_needed, NULL, NULL);
    return str;
}

LPSTR StringToLPSTR(const std::string& str) {
    int len;
    int slength = (int)str.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, 0, 0);
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, buf, len);
    len = WideCharToMultiByte(CP_ACP, 0, buf, len, 0, 0, 0, 0);
    char* cbuf = new char[len];
    WideCharToMultiByte(CP_ACP, 0, buf, len, cbuf, len, 0, 0);
    delete[] buf;
    return cbuf;
}

LPSTR WStringToLPSTR(const std::wstring& wstr) {
    int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr,
                                         0, nullptr, nullptr);
    std::vector<char> charVector(bufferSize);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, charVector.data(),
                        bufferSize, nullptr, nullptr);
    return charVector.data();
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

}  // namespace atom::utils

#endif
