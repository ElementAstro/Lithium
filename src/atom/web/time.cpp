/*
 * time.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-31

Description: Time

**************************************************/

#include "time.hpp"

#include <time.h>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>

#ifdef _WIN32  // Windows
#include <winsock2.h>
#include <windows.h>
#include <winreg.h>
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

#include "atom/system/system.hpp"

#include "atom/log/loguru.hpp"

namespace atom::web {
std::time_t getSystemTime() {
    return std::chrono::system_clock::to_time_t(
        std::chrono::system_clock::now());
}

#ifdef _WIN32
void setSystemTime(int year, int month, int day, int hour, int minute,
                   int second) {
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

bool GetTimeZoneInformationByName(const std::string &timezone, DWORD *tz_id);

bool setSystemTimezone(const std::string &timezone) {
    bool success = true;
    DWORD tz_id;
    if (!GetTimeZoneInformationByName(timezone, &tz_id)) {
        LOG_F(ERROR, "Error getting time zone id for {}: {}", timezone,
              GetLastError());
        success = false;
    }
    TIME_ZONE_INFORMATION tz_info;
    if (!GetTimeZoneInformation(&tz_info)) {
        LOG_F(ERROR, "Error getting current time zone information: {}",
              GetLastError());
        success = false;
    } else if (tz_info.StandardBias != -static_cast<int>(tz_id)) {
        LOG_F(ERROR, "Time zone id obtained does not match offset: {} != {}",
              tz_id, -tz_info.StandardBias);
        success = false;
    }
    if (!SetTimeZoneInformation(&tz_info)) {
        LOG_F(ERROR, "Error setting time zone to {}: {}", timezone,
              GetLastError());
        success = false;
    }
    return success;
}

bool GetTimeZoneInformationByName(const std::string &timezone, DWORD *tz_id) {
    HKEY hkey;
    LPCTSTR reg_path =
        TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones\\");

    LONG ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, reg_path, 0, KEY_READ, &hkey);
    if (ret != ERROR_SUCCESS) {
        return false;
    }

    TCHAR sub_key[MAX_PATH], disp_name[MAX_PATH];
    FILETIME ft_last_write_time;
    DWORD index = 0;
    DWORD size_sub_key = MAX_PATH;

    while (RegEnumKeyEx(hkey, index++, sub_key, &size_sub_key, NULL, NULL, NULL,
                        &ft_last_write_time) == ERROR_SUCCESS) {
        HKEY sub_hkey;
        if (RegOpenKeyEx(hkey, sub_key, 0, KEY_READ, &sub_hkey) ==
            ERROR_SUCCESS) {
            DWORD size_disp_name = MAX_PATH;
            if (RegQueryValueEx(sub_hkey, TEXT("Display"), NULL, NULL,
                                reinterpret_cast<LPBYTE>(disp_name),
                                &size_disp_name) == ERROR_SUCCESS) {
                if (timezone.compare(disp_name) == 0) {
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

bool syncTimeFromRTC() {
    // 获取当前时间戳（单位：秒）
    time_t now = time(nullptr);
    // 获取Windows下的本地时间
    SYSTEMTIME local_time;
    GetLocalTime(&local_time);

    // 计算距离UTC时间的时差（单位：毫秒）
    TIME_ZONE_INFORMATION tz_info;
    GetTimeZoneInformation(&tz_info);
    long utc_diff_ms = -tz_info.Bias * 60 * 1000;

    // 计算本地时间距离1970年1月1日0时0分0秒（UTC时间）的秒数
    SYSTEMTIME epoch;
    epoch.wYear = 1970;
    epoch.wMonth = 1;
    epoch.wDayOfWeek = 4;  // 1970年1月1日是星期四
    epoch.wDay = 1;
    epoch.wHour = 0;
    epoch.wMinute = 0;
    epoch.wSecond = 0;
    epoch.wMilliseconds = 0;
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

    // 计算当前RTC时间距离1970年1月1日0时0分0秒（UTC时间）的秒数
    // 这里需要根据具体硬件RTC模块的接口来实现读取RTC时间的功能
    time_t rtc_timestamp;

    // 计算RTC时间距离本地时间的毫秒偏差
    long ms_offset = (int)(local_timestamp - rtc_timestamp) * 1000L;

    // 调用Windows API函数来调整系统时间
    SYSTEMTIME new_time;
    GetLocalTime(&new_time);
    FILETIME new_ft;
    SystemTimeToFileTime(&new_time, &new_ft);
    ULARGE_INTEGER new_ui;
    new_ui.LowPart = new_ft.dwLowDateTime;
    new_ui.HighPart = new_ft.dwHighDateTime;
    long long new_timestamp =
        new_ui.QuadPart / 10000000LL + (long long)ms_offset / 1000LL;
    FILETIME ft;
    ft.dwLowDateTime = (DWORD)new_timestamp;
    ft.dwHighDateTime = (DWORD)(new_timestamp >> 32);
    if (!system::isRoot()) {
        LOG_F(ERROR,
              "Permission denied. Need root privilege to set system time.");
        return false;
    }
    SetSystemTime(&local_time);  // 此处需要管理员权限才能调用成功
    return true;
}

#else

void setSystemTime(int year, int month, int day, int hour, int minute,
                   int second) {
    if (!System::isRoot()) {
        LOG_F(ERROR,
              "Permission denied. Need root privilege to set system time.");
        return;
    }

    struct tm new_time;
    std::time_t now = std::time(nullptr);
    new_time.tm_sec = second;
    new_time.tm_min = minute;
    new_time.tm_hour = hour;
    new_time.tm_mday = day;
    new_time.tm_mon = month - 1;
    new_time.tm_year = year - 1900;
    new_time.tm_isdst = -1;

    if (std::mktime(&new_time) == -1) {
        LOG_F(
            ERROR,
            "Failed to set new time to {}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}.",
            year, month, day, hour, minute, second);
        return;
    }

    if (std::abs(std::difftime(now, std::mktime(&new_time))) < 2) {
        DLOG_F(INFO,
               "System time has been set to {}-{:02d}-{:02d} "
               "{:02d}:{:02d}:{:02d}.",
               year, month, day, hour, minute, second);
    } else {
        LOG_F(
            ERROR,
            "Failed to set new time to {}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}.",
            year, month, day, hour, minute, second);
    }
}

bool setSystemTimezone(const std::string &timezone) {
    bool success = true;
    struct tm new_time;
    memset(&new_time, 0, sizeof(new_time));
    if (strptime("20200101", "%Y%m%d", &new_time) == NULL) {
        LOG_F(ERROR, "Failed to initialize struct tm.");
        success = false;
    }
    tzset();
    if (setenv("TZ", timezone.c_str(), 1) != 0) {
        LOG_F(ERROR, "Error setting time zone to {}: {}", timezone,
              strerror(errno));
        success = false;
    } else {
        tzset();
        if (strftime(NULL, 0, "%Z", &new_time) == 0) {
            LOG_F(ERROR, "Error setting time zone to {}: {}", timezone,
                  strerror(errno));
            success = false;
        }
    }
    return success;
}

bool syncTimeFromRTC() {
    // 获取当前时间戳（单位：秒）
    time_t now = time(nullptr);
    // Linux平台使用sysfs接口读取RTC时间
    const char *rtc_path = "/sys/class/rtc/rtc0/time";
    struct stat rtc_stat;
    if (stat(rtc_path, &rtc_stat) != 0) {
        LOG_F(ERROR, "Failed to stat RTC file: {}", strerror(errno));
        return false;
    }
    if (!S_ISREG(rtc_stat.st_mode)) {
        LOG_F(ERROR, "RTC path is not a regular file");
        return false;
    }
    std::ifstream rtc_file(rtc_path);
    if (!rtc_file.is_open()) {
        LOG_F(ERROR, "Failed to open RTC file: {}", strerror(errno));
        return false;
    }
    int year, month, day, hour, minute, second;
    rtc_file >> year >> month >> day >> hour >> minute >> second;
    rtc_file.close();
    struct tm rtc_tm;
    rtc_tm.tm_year = year - 1900;
    rtc_tm.tm_mon = month - 1;
    rtc_tm.tm_mday = day;
    rtc_tm.tm_hour = hour;
    rtc_tm.tm_min = minute;
    rtc_tm.tm_sec = second;
    time_t rtc_timestamp = mktime(&rtc_tm);

    // 计算当前UTC时间距离1970年1月1日0时0分0秒的微秒数
    long long local_timestamp = (long long)now * 1000000LL;

    // 计算RTC时间距离当前时间的微秒偏差
    long us_offset = (int)(local_timestamp - rtc_timestamp * 1000000LL);

    // 调用Linux系统调用函数来调整系统时间
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    tv.tv_sec += us_offset / 1000000;
    tv.tv_usec += us_offset % 1000000;
    if (tv.tv_usec >= 1000000) {
        tv.tv_sec += 1;
        tv.tv_usec -= 1000000;
    }
    if (tv.tv_sec < now - 60 || tv.tv_sec > now + 60) {
        LOG_F(ERROR, "RTC time is too far away from current time");
        return false;  // 如果调整后的时间与当前时间相差超过1分钟，则认为调整失败
    }
    if (settimeofday(&tv, nullptr) != 0) {
        LOG_F(ERROR, "Failed to adjust system time: {}", strerror(errno));
        return false;
    }

    return true;
}

#endif

time_t getNtpTime(const char *hostname) {
    constexpr int NTP_PACKET_SIZE = 48;
    uint8_t packetBuffer[NTP_PACKET_SIZE] = {0};

#ifdef _WIN32
    // 在 Windows 上使用 Winsock2 库
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        LOG_F(ERROR, "Failed to initialize Winsock2.");
        return 0;
    }
#endif

    // 创建 UDP socket，并连接到 NTP 服务器的端口上
    int socketFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socketFd < 0) {
        LOG_F(ERROR, "Failed to create socket.");
        return 0;
    }

    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(123);  // NTP 端口
    inet_pton(AF_INET, hostname, &serverAddr.sin_addr);

    // 组装 NTP 请求数据包
    packetBuffer[0] = 0b11100011;  // LI, Version, Mode
    packetBuffer[1] = 0;           // Stratum, or type of clock
    packetBuffer[2] = 6;           // Polling Interval
    packetBuffer[3] = 0xEC;        // Peer Clock Precision
    packetBuffer[12] = 49;
    packetBuffer[13] = 0x4E;
    packetBuffer[14] = 49;
    packetBuffer[15] = 52;

    // 发送 NTP 请求数据包到服务器
    if (sendto(socketFd, (char *)packetBuffer, NTP_PACKET_SIZE, 0,
               (sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        LOG_F(ERROR, "Failed to send NTP request: {}", strerror(errno));
        return 0;
    }

    // 接收 NTP 响应数据包，并提取时间
    timeval timeout{10, 0};  // 10s 的超时时间
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(socketFd, &readfds);
    if (select(socketFd + 1, &readfds, nullptr, nullptr, &timeout) <= 0) {
        LOG_F(ERROR, "Failed to receive NTP response: {}", strerror(errno));
        return 0;
    }

    sockaddr_in serverResponseAddr;
    socklen_t addrLen = sizeof(serverResponseAddr);
    memset(&serverResponseAddr, 0, addrLen);

    if (recvfrom(socketFd, (char *)packetBuffer, NTP_PACKET_SIZE, 0,
                 (sockaddr *)&serverResponseAddr, &addrLen) < 0) {
        LOG_F(ERROR, "Failed to receive NTP response: {}", strerror(errno));
        return 0;
    }

    // 关闭 socket
#ifdef _WIN32
    closesocket(socketFd);
    WSACleanup();
#else
    close(socketFd);
#endif

    // 从 NTP 响应数据包中提取时间戳字段（64 bits）
    uint64_t timestamp = 0;
    for (int i = 40; i <= 43; i++)
        timestamp = (timestamp << 8) | packetBuffer[i];

    timestamp -=
        2208988800UL;  // 将从 1900 年至今的秒数转换为从 1970 年至今的秒数

    DLOG_F(INFO, "From NTP server: {} {}", hostname, timestamp);

    return (time_t)timestamp;
}
}  // namespace atom::web
