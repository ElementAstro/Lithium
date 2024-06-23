#include "atom/utils/convert.hpp"
#include <gtest/gtest.h>

using namespace atom::utils;

#ifdef _WIN32

TEST(ConvertUtilsTest, CharToLPWSTR) {
    std::string_view charString = "Hello, World!";
    LPWSTR lpwstr = CharToLPWSTR(charString);
    std::wstring expected = L"Hello, World!";
    EXPECT_EQ(lpwstr, expected);
    // Free the allocated memory
    delete[] lpwstr;
}

TEST(ConvertUtilsTest, WCharArrayToString) {
    const WCHAR wCharArray[] = L"Hello, World!";
    std::string result = WCharArrayToString(wCharArray);
    std::string expected = "Hello, World!";
    EXPECT_EQ(result, expected);
}

TEST(ConvertUtilsTest, StringToLPSTR) {
    std::string str = "Hello, World!";
    LPSTR lpstr = StringToLPSTR(str);
    std::string result = lpstr;
    EXPECT_EQ(result, str);
    // Free the allocated memory
    delete[] lpstr;
}

TEST(ConvertUtilsTest, WStringToLPSTR) {
    std::wstring wstr = L"Hello, World!";
    LPSTR lpstr = WStringToLPSTR(wstr);
    std::string result = lpstr;
    std::string expected = "Hello, World!";
    EXPECT_EQ(result, expected);
    // Free the allocated memory
    delete[] lpstr;
}

TEST(ConvertUtilsTest, StringToLPWSTR) {
    std::string str = "Hello, World!";
    LPWSTR lpwstr = StringToLPWSTR(str);
    std::wstring expected = L"Hello, World!";
    EXPECT_EQ(lpwstr, expected);
    // Free the allocated memory
    delete[] lpwstr;
}

TEST(ConvertUtilsTest, LPWSTRToString) {
    LPWSTR lpwstr = L"Hello, World!";
    std::string result = LPWSTRToString(lpwstr);
    std::string expected = "Hello, World!";
    EXPECT_EQ(result, expected);
}

TEST(ConvertUtilsTest, LPCWSTRToString) {
    LPCWSTR lpcwstr = L"Hello, World!";
    std::string result = LPCWSTRToString(lpcwstr);
    std::string expected = "Hello, World!";
    EXPECT_EQ(result, expected);
}

TEST(ConvertUtilsTest, WStringToLPWSTR) {
    std::wstring wstr = L"Hello, World!";
    LPWSTR lpwstr = WStringToLPWSTR(wstr);
    std::wstring expected = L"Hello, World!";
    EXPECT_EQ(lpwstr, expected);
    // Free the allocated memory
    delete[] lpwstr;
}

TEST(ConvertUtilsTest, LPWSTRToWString) {
    LPWSTR lpwstr = L"Hello, World!";
    std::wstring result = LPWSTRToWString(lpwstr);
    std::wstring expected = L"Hello, World!";
    EXPECT_EQ(result, expected);
}

TEST(ConvertUtilsTest, LPCWSTRToWString) {
    LPCWSTR lpcwstr = L"Hello, World!";
    std::wstring result = LPCWSTRToWString(lpcwstr);
    std::wstring expected = L"Hello, World!";
    EXPECT_EQ(result, expected);
}

#endif  // _WIN32