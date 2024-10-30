/*
 * time.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#include "time.hpp"

#include <chrono>
#include <cmath>
#include <ctime>
#include <fstream>
#include <mutex>

#ifdef _WIN32  // Windows
#include <windows.h>
#include <winreg.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Iphlpapi.lib")
#else  // Linux
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#endif

#include "atom/log/loguru.hpp"
#include "atom/system/user.hpp"

namespace atom::web {

class TimeManagerImpl {
public:
    auto getSystemTime() -> std::time_t {
        LOG_F(INFO, "Entering getSystemTime");
        std::lock_guard<std::mutex> lock(mutex_);
        auto systemTime = std::chrono::system_clock::to_time_t(
            std::chrono::system_clock::now());
        LOG_F(INFO, "Exiting getSystemTime with value: {}", systemTime);
        return systemTime;
    }

#ifdef _WIN32
    void setSystemTime(int year, int month, int day, int hour, int minute,
                       int second) {
        LOG_F(INFO,
              "Entering setSystemTime with values: {}-{:02d}-{:02d} "
              "{:02d}:{:02d}:{:02d}",
              year, month, day, hour, minute, second);
        std::lock_guard<std::mutex> lock(mutex_);
        SYSTEMTIME sysTime;
        sysTime.wYear = year;
        sysTime.wMonth = month;
        sysTime.wDay = day;
        sysTime.wHour = hour;
        sysTime.wMinute = minute;
        sysTime.wSecond = second;
        sysTime.wMilliseconds = 0;
        if (SetSystemTime(&sysTime) == 0) {
            LOG_F(ERROR,
                  "Failed to set system time to {}-{:02d}-{:02d} "
                  "{:02d}:{:02d}:{:02d}. Error code: {}",
                  year, month, day, hour, minute, second, GetLastError());
        } else {
            DLOG_F(INFO,
                   "System time has been set to {}-{:02d}-{:02d} "
                   "{:02d}:{:02d}:{:02d}.",
                   year, month, day, hour, minute, second);
        }
        LOG_F(INFO, "Exiting setSystemTime");
    }

    auto setSystemTimezone(const std::string &timezone) -> bool {
        LOG_F(INFO, "Entering setSystemTimezone with timezone: {}", timezone);
        std::lock_guard<std::mutex> lock(mutex_);
        DWORD tzId;
        if (!getTimeZoneInformationByName(timezone, &tzId)) {
            LOG_F(ERROR, "Error getting time zone id for {}: {}", timezone,
                  GetLastError());
            return false;
        }
        TIME_ZONE_INFORMATION tzInfo;
        if (GetTimeZoneInformation(&tzInfo) == TIME_ZONE_ID_INVALID) {
            LOG_F(ERROR, "Error getting current time zone information: {}",
                  GetLastError());
            return false;
        }
        if (tzInfo.StandardBias != -static_cast<int>(tzId)) {
            LOG_F(ERROR,
                  "Time zone id obtained does not match offset: {} != {}", tzId,
                  -tzInfo.StandardBias);
            return false;
        }
        if (SetTimeZoneInformation(&tzInfo) == 0) {
            LOG_F(ERROR, "Error setting time zone to {}: {}", timezone,
                  GetLastError());
            return false;
        }
        LOG_F(INFO, "Exiting setSystemTimezone with success");
        return true;
    }

    auto syncTimeFromRTC() -> bool {
        LOG_F(INFO, "Entering syncTimeFromRTC");
        std::lock_guard<std::mutex> lock(mutex_);
        SYSTEMTIME localTime;
        GetLocalTime(&localTime);

        TIME_ZONE_INFORMATION tzInfo;
        GetTimeZoneInformation(&tzInfo);

        SYSTEMTIME epoch = {0};
        epoch.wYear = 1970;
        epoch.wMonth = 1;
        epoch.wDay = 1;
        epoch.wHour = 0;
        epoch.wMinute = 0;
        epoch.wSecond = 0;
        epoch.wMilliseconds = 0;
        FILETIME epochFt;
        SystemTimeToFileTime(&epoch, &epochFt);
        ULARGE_INTEGER epochUi;
        epochUi.LowPart = epochFt.dwLowDateTime;
        epochUi.HighPart = epochFt.dwHighDateTime;

        FILETIME localFt;
        SystemTimeToFileTime(&localTime, &localFt);
        ULARGE_INTEGER localUi;
        localUi.LowPart = localFt.dwLowDateTime;
        localUi.HighPart = localFt.dwHighDateTime;

        long long localTimestamp =
            (localUi.QuadPart - epochUi.QuadPart) / 10000000LL;

        time_t rtcTimestamp =
            0;  // Initialize rtcTimestamp to avoid uninitialized use

        long msOffset =
            static_cast<long>(localTimestamp - rtcTimestamp) * 1000L;
        SYSTEMTIME newTime;
        GetLocalTime(&newTime);
        FILETIME newFt;
        SystemTimeToFileTime(&newTime, &newFt);
        ULARGE_INTEGER newUi;
        newUi.LowPart = newFt.dwLowDateTime;
        newUi.HighPart = newFt.dwHighDateTime;
        long long newTimestamp = newUi.QuadPart / 10000000LL +
                                 static_cast<long long>(msOffset) / 1000LL;
        FILETIME fileTime;
        fileTime.dwLowDateTime = static_cast<DWORD>(newTimestamp);
        fileTime.dwHighDateTime = static_cast<DWORD>(newTimestamp >> 32);
        if (!atom::system::isRoot()) {
            LOG_F(ERROR,
                  "Permission denied. Need root privilege to set system time.");
            return false;
        }
        SetSystemTime(&localTime);
        LOG_F(INFO, "Exiting syncTimeFromRTC with success");
        return true;
    }

private:
    auto getTimeZoneInformationByName(const std::string &timezone,
                                      DWORD *tzId) -> bool {
        LOG_F(INFO, "Entering getTimeZoneInformationByName with timezone: {}",
              timezone);
        HKEY hkey;
        LPCTSTR regPath = TEXT(
            "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones\\");
        LONG ret =
            RegOpenKeyEx(HKEY_LOCAL_MACHINE, regPath, 0, KEY_READ, &hkey);
        if (ret != ERROR_SUCCESS) {
            LOG_F(ERROR, "Failed to open registry key: {}", ret);
            return false;
        }

        TCHAR subKey[MAX_PATH];
        TCHAR dispName[MAX_PATH];
        FILETIME ftLastWriteTime;
        DWORD index = 0;
        DWORD sizeSubKey = MAX_PATH;

        while (RegEnumKeyEx(hkey, index++, subKey, &sizeSubKey, nullptr,
                            nullptr, nullptr,
                            &ftLastWriteTime) == ERROR_SUCCESS) {
            HKEY subHkey;
            if (RegOpenKeyEx(hkey, subKey, 0, KEY_READ, &subHkey) ==
                ERROR_SUCCESS) {
                DWORD sizeDispName = MAX_PATH;
                if (RegQueryValueEx(subHkey, TEXT("Display"), nullptr, nullptr,
                                    reinterpret_cast<LPBYTE>(dispName),
                                    &sizeDispName) == ERROR_SUCCESS) {
                    if (timezone == dispName) {
                        DWORD sizeTzId = sizeof(DWORD);
                        if (RegQueryValueEx(subHkey, TEXT("TZI"), nullptr,
                                            nullptr,
                                            reinterpret_cast<LPBYTE>(tzId),
                                            &sizeTzId) == ERROR_SUCCESS) {
                            RegCloseKey(subHkey);
                            RegCloseKey(hkey);
                            LOG_F(INFO,
                                  "Exiting getTimeZoneInformationByName with "
                                  "success");
                            return true;
                        }
                    }
                }
                RegCloseKey(subHkey);
            }
            sizeSubKey = MAX_PATH;
        }
        RegCloseKey(hkey);
        LOG_F(ERROR, "Failed to find time zone information for: {}", timezone);
        return false;
    }
#else
public:
    void setSystemTime(int year, int month, int day, int hour, int minute,
                       int second) {
        LOG_F(INFO,
              "Entering setSystemTime with values: {}-{:02d}-{:02d} "
              "{:02d}:{:02d}:{:02d}",
              year, month, day, hour, minute, second);
        std::lock_guard<std::mutex> lock(mutex_);
        if (!atom::system::isRoot()) {
            LOG_F(ERROR,
                  "Permission denied. Need root privilege to set system time.");
            return;
        }

        constexpr int BASE_YEAR = 1900;
        constexpr int BASE_MONTH = 1;
        constexpr int BASE_SECOND = 0;
        struct tm newTime;
        std::memset(&newTime, 0, sizeof(newTime));
        newTime.tm_sec = second;
        newTime.tm_min = minute;
        newTime.tm_hour = hour;
        newTime.tm_mday = day;
        newTime.tm_mon = month - 1;
        newTime.tm_year = year - BASE_YEAR;
        newTime.tm_isdst = -1;

        if (std::mktime(&newTime) == -1) {
            LOG_F(ERROR,
                  "Failed to set new time to {}-{:02d}-{:02d} "
                  "{:02d}:{:02d}:{:02d}.",
                  year, month, day, hour, minute, second);
            return;
        }

        if (std::abs(std::difftime(std::time(nullptr), std::mktime(&newTime))) <
            2) {
            DLOG_F(INFO,
                   "System time has been set to {}-{:02d}-{:02d} "
                   "{:02d}:{:02d}:{:02d}.",
                   year, month, day, hour, minute, second);
        } else {
            LOG_F(ERROR,
                  "Failed to set new time to {}-{:02d}-{:02d} "
                  "{:02d}:{:02d}:{:02d}.",
                  year, month, day, hour, minute, second);
        }
        LOG_F(INFO, "Exiting setSystemTime");
    }

    auto setSystemTimezone(const std::string &timezone) -> bool {
        LOG_F(INFO, "Entering setSystemTimezone with timezone: {}", timezone);
        std::lock_guard<std::mutex> lock(mutex_);
        struct tm newTime;
        std::memset(&newTime, 0, sizeof(newTime));
        if (strptime("20200101", "%Y%m%d", &newTime) == nullptr) {
            LOG_F(ERROR, "Failed to initialize struct tm.");
            return false;
        }
        tzset();
        if (setenv("TZ", timezone.c_str(), 1) != 0) {
            LOG_F(ERROR, "Error setting time zone to {}: {}", timezone,
                  strerror_r(errno, buffer_, sizeof(buffer_)));
            return false;
        } else {
            tzset();
            // strftime(nullptr, 0, "%Z", &newTime); // 不建议传入nullptr
            char tzName[128];
            if (strftime(tzName, sizeof(tzName), "%Z", &newTime) == 0) {
                LOG_F(ERROR, "Error setting time zone to {}: {}", timezone,
                      strerror_r(errno, buffer_, sizeof(buffer_)));
                return false;
            }
        }
        LOG_F(INFO, "Exiting setSystemTimezone with success");
        return true;
    }

    auto syncTimeFromRTC() -> bool {
        LOG_F(INFO, "Entering syncTimeFromRTC");
        std::lock_guard<std::mutex> lock(mutex_);
        const char *rtcPath = "/sys/class/rtc/rtc0/time";
        struct stat rtcStat;
        if (stat(rtcPath, &rtcStat) != 0) {
            LOG_F(ERROR, "Failed to stat RTC file: {}",
                  strerror_r(errno, buffer_, sizeof(buffer_)));
            return false;
        }
        if (!S_ISREG(rtcStat.st_mode)) {
            LOG_F(ERROR, "RTC path is not a regular file");
            return false;
        }
        std::ifstream rtcFile(rtcPath);
        if (!rtcFile.is_open()) {
            LOG_F(ERROR, "Failed to open RTC file: {}",
                  strerror_r(errno, buffer_, sizeof(buffer_)));
            return false;
        }
        int year, month, day, hour, minute, second;
        rtcFile >> year >> month >> day >> hour >> minute >> second;
        rtcFile.close();
        struct tm rtcTm;
        std::memset(&rtcTm, 0, sizeof(rtcTm));
        rtcTm.tm_year = year - 1900;
        rtcTm.tm_mon = month - 1;
        rtcTm.tm_mday = day;
        rtcTm.tm_hour = hour;
        rtcTm.tm_min = minute;
        rtcTm.tm_sec = second;
        time_t rtcTimestamp = std::mktime(&rtcTm);

        struct timeval currentTime;
        gettimeofday(&currentTime, nullptr);
        currentTime.tv_sec = rtcTimestamp;
        currentTime.tv_usec = 0;
        if (settimeofday(&currentTime, nullptr) != 0) {
            LOG_F(ERROR, "Failed to adjust system time: {}",
                  strerror_r(errno, buffer_, sizeof(buffer_)));
            return false;
        }
        LOG_F(INFO, "Exiting syncTimeFromRTC with success");
        return true;
    }
#endif
    auto getNtpTime(const std::string &hostname) -> std::time_t {
        LOG_F(INFO, "Entering getNtpTime with hostname: {}", hostname);
        constexpr int NTP_PACKET_SIZE = 48;
        std::array<uint8_t, NTP_PACKET_SIZE> packetBuffer = {0};

#ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            LOG_F(ERROR, "Failed to initialize Winsock2.");
            return 0;
        }
#endif

        int socketFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (socketFd < 0) {
            LOG_F(ERROR, "Failed to create socket.");
            return 0;
        }

        sockaddr_in serverAddr = {};
        serverAddr.sin_family = AF_INET;
        constexpr uint16_t NTP_PORT = 123;
        serverAddr.sin_port = htons(NTP_PORT);
        if (inet_pton(AF_INET, hostname.c_str(), &serverAddr.sin_addr) <= 0) {
            LOG_F(ERROR, "Invalid NTP server address: {}", hostname);
#ifdef _WIN32
            closesocket(socketFd);
            WSACleanup();
#else
            close(socketFd);
#endif
            return 0;
        }

        constexpr uint8_t NTP_VERSION = 3;
        packetBuffer[0] = (3 << 3) | 3;  // LI=0, VN=3, Mode=3 (client)
        packetBuffer[1] = 0;
        packetBuffer[2] = 6;
        packetBuffer[3] = 0xEC;
        packetBuffer[12] = 49;
        packetBuffer[13] = 0x4E;
        packetBuffer[14] = 49;
        packetBuffer[15] = 52;

        if (sendto(socketFd, reinterpret_cast<char *>(packetBuffer.data()),
                   NTP_PACKET_SIZE, 0,
                   reinterpret_cast<sockaddr *>(&serverAddr),
                   sizeof(serverAddr)) < 0) {
            LOG_F(ERROR, "Failed to send NTP request: {}",
                  strerror_r(errno, buffer_, sizeof(buffer_)));
#ifdef _WIN32
            closesocket(socketFd);
            WSACleanup();
#else
            close(socketFd);
#endif
            return 0;
        }

        timeval timeout = {10, 0};
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(socketFd, &readfds);
        int selectResult =
            select(socketFd + 1, &readfds, nullptr, nullptr, &timeout);
        if (selectResult <= 0) {
            LOG_F(ERROR, "Failed to receive NTP response: {}",
                  strerror_r(errno, buffer_, sizeof(buffer_)));
#ifdef _WIN32
            closesocket(socketFd);
            WSACleanup();
#else
            close(socketFd);
#endif
            return 0;
        }

        sockaddr_in serverResponseAddr = {};
        socklen_t addrLen = sizeof(serverResponseAddr);

        if (recvfrom(socketFd, reinterpret_cast<char *>(packetBuffer.data()),
                     NTP_PACKET_SIZE, 0,
                     reinterpret_cast<sockaddr *>(&serverResponseAddr),
                     &addrLen) < 0) {
            LOG_F(ERROR, "Failed to receive NTP response: {}",
                  strerror_r(errno, buffer_, sizeof(buffer_)));
#ifdef _WIN32
            closesocket(socketFd);
            WSACleanup();
#else
            close(socketFd);
#endif
            return 0;
        }

#ifdef _WIN32
        closesocket(socketFd);
        WSACleanup();
#else
        close(socketFd);
#endif

        uint64_t timestamp = 0;
        constexpr int NTP_TIMESTAMP_START = 40;
        constexpr int NTP_TIMESTAMP_END = 43;
        constexpr size_t TIMESTAMP_BITS = 8;
        for (int i = NTP_TIMESTAMP_START; i <= NTP_TIMESTAMP_END; i++) {
            timestamp = (timestamp << TIMESTAMP_BITS) | packetBuffer[i];
        }

        constexpr uint32_t NTP_DELTA = 2208988800UL;
        timestamp -= NTP_DELTA;

        DLOG_F(INFO, "From NTP server: {} {}", hostname, timestamp);

        LOG_F(INFO, "Exiting getNtpTime with value: {}", timestamp);
        return static_cast<std::time_t>(timestamp);
    }

private:
    std::mutex mutex_;
    char buffer_[256] = {0};
};

}  // namespace atom::web

// TimeManager methods

namespace atom::web {

TimeManager::TimeManager() : impl_(std::make_unique<TimeManagerImpl>()) {
    LOG_F(INFO, "TimeManager constructor called");
}

TimeManager::~TimeManager() { LOG_F(INFO, "TimeManager destructor called"); }

auto TimeManager::getSystemTime() -> std::time_t {
    LOG_F(INFO, "TimeManager::getSystemTime called");
    auto systemTime = impl_->getSystemTime();
    LOG_F(INFO, "TimeManager::getSystemTime returning: {}", systemTime);
    return systemTime;
}

void TimeManager::setSystemTime(int year, int month, int day, int hour,
                                int minute, int second) {
    LOG_F(INFO,
          "TimeManager::setSystemTime called with values: {}-{:02d}-{:02d} "
          "{:02d}:{:02d}:{:02d}",
          year, month, day, hour, minute, second);
    impl_->setSystemTime(year, month, day, hour, minute, second);
    LOG_F(INFO, "TimeManager::setSystemTime completed");
}

auto TimeManager::setSystemTimezone(const std::string &timezone) -> bool {
    LOG_F(INFO, "TimeManager::setSystemTimezone called with timezone: {}",
          timezone);
    auto result = impl_->setSystemTimezone(timezone);
    LOG_F(INFO, "TimeManager::setSystemTimezone returning: {}", result);
    return result;
}

auto TimeManager::syncTimeFromRTC() -> bool {
    LOG_F(INFO, "TimeManager::syncTimeFromRTC called");
    auto result = impl_->syncTimeFromRTC();
    LOG_F(INFO, "TimeManager::syncTimeFromRTC returning: {}", result);
    return result;
}

auto TimeManager::getNtpTime(const std::string &hostname) -> std::time_t {
    LOG_F(INFO, "TimeManager::getNtpTime called with hostname: {}", hostname);
    auto ntpTime = impl_->getNtpTime(hostname);
    LOG_F(INFO, "TimeManager::getNtpTime returning: {}", ntpTime);
    return ntpTime;
}

}  // namespace atom::web