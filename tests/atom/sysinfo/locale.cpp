#include "atom/sysinfo/locale.hpp"
#include <gtest/gtest.h>

using namespace atom::system;

// Test getSystemLanguageInfo function
TEST(LocaleTest, GetSystemLanguageInfo) {
    LocaleInfo info = getSystemLanguageInfo();
    EXPECT_FALSE(info.languageCode.empty());
    EXPECT_FALSE(info.localeName.empty());
    EXPECT_FALSE(info.currencySymbol.empty());
    EXPECT_FALSE(info.decimalSymbol.empty());
    EXPECT_FALSE(info.dateFormat.empty());
    EXPECT_FALSE(info.timeFormat.empty());
    EXPECT_FALSE(info.characterEncoding.empty());
}

// Test printLocaleInfo function
TEST(LocaleTest, PrintLocaleInfo) {
    LocaleInfo info = getSystemLanguageInfo();
    printLocaleInfo(info);
    // Since printLocaleInfo only prints to console, we can't directly test its output.
    // However, we can ensure it doesn't crash or throw exceptions.
    SUCCEED();
}