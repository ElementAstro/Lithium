#ifndef ATOM_SYSINFO_LOCALE_HPP
#define ATOM_SYSINFO_LOCALE_HPP

#include <string>

#include "atom/macro.hpp"

namespace atom::system {
// Define a structure to hold locale information
struct LocaleInfo {
    std::string languageCode;
    std::string countryCode;
    std::string localeName;
    std::string languageDisplayName;
    std::string countryDisplayName;
    std::string currencySymbol;
    std::string decimalSymbol;
    std::string thousandSeparator;
    std::string dateFormat;
    std::string timeFormat;
    std::string characterEncoding;
} ATOM_ALIGNAS(128);

// Function to get system language info, cross-platform
auto getSystemLanguageInfo() -> LocaleInfo;

// Function to display locale information
void printLocaleInfo(const LocaleInfo& info);
}  // namespace atom::system

#endif  // ATOM_SYSINFO_LOCALE_HPP
