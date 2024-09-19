/*
 * time.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#include "time.hpp"

#include <chrono>
#include <mutex>

#ifdef _WIN32  // Windows
#include <windows.h>
#include <winreg.h>
#include <winsock2.h>
#include <ws2tcpip.h>
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
        std::lock_guard lock(mutex_);
        return std::chrono::system_clock::to_time_t(
            std::chrono::system_clock::now());
    }

#ifdef _WIN32
    void setSystemTime(int year, int month, int day, int hour, int minute,
                       int second) {
        std::lock_guard lock(mutex_);
        SYSTEMTIME sysTime;
        sysTime.wYear = year;
        sysTime.wMonth = month;
        sysTime.wDay = day;
        sysTime.wHour = hour;
        sysTime.wMinute = minute;
        sysTime.wSecond = second;
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
    }

    auto setSystemTimezone(const std::string &timezone) -> bool {
        std::lock_guard lock(mutex_);
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
        return true;
    }

    auto syncTimeFromRTC() -> bool {
        std::lock_guard lock(mutex_);
        SYSTEMTIME localTime;
        GetLocalTime(&localTime);

        TIME_ZONE_INFORMATION tzInfo;
        GetTimeZoneInformation(&tzInfo);

        SYSTEMTIME epoch = {1970, 1, 4, 1, 0, 0, 0, 0};
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

        long msOffset = static_cast<int>(localTimestamp - rtcTimestamp) * 1000L;
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
        if (!system::isRoot()) {
            LOG_F(ERROR,
                  "Permission denied. Need root privilege to set system time.");
            return false;
        }
        SetSystemTime(&localTime);
        return true;
    }

private:
    auto getTimeZoneInformationByName(const std::string &timezone,
                                      DWORD *tzId) -> bool {
        HKEY hkey;
        LPCTSTR regPath = TEXT(
            "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones\\");
        LONG ret =
            RegOpenKeyEx(HKEY_LOCAL_MACHINE, regPath, 0, KEY_READ, &hkey);
        if (ret != ERROR_SUCCESS) {
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
                            return true;
                        }
                    }
                }
                RegCloseKey(subHkey);
            }
            sizeSubKey = MAX_PATH;
        }
        RegCloseKey(hkey);
        return false;
    }
#else
public:
    void setSystemTime(int year, int month, int day, int hour, int minute,
                       int second) {
        std::lock_guard lock(mutex_);
        if (!system::isRoot()) {
            LOG_F(ERROR,
                  "Permission denied. Need root privilege to set system time.");
            return;
        }

        struct tm newTime = {0};
        newTime.tm_sec = second;
        newTime.tm_min = minute;
        newTime.tm_hour = hour;
        newTime.tm_mday = day;
        newTime.tm_mon = month - 1;
        newTime.tm_year = year - 1900;
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
    }

    auto setSystemTimezone(const std::string &timezone) -> bool {
        std::lock_guard lock(mutex_);
        struct tm newTime = {0};
        if (strptime("20200101", "%Y%m%d", &newTime) == nullptr) {
            LOG_F(ERROR, "Failed to initialize struct tm.");
            return false;
        }
        tzset();
        if (setenv("TZ", timezone.c_str(), 1) != 0) {
            LOG_F(ERROR, "Error setting time zone to {}: {}", timezone,
                  strerror(errno));
            return false;
        } else {
            tzset();
            if (strftime(nullptr, 0, "%Z", &newTime) == 0) {
                LOG_F(ERROR, "Error setting time zone to {}: {}", timezone,
                      strerror(errno));
                return false;
            }
        }
        return true;
    }

    auto syncTimeFromRTC() -> bool {
        std::lock_guard lock(mutex_);
        const char *rtcPath = "/sys/class/rtc/rtc0/time";
        struct stat rtcStat;
        if (stat(rtcPath, &rtcStat) != 0) {
            LOG_F(ERROR, "Failed to stat RTC file: {}", strerror(errno));
            return false;
        }
        if (!S_ISREG(rtcStat.st_mode)) {
            LOG_F(ERROR, "RTC path is not a regular file");
            return false;
        }
        std::ifstream rtcFile(rtcPath);
        if (!rtcFile.is_open()) {
            LOG_F(ERROR, "Failed to open RTC file: {}", strerror(errno));
            return false;
        }
        int year, month, day, hour, minute, second;
        rtcFile >> year >> month >> day >> hour >> minute >> second;
        rtcFile.close();
        struct tm rtcTm;
        rtcTm.tm_year = year - 1900;
        rtcTm.tm_mon = month - 1;
        rtcTm.tm_mday = day;
        rtcTm.tm_hour = hour;
        rtcTm.tm_min = minute;
        rtcTm.tm_sec = second;
        time_t rtcTimestamp = mktime(&rtcTm);

        struct timeval tv;
        gettimeofday(&tv, nullptr);
        tv.tv_sec = rtcTimestamp;
        tv.tv_usec = 0;
        if (settimeofday(&tv, nullptr) != 0) {
            LOG_F(ERROR, "Failed to adjust system time: {}", strerror(errno));
            return false;
        }
        return true;
    }
#endif

public:
    auto getNtpTime(const std::string &hostname) -> std::time_t {
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
        serverAddr.sin_port = htons(123);
        inet_pton(AF_INET, hostname.c_str(), &serverAddr.sin_addr);

        packetBuffer[0] = 0b11100011;
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
            LOG_F(ERROR, "Failed to send NTP request: {}", strerror(errno));
            return 0;
        }

        timeval timeout = {10, 0};
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(socketFd, &readfds);
        if (select(socketFd + 1, &readfds, nullptr, nullptr, &timeout) <= 0) {
            LOG_F(ERROR, "Failed to receive NTP response: {}", strerror(errno));
            return 0;
        }

        sockaddr_in serverResponseAddr = {};
        socklen_t addrLen = sizeof(serverResponseAddr);

        if (recvfrom(socketFd, reinterpret_cast<char *>(packetBuffer.data()),
                     NTP_PACKET_SIZE, 0,
                     reinterpret_cast<sockaddr *>(&serverResponseAddr),
                     &addrLen) < 0) {
            LOG_F(ERROR, "Failed to receive NTP response: {}", strerror(errno));
            return 0;
        }

#ifdef _WIN32
        closesocket(socketFd);
        WSACleanup();
#else
        close(socketFd);
#endif

        uint64_t timestamp = 0;
        for (int i = 40; i <= 43; i++) {
            timestamp = (timestamp << 8) | packetBuffer[i];
        }

        timestamp -= 2208988800UL;

        DLOG_F(INFO, "From NTP server: {} {}", hostname, timestamp);

        return static_cast<std::time_t>(timestamp);
    }

private:
    std::mutex mutex_;
};

// TimeManager methods

TimeManager::TimeManager() : impl_(std::make_unique<TimeManagerImpl>()) {}

TimeManager::~TimeManager() = default;

auto TimeManager::getSystemTime() -> std::time_t {
    return impl_->getSystemTime();
}

void TimeManager::setSystemTime(int year, int month, int day, int hour,
                                int minute, int second) {
    impl_->setSystemTime(year, month, day, hour, minute, second);
}

auto TimeManager::setSystemTimezone(const std::string &timezone) -> bool {
    return impl_->setSystemTimezone(timezone);
}

auto TimeManager::syncTimeFromRTC() -> bool { return impl_->syncTimeFromRTC(); }

auto TimeManager::getNtpTime(const std::string &hostname) -> std::time_t {
    return impl_->getNtpTime(hostname);
}

}  // namespace atom::web
