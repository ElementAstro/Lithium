#include "locale.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <langinfo.h>
#endif

#ifdef ATOM_ENABLE_DEBUG
#include <iostream>
#endif

#include "atom/log/loguru.hpp"

namespace atom::system {
#ifdef _WIN32
// Windows-specific helper function to convert wstring to string
auto wstringToString(const std::wstring& wstr) -> std::string {
    LOG_F(INFO, "Converting wstring to string");
    return std::string(wstr.begin(), wstr.end());
}

// Function to get locale info on Windows
std::string getLocaleInfo(LCTYPE type) {
    LOG_F(INFO, "Getting locale info for type: %d", type);
    WCHAR buffer[LOCALE_NAME_MAX_LENGTH];
    int result = GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, type, buffer,
                                 LOCALE_NAME_MAX_LENGTH);
    if (result != 0) {
        LOG_F(INFO, "Successfully retrieved locale info");
        return wstringToString(buffer);
    }
    LOG_F(WARNING, "Failed to retrieve locale info");
    return "Unknown";
}
#endif

// Function to get system language info, cross-platform
LocaleInfo getSystemLanguageInfo() {
    LOG_F(INFO, "Starting getSystemLanguageInfo function");
    LocaleInfo localeInfo;

#ifdef _WIN32
    // On Windows, use GetLocaleInfoEx to retrieve locale info
    LOG_F(INFO, "Retrieving locale info on Windows");
    localeInfo.languageCode = getLocaleInfo(LOCALE_SISO639LANGNAME);
    localeInfo.countryCode = getLocaleInfo(LOCALE_SISO3166CTRYNAME);
    localeInfo.localeName = getLocaleInfo(LOCALE_SNAME);
    localeInfo.languageDisplayName = getLocaleInfo(LOCALE_SNATIVELANGNAME);
    localeInfo.countryDisplayName = getLocaleInfo(LOCALE_SNATIVECTRYNAME);
    localeInfo.currencySymbol = getLocaleInfo(LOCALE_SCURRENCY);
    localeInfo.decimalSymbol = getLocaleInfo(LOCALE_SDECIMAL);
    localeInfo.thousandSeparator = getLocaleInfo(LOCALE_STHOUSAND);
    localeInfo.dateFormat = getLocaleInfo(LOCALE_SSHORTDATE);
    localeInfo.timeFormat = getLocaleInfo(LOCALE_STIMEFORMAT);
    localeInfo.characterEncoding = getLocaleInfo(LOCALE_IDEFAULTANSICODEPAGE);
#else
    // On Unix-like systems, use setlocale and nl_langinfo
    LOG_F(INFO, "Retrieving locale info on Unix-like system");
    std::setlocale(LC_ALL, "");
    localeInfo.languageCode =
        std::string(nl_langinfo(CODESET));  // Language code as encoding
    localeInfo.countryCode = "N/A";  // No direct equivalent for country code
    localeInfo.localeName =
        std::string(setlocale(LC_ALL, NULL));  // Full locale name
    localeInfo.languageDisplayName = "N/A";    // Not directly available
    localeInfo.countryDisplayName = "N/A";     // Not directly available
    localeInfo.currencySymbol = std::string(nl_langinfo(CRNCYSTR));
    localeInfo.decimalSymbol = std::string(nl_langinfo(RADIXCHAR));
    localeInfo.thousandSeparator =
        "N/A";  // Not directly available from nl_langinfo
    localeInfo.dateFormat = std::string(nl_langinfo(D_FMT));  // Date format
    localeInfo.timeFormat = std::string(nl_langinfo(T_FMT));  // Time format
    localeInfo.characterEncoding =
        std::string(nl_langinfo(CODESET));  // Character encoding
#endif

    LOG_F(INFO, "Finished getSystemLanguageInfo function");
    return localeInfo;
}

// Function to display locale information
void printLocaleInfo([[maybe_unused]] const LocaleInfo& info) {
#if ATOM_ENABLE_DEBUG
    LOG_F(INFO, "Printing locale information");
    std::cout << "Language code (ISO 639): " << info.languageCode << "\n";
    std::cout << "Country code (ISO 3166): " << info.countryCode << "\n";
    std::cout << "Full locale name: " << info.localeName << "\n";
    std::cout << "Language display name: " << info.languageDisplayName << "\n";
    std::cout << "Country display name: " << info.countryDisplayName << "\n";
    std::cout << "Currency symbol: " << info.currencySymbol << "\n";
    std::cout << "Decimal symbol: " << info.decimalSymbol << "\n";
    std::cout << "Thousand separator: " << info.thousandSeparator << "\n";
    std::cout << "Date format: " << info.dateFormat << "\n";
    std::cout << "Time format: " << info.timeFormat << "\n";
    std::cout << "Character encoding: " << info.characterEncoding << "\n";
#endif
}

}  // namespace atom::system
