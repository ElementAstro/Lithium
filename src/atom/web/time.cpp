/*
 * time.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#include "time.hpp"

#include <chrono>
#include <fstream>
#include <iostream>
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
        if (!SetSystemTime(&sysTime)) {
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

    bool setSystemTimezone(const std::string &timezone) {
        std::lock_guard lock(mutex_);
        DWORD tz_id;
        if (!getTimeZoneInformationByName(timezone, &tz_id)) {
            LOG_F(ERROR, "Error getting time zone id for {}: {}", timezone,
                  GetLastError());
            return false;
        }
        TIME_ZONE_INFORMATION tz_info;
        if (!GetTimeZoneInformation(&tz_info)) {
            LOG_F(ERROR, "Error getting current time zone information: {}",
                  GetLastError());
            return false;
        }
        if (tz_info.StandardBias != -static_cast<int>(tz_id)) {
            LOG_F(ERROR,
                  "Time zone id obtained does not match offset: {} != {}",
                  tz_id, -tz_info.StandardBias);
            return false;
        }
        if (!SetTimeZoneInformation(&tz_info)) {
            LOG_F(ERROR, "Error setting time zone to {}: {}", timezone,
                  GetLastError());
            return false;
        }
        return true;
    }

    bool syncTimeFromRTC() {
        std::lock_guard lock(mutex_);
        time_t now = time(nullptr);
        SYSTEMTIME local_time;
        GetLocalTime(&local_time);

        TIME_ZONE_INFORMATION tz_info;
        GetTimeZoneInformation(&tz_info);
        long utc_diff_ms = -tz_info.Bias * 60 * 1000;

        SYSTEMTIME epoch = {1970, 1, 4, 1, 0, 0, 0, 0};
        FILETIME epoch_ft, local_ft;
        SystemTimeToFileTime(&epoch, &epoch_ft);
        SystemTimeToFileTime(&local_time, &local_ft);
        ULARGE_INTEGER epoch_ui, local_ui;
        epoch_ui.LowPart = epoch_ft.dwLowDateTime;
        epoch_ui.HighPart = epoch_ft.dwHighDateTime;
        local_ui.LowPart = local_ft.dwLowDateTime;
        local_ui.HighPart = local_ft.dwHighDateTime;
        long long local_timestamp =
            (local_ui.QuadPart - epoch_ui.QuadPart) / 10000000LL;

        time_t rtc_timestamp;
        // RTC读取代码...

        long ms_offset =
            static_cast<int>(local_timestamp - rtc_timestamp) * 1000L;
        SYSTEMTIME new_time;
        GetLocalTime(&new_time);
        FILETIME new_ft;
        SystemTimeToFileTime(&new_time, &new_ft);
        ULARGE_INTEGER new_ui;
        new_ui.LowPart = new_ft.dwLowDateTime;
        new_ui.HighPart = new_ft.dwHighDateTime;
        long long new_timestamp = new_ui.QuadPart / 10000000LL +
                                  static_cast<long long>(ms_offset) / 1000LL;
        FILETIME ft;
        ft.dwLowDateTime = static_cast<DWORD>(new_timestamp);
        ft.dwHighDateTime = static_cast<DWORD>(new_timestamp >> 32);
        if (!system::isRoot()) {
            LOG_F(ERROR,
                  "Permission denied. Need root privilege to set system time.");
            return false;
        }
        SetSystemTime(&local_time);
        return true;
    }

private:
    bool getTimeZoneInformationByName(const std::string &timezone,
                                      DWORD *tz_id) {
        HKEY hkey;
        LPCTSTR reg_path = TEXT(
            "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones\\");
        LONG ret =
            RegOpenKeyEx(HKEY_LOCAL_MACHINE, reg_path, 0, KEY_READ, &hkey);
        if (ret != ERROR_SUCCESS) {
            return false;
        }

        TCHAR sub_key[MAX_PATH], disp_name[MAX_PATH];
        FILETIME ft_last_write_time;
        DWORD index = 0;
        DWORD size_sub_key = MAX_PATH;

        while (RegEnumKeyEx(hkey, index++, sub_key, &size_sub_key, NULL, NULL,
                            NULL, &ft_last_write_time) == ERROR_SUCCESS) {
            HKEY sub_hkey;
            if (RegOpenKeyEx(hkey, sub_key, 0, KEY_READ, &sub_hkey) ==
                ERROR_SUCCESS) {
                DWORD size_disp_name = MAX_PATH;
                if (RegQueryValueEx(sub_hkey, TEXT("Display"), NULL, NULL,
                                    reinterpret_cast<LPBYTE>(disp_name),
                                    &size_disp_name) == ERROR_SUCCESS) {
                    if (timezone == disp_name) {
                        DWORD size_tz_id = sizeof(DWORD);
                        if (RegQueryValueEx(sub_hkey, TEXT("TZI"), NULL, NULL,
                                            reinterpret_cast<LPBYTE>(tz_id),
                                            &size_tz_id) == ERROR_SUCCESS) {
                            RegCloseKey(sub_hkey);
                            RegCloseKey(hkey);
                            return true;
                        }
                    }
                }
                RegCloseKey(sub_hkey);
            }
            size_sub_key = MAX_PATH;
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
        std::time_t now = std::time(nullptr);
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

        if (std::abs(std::difftime(now, std::mktime(&newTime))) < 2) {
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
        if (strptime("20200101", "%Y%m%d", &newTime) == NULL) {
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

    bool syncTimeFromRTC() {
        std::lock_guard lock(mutex_);
        time_t now = time(nullptr);
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

        long long localTimestamp = static_cast<long long>(now) * 1000000LL;
        long usOffset = static_cast<int>(
            (localTimestamp - rtcTimestamp * 1000000LL) / 1000LL);

        struct timeval tv;
        gettimeofday(&tv, nullptr);
        tv.tv_sec += usOffset / 1000000;
        tv.tv_usec += usOffset % 1000000;
        if (tv.tv_usec >= 1000000) {
            tv.tv_sec += 1;
            tv.tv_usec -= 1000000;
        }
        if (tv.tv_sec < now - 60 || tv.tv_sec > now + 60) {
            LOG_F(ERROR, "RTC time is too far away from current time");
            return false;
        }
        if (settimeofday(&tv, nullptr) != 0) {
            LOG_F(ERROR, "Failed to adjust system time: {}", strerror(errno));
            return false;
        }
        return true;
    }
#endif

    std::time_t getNtpTime(const std::string &hostname) {
        constexpr int NTP_PACKET_SIZE = 48;
        uint8_t packetBuffer[NTP_PACKET_SIZE] = {0};

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

        sockaddr_in serverAddr = {0};
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

        if (sendto(socketFd, reinterpret_cast<char *>(packetBuffer),
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

        sockaddr_in serverResponseAddr = {0};
        socklen_t addrLen = sizeof(serverResponseAddr);

        if (recvfrom(socketFd, reinterpret_cast<char *>(packetBuffer),
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
        for (int i = 40; i <= 43; i++)
            timestamp = (timestamp << 8) | packetBuffer[i];

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

auto TimeManager::getSystemTime() -> std::time_t { return impl_->getSystemTime(); }

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
