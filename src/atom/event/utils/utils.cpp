/*
 * utils.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-10

Description: Utils

*************************************************/

#include "utils.hpp"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef ATOM_OS_WIN
#include <MSWSock.h>
#include <Ws2tcpip.h>
#include <windows.h>
#else
#include <arpa/inet.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#if defined(ATOM_OS_LINUX) || defined(ATOM_OS_ANDROID)
#include <sys/prctl.h>
#endif
#if defined(ATOM_OS_MAC) || defined(ATOM_OS_IOS)
#include <mach-o/dyld.h>
#include <pthread.h>
#include "CoreFoundation/CoreFoundation.h"
#ifndef ATOM_OS_IOS
#include <libproc.h>
#endif
#endif
#endif

#include <limits.h>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>

#include "kmtrace.hpp"

using namespace std::chrono;

ATOM_NS_BEGIN

enum { KM_RESOLVE_IPV0 = 0, KM_RESOLVE_IPV4 = 1, KM_RESOLVE_IPV6 = 2 };

#if 0
int km_resolve_2_ip_v4(const char *host_name, char *ip_buf, size_t ip_buf_len)
{
    const char* ptr = host_name;
    bool is_digit = true;
    while(*ptr) {
        if(*ptr != ' ' &&  *ptr != '\t') {
            if(*ptr != '.' && !(*ptr >= '0' && *ptr <= '9')) {
                is_digit = false;
                break;
            }
        }
        ++ptr;
    }
    
    if (is_digit) {
        strncpy_s(ip_buf, ip_buf_len, host_name, strlen(host_name));
        return 0;
    }
    
    struct hostent* he = nullptr;
#ifdef ATOM_OS_LINUX
    int nError = 0;
    char szBuffer[1024] = {0};
    struct hostent *pheResultBuf = reinterpret_cast<struct hostent *>(szBuffer);
    
    if (::gethostbyname_r(
                          host_name,
                          pheResultBuf,
                          szBuffer + sizeof(struct hostent),
                          sizeof(szBuffer) - sizeof(struct hostent),
                          &pheResultBuf,
                          &nError) == 0)
    {
        he = pheResultBuf;
    }
#else
    he = gethostbyname(host_name);
#endif
    
    if(he && he->h_addr_list && he->h_addr_list[0])
    {
#ifndef ATOM_OS_WIN
        inet_ntop(AF_INET, he->h_addr_list[0], ip_buf, ip_buf_len);
        return 0;
#else
        char* tmp = (char*)inet_ntoa((in_addr&)(*he->h_addr_list[0]));
        if(tmp) {
            strncpy_s(ip_buf, ip_buf_len, tmp, strlen(tmp));
            return 0;
        }
#endif
    }
    
    ip_buf[0] = 0;
    return -1;
}
#endif

int km_resolve_2_ip(const char *host_name, char *ip_buf, size_t ip_buf_len,
                    int ipv) {
    if (!host_name || !ip_buf) {
        return -1;
    }

    ip_buf[0] = '\0';

    addrinfo *ai = nullptr;
    struct addrinfo hints = {0};
    if (KM_RESOLVE_IPV6 == ipv) {
        hints.ai_family = AF_INET6;
    } else if (KM_RESOLVE_IPV4 == ipv) {
        hints.ai_family = AF_INET;
    } else {
        hints.ai_family = AF_UNSPEC;
    }
    hints.ai_flags = AI_ADDRCONFIG;  // will block 10 seconds in some case if
                                     // not set AI_ADDRCONFIG
    if (getaddrinfo(host_name, nullptr, &hints, &ai) != 0 || !ai) {
        return -1;
    }
#ifdef ATOM_OS_WIN
    auto ip_buf_size = static_cast<DWORD>(ip_buf_len);
#else
    auto ip_buf_size = ip_buf_len;
#endif
    for (addrinfo *aii = ai; aii; aii = aii->ai_next) {
        if (AF_INET6 == aii->ai_family &&
            (KM_RESOLVE_IPV6 == ipv || KM_RESOLVE_IPV0 == ipv)) {
            sockaddr_in6 *sa6 = (sockaddr_in6 *)aii->ai_addr;
            if (IN6_IS_ADDR_LINKLOCAL(&(sa6->sin6_addr)))
                continue;
            if (IN6_IS_ADDR_SITELOCAL(&(sa6->sin6_addr)))
                continue;
            if (getnameinfo(aii->ai_addr,
                            static_cast<socklen_t>(aii->ai_addrlen), ip_buf,
                            ip_buf_size, NULL, 0,
                            NI_NUMERICHOST | NI_NUMERICSERV) != 0)
                continue;
            else
                break;  // found a ipv6 address
        } else if (AF_INET == aii->ai_family &&
                   (KM_RESOLVE_IPV4 == ipv || KM_RESOLVE_IPV0 == ipv)) {
            if (getnameinfo(aii->ai_addr,
                            static_cast<socklen_t>(aii->ai_addrlen), ip_buf,
                            ip_buf_size, NULL, 0,
                            NI_NUMERICHOST | NI_NUMERICSERV) != 0)
                continue;
            else
                break;  // found a ipv4 address
        }
    }
    if ('\0' == ip_buf[0] && KM_RESOLVE_IPV0 == ipv &&
        getnameinfo(ai->ai_addr, static_cast<socklen_t>(ai->ai_addrlen), ip_buf,
                    ip_buf_size, NULL, 0,
                    NI_NUMERICHOST | NI_NUMERICSERV) != 0) {
        freeaddrinfo(ai);
        return -1;
    }
    freeaddrinfo(ai);
    return 0;
}

int km_set_sock_addr(const char *addr, unsigned short port, addrinfo *hints,
                     sockaddr *sk_addr, size_t sk_addr_len) {
    char service[128] = {0};
    struct addrinfo *ai = nullptr;
    if (!addr && hints) {
        hints->ai_flags |= AI_PASSIVE;
    }
    snprintf(service, sizeof(service) - 1, "%d", port);
    auto ret = getaddrinfo(addr, service, hints, &ai);
    if (ret != 0 || !ai) {
        if (ai)
            freeaddrinfo(ai);
        return ret;
    }
    if (ai->ai_addrlen > sk_addr_len) {
        if (ai)
            freeaddrinfo(ai);
        return -1;
    }
    if (sk_addr) {
        memcpy(sk_addr, ai->ai_addr, ai->ai_addrlen);
    }
    freeaddrinfo(ai);
    return 0;
}

int km_get_sock_addr(const sockaddr *sk_addr, size_t sk_addr_len, char *addr,
                     size_t addr_len, unsigned short *port) {
#ifdef ATOM_OS_WIN
    auto addr_size = static_cast<DWORD>(addr_len);
#else
    auto addr_size = addr_len;
#endif
    char service[16] = {0};
    if (getnameinfo(sk_addr, static_cast<socklen_t>(sk_addr_len), addr,
                    addr_size, service, sizeof(service),
                    NI_NUMERICHOST | NI_NUMERICSERV) != 0)
        return -1;
    if (port)
        *port = atoi(service);
    return 0;
}

int km_get_sock_addr(const sockaddr *addr, size_t addr_len, std::string &ip,
                     uint16_t *port) {
    char ip_buf[128] = {0};
    if (km_get_sock_addr(addr, addr_len, ip_buf, sizeof(ip_buf), port) != 0) {
        return -1;
    }
    ip = ip_buf;
    return 0;
}

int km_get_sock_addr(const sockaddr_storage &addr, std::string &ip,
                     uint16_t *port) {
    char ip_buf[128] = {0};
    auto addr_len = km_get_addr_length(addr);
    if (km_get_sock_addr((const sockaddr *)&addr, addr_len, ip_buf,
                         sizeof(ip_buf), port) != 0) {
        return -1;
    }
    ip = ip_buf;
    return 0;
}

int km_set_addr_port(uint16_t port, sockaddr_storage &addr) {
    if (AF_INET == addr.ss_family) {
        sockaddr_in *p = (sockaddr_in *)&addr;
        p->sin_port = htons(port);
    } else if (AF_INET6 == addr.ss_family) {
        sockaddr_in6 *p = (sockaddr_in6 *)&addr;
        p->sin6_port = htons(port);
    } else {
        return -1;
    }
    return 0;
}

size_t km_get_addr_length(const sockaddr_storage &addr) {
    size_t addr_len = sizeof(addr);
    if (AF_INET == addr.ss_family) {
        addr_len = sizeof(sockaddr_in);
    } else if (AF_INET6 == addr.ss_family) {
        addr_len = sizeof(sockaddr_in6);
    }
    return addr_len;
}

bool km_is_ipv6_address(const char *addr) {
    sockaddr_storage ss_addr = {0};
    struct addrinfo hints = {0};
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_NUMERICHOST;
    if (km_set_sock_addr(addr, 0, &hints, (struct sockaddr *)&ss_addr,
                         sizeof(ss_addr)) != 0) {
        return false;
    }
    return AF_INET6 == ss_addr.ss_family;
}

bool km_is_ip_address(const char *addr) {
    sockaddr_storage ss_addr = {0};
    struct addrinfo hints = {0};
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_NUMERICHOST;
    return km_set_sock_addr(addr, 0, &hints, (struct sockaddr *)&ss_addr,
                            sizeof(ss_addr)) == 0;
}

bool km_is_mcast_address(const char *addr) {
    sockaddr_storage ss_addr = {0};
    struct addrinfo hints = {0};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags =
        AI_NUMERICHOST | AI_ADDRCONFIG;  // will block 10 seconds in some case
                                         // if not set AI_ADDRCONFIG
    km_set_sock_addr(addr, 0, &hints, (struct sockaddr *)&ss_addr,
                     sizeof(ss_addr));
    switch (ss_addr.ss_family) {
        case AF_INET: {
            struct sockaddr_in *sa_in4 = (struct sockaddr_in *)&ss_addr;
            return IN_MULTICAST(ntohl(sa_in4->sin_addr.s_addr));
        }
        case AF_INET6: {
            struct sockaddr_in6 *sa_in6 = (struct sockaddr_in6 *)&ss_addr;
            return IN6_IS_ADDR_MULTICAST(&sa_in6->sin6_addr) ? true : false;
        }
    }
    return false;
}

int km_parse_address(const char *addr, char *proto, size_t proto_len,
                     char *host, size_t host_len, unsigned short *port) {
    if (!addr || !host)
        return -1;

    const char *tmp = strstr(addr, "://");
    if (tmp) {
        if (proto) {
            auto tmp_len =
                (std::min)(proto_len - 1, static_cast<size_t>(tmp - addr));
            memcpy(proto, addr, tmp_len);
            proto[tmp_len] = '\0';
        }
        tmp += 3;
    } else {
        if (proto)
            proto[0] = '\0';
        tmp = addr;
    }
    const char *end = strchr(tmp, '/');
    if (!end) {
        end = addr + strlen(addr);
    }

    const char *tmp1 = strchr(tmp, '[');
    if (tmp1) {  // ipv6 address
        tmp = tmp1 + 1;
        tmp1 = strchr(tmp, ']');
        if (!tmp1)
            return -1;
        auto tmp_len =
            (std::min)(host_len - 1, static_cast<size_t>(tmp1 - tmp));
        memcpy(host, tmp, tmp_len);
        host[tmp_len] = '\0';
        tmp = tmp1 + 1;
        tmp1 = strchr(tmp, ':');
        if (tmp1 && tmp1 <= end)
            tmp = tmp1 + 1;
        else
            tmp = nullptr;
    } else {  // ipv4 address
        tmp1 = strchr(tmp, ':');
        if (tmp1 && tmp1 <= end) {
            auto tmp_len =
                (std::min)(host_len - 1, static_cast<size_t>(tmp1 - tmp));
            memcpy(host, tmp, tmp_len);
            host[tmp_len] = '\0';
            tmp = tmp1 + 1;
        } else {
            auto tmp_len =
                (std::min)(host_len - 1, static_cast<size_t>(end - tmp));
            memcpy(host, tmp, tmp_len);
            host[tmp_len] = '\0';
            tmp = nullptr;
        }
    }

    if (port) {
        *port = tmp ? atoi(tmp) : 0;
    }

    return 0;
}

int set_nonblocking(SOCKET_FD fd) {
#ifdef ATOM_OS_WIN
    int mode = 1;
    ::ioctlsocket(fd, FIONBIO, (ULONG *)&mode);
#else
    int flag = ::fcntl(fd, F_GETFL, 0);
    ::fcntl(fd, F_SETFL, flag | O_NONBLOCK | O_ASYNC);
#endif
    return 0;
}

int set_tcpnodelay(SOCKET_FD fd) {
    int opt_val = 1;
    return ::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&opt_val,
                        sizeof(int));
}

int find_first_set(uint32_t b) {
    if (0 == b) {
        return -1;
    }
    int n = 0;
    if (!(0xffff & b))
        n += 16;
    if (!((0xff << n) & b))
        n += 8;
    if (!((0xf << n) & b))
        n += 4;
    if (!((0x3 << n) & b))
        n += 2;
    if (!((0x1 << n) & b))
        n += 1;
    return n;
}

int find_first_set(uint64_t b) {
    if (0 == b) {
        return -1;
    }
    int n = 0;
    if (!(0xffffffff & b))
        n += 32;
    if (!((0xffffLL << n) & b))
        n += 16;
    if (!((0xffLL << n) & b))
        n += 8;
    if (!((0xfLL << n) & b))
        n += 4;
    if (!((0x3LL << n) & b))
        n += 2;
    if (!((0x1LL << n) & b))
        n += 1;
    return n;
}

TICK_COUNT_TYPE get_tick_count_ms() {
    using namespace std::chrono;
    steady_clock::time_point _now = steady_clock::now();
    milliseconds _now_ms = duration_cast<milliseconds>(_now.time_since_epoch());
    return (TICK_COUNT_TYPE)_now_ms.count();
}

TICK_COUNT_TYPE calc_time_elapse_delta_ms(TICK_COUNT_TYPE now_tick,
                                          TICK_COUNT_TYPE &start_tick) {
    if (now_tick - start_tick > (((TICK_COUNT_TYPE)-1) >> 1)) {
        start_tick = now_tick;
        return 0;
    }
    return now_tick - start_tick;
}

#if 0
// need c++ 14
bool is_equal(const std::string &s1, const std::string &s2)
{
    return std::equal(s1.begin(), s1.end(), s2.begin(), s2.end(),
        [] (const char &ch1, const char &ch2) {
            return std::toupper(ch1) == std::toupper(ch2);
    });
}
#endif

bool is_equal(const char *str1, const char *str2) {
    return strcasecmp(str1, str2) == 0;
}

bool is_equal(const std::string &str1, const std::string &str2) {
    return str1.length() == str2.length() &&
           strcasecmp(str1.c_str(), str2.c_str()) == 0;
}

bool is_equal(const char *str1, const std::string &str2) {
    return strcasecmp(str1, str2.c_str()) == 0;
}

bool is_equal(const std::string &str1, const char *str2) {
    return strcasecmp(str1.c_str(), str2) == 0;
}

bool is_equal(const char *str1, const char *str2, int n) {
    return strncasecmp(str1, str2, n) == 0;
}

bool is_equal(const std::string &str1, const std::string &str2, int n) {
    return strncasecmp(str1.c_str(), str2.c_str(), n) == 0;
}

bool is_equal(const char *str1, const std::string &str2, int n) {
    return strncasecmp(str1, str2.c_str(), n) == 0;
}

bool is_equal(const std::string &str1, const char *str2, int n) {
    return strncasecmp(str1.c_str(), str2, n) == 0;
}

char *trim_left(char *str, char c) {
    while (*str && *str++ == c) {
        ;
    }

    return str;
}

char *trim_right(char *str, char c) {
    return trim_right(str, str + strlen(str), c);
}

char *trim_right(char *str, char *str_end, char c) {
    while (--str_end >= str && *str_end == c) {
        ;
    }
    *(++str_end) = 0;

    return str;
}

std::string &trim_left(std::string &str, char c) {
    str.erase(0, str.find_first_not_of(c));
    return str;
}

std::string &trim_right(std::string &str, char c) {
    auto pos = str.find_last_not_of(c);
    if (pos != std::string::npos) {
        str.erase(pos + 1);
    }
    return str;
}

bool contains_token(const std::string &str, const std::string &token,
                    char delim) {
    bool found = false;
    for_each_token(str, delim, [&found, &token](std::string &t) {
        if (is_equal(t, token)) {
            found = true;
            return false;
        }
        return true;
    });

    return found;
}

bool remove_token(std::string &tokens, const std::string &token, char delim) {
    bool removed = false;
    std::string str;
    for_each_token(tokens, delim, [&removed, &token, &str](std::string &t) {
        if (is_equal(t, token)) {
            removed = true;
        } else {
            if (!str.empty()) {
                str += ", ";
            }
            str += t;
        }
        return true;
    });
    tokens = std::move(str);

    return removed;
}

size_t random_bytes(void *buf, size_t len) {
    using rand_type = unsigned int;
    using bytes_randomizer =
        std::independent_bits_engine<std::default_random_engine,
                                     sizeof(rand_type) * 8, rand_type>;

    auto sl = len / sizeof(rand_type);
    auto rl = len - sl * sizeof(rand_type);
    auto *p = static_cast<rand_type *>(buf);
    bytes_randomizer br(std::random_device{}());
    if (sl > 0) {
        std::generate(p, p + sl, std::ref(br));
    }
    if (rl > 0) {
        rand_type r = br();
        memcpy(static_cast<char *>(buf) + len - rl, &r, rl);
    }
    return len;
}

std::string getExecutablePath() {
    std::string str_path;
#ifdef ATOM_OS_WIN
    char c_path[MAX_PATH] = {0};
    GetModuleFileNameA(NULL, c_path, sizeof(c_path));
    str_path = c_path;
#elif defined(ATOM_OS_MAC)
#ifndef ATOM_OS_IOS
    char c_path[PROC_PIDPATHINFO_MAXSIZE];
    if (proc_pidpath(getpid(), c_path, sizeof(c_path)) <= 0) {
        return "./";
    }
    str_path = c_path;
#else
    char c_path[PATH_MAX] = {0};
    uint32_t size = sizeof(c_path);
    CFBundleRef cf_bundle = CFBundleGetMainBundle();
    if (cf_bundle) {
        CFURLRef cf_url = CFBundleCopyBundleURL(cf_bundle);
        if (CFURLGetFileSystemRepresentation(cf_url, TRUE, (UInt8 *)c_path,
                                             PATH_MAX)) {
            CFStringRef cf_str =
                CFURLCopyFileSystemPath(cf_url, kCFURLPOSIXPathStyle);
            CFStringGetCString(cf_str, c_path, PATH_MAX,
                               kCFStringEncodingASCII);
            CFRelease(cf_str);
        }
        CFRelease(cf_url);
        str_path = c_path;
        if (str_path.at(str_path.length() - 1) != PATH_SEPARATOR) {
            str_path += PATH_SEPARATOR;
        }
        return str_path;
    } else {
        _NSGetExecutablePath(c_path, &size);
    }
    str_path = c_path;
#endif
#elif defined(ATOM_OS_LINUX)
    char c_path[1024] = {0};
    if (readlink("/proc/self/exe", c_path, sizeof(c_path)) < 0) {
        return "./";
    }
    str_path = c_path;
#else
    return "./";
#endif
    if (str_path.empty()) {
        return "./";
    }
    auto pos = str_path.rfind(PATH_SEPARATOR, str_path.size());
    if (pos != std::string::npos) {
        str_path.resize(pos);
    }
    str_path.append(1, PATH_SEPARATOR);
    return str_path;
}

std::string getModuleFullPath(const void *addr_in_module) {
    if (!addr_in_module) {
        return "";
    }
    std::string str_path;
#ifdef ATOM_OS_WIN
    HMODULE hmodule = 0;
    auto flags = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                 GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;
    auto ret = ::GetModuleHandleEx(flags, (LPCTSTR)addr_in_module, &hmodule);
    if (!ret) {
        return "";
    }
    char file_name[2048] = {0};
    auto count = ::GetModuleFileNameA(hmodule, file_name, ARRAYSIZE(file_name));
    if (count == 0) {
        return "";
    }
    str_path = file_name;
#elif defined(ATOM_OS_MAC) || defined(ATOM_OS_LINUX)
    Dl_info dl_info;
    dladdr((void *)addr_in_module, &dl_info);

    str_path = dl_info.dli_fname;
#else
    return "";
#endif

#ifdef ATOM_OS_MAC
    auto pos1 = str_path.rfind(PATH_SEPARATOR);
    auto pos2 = pos1;
    int count = 4;
#ifdef ATOM_OS_IOS
    count = 2;
#endif
    while (pos2 != std::string::npos && pos2 > 0 && --count > 0) {
        pos1 = pos2;
        pos2 = str_path.rfind(PATH_SEPARATOR, pos1 - 1);
    }
    if (pos2 != std::string::npos) {
        auto name = str_path.substr(pos2 + 1, pos1 - pos2 - 1);
        auto pos3 = name.rfind('.');
        if (pos3 != std::string::npos) {
            auto ext = name.substr(pos3 + 1);
            if (ext == "framework" || ext == "bundle" || ext == "app") {
                str_path.erase(pos1);
            }
        }
    }
#endif

    return str_path;
}

std::string getCurrentModulePath() {
    std::string str_path = getModuleFullPath((void *)getCurrentModulePath);
    auto pos = str_path.rfind(PATH_SEPARATOR, str_path.size());
    str_path.resize(pos);
    return str_path;
}

std::string toString(const system_clock::time_point &time, bool utc) {
    auto msecs =
        duration_cast<milliseconds>(time.time_since_epoch()).count() % 1000;
    auto itt = system_clock::to_time_t(time);
    struct tm res;
#ifdef ATOM_OS_WIN
    utc ? gmtime_s(&res, &itt) : localtime_s(&res, &itt);  // windows and C11
#else
    utc ? gmtime_r(&itt, &res) : localtime_r(&itt, &res);
#endif
    std::ostringstream ss;
    ss << std::put_time(&res, "%FT%T.") << std::setfill('0') << std::setw(3)
       << msecs;
    if (utc) {
        ss << 'Z';
    } else {
        ss << std::put_time(&res, "%z");
    }
    return ss.str();
}

std::string getDateTimeString(bool utc) {
    auto now = system_clock::now();
    return toString(now, utc);
}

#if defined(ATOM_OS_WIN)
std::string utf8_encode(const wchar_t *wstr, int len) {
    if (!wstr || !len)
        return std::string();
    auto utf8_len =
        WideCharToMultiByte(CP_UTF8, 0, wstr, len, NULL, 0, NULL, NULL);
    if (utf8_len <= 0) {
        return "";
    }
    std::string utf8_str(utf8_len, 0);
    utf8_len = WideCharToMultiByte(CP_UTF8, 0, wstr, len, &utf8_str[0],
                                   utf8_len, NULL, NULL);
    if (utf8_len < 0) {
        return "";
    }
    return utf8_str;
}

std::string utf8_encode(const wchar_t *wstr) {
    auto utf8_len =
        WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    if (utf8_len <= 1) {  // include the terminating null character
        return "";
    }
    std::string utf8_str(utf8_len, 0);
    utf8_len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &utf8_str[0], utf8_len,
                                   NULL, NULL);
    --utf8_len;  // exclude the terminating null character
    if (utf8_len < 0) {
        return "";
    }
    utf8_str.resize(utf8_len);
    return utf8_str;
}
// Convert a wide Unicode string to an UTF8 string
std::string utf8_encode(const std::wstring &wstr) {
    if (wstr.empty())
        return std::string();
    int size_needed = WideCharToMultiByte(
        CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0],
                        size_needed, NULL, NULL);
    return strTo;
}

// Convert an UTF8 string to a wide Unicode String
std::wstring utf8_decode(const std::string &str) {
    if (str.empty())
        return std::wstring();
    int size_needed =
        MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0],
                        size_needed);
    return wstrTo;
}

void setCurrentThreadNameX(const char *name) {
    struct {
        DWORD dwType;
        LPCSTR szName;
        DWORD dwThreadID;
        DWORD dwFlags;
    } threadname_info = {0x1000, name, static_cast<DWORD>(-1), 0};

    try {
        ::RaiseException(0x406D1388, 0, sizeof(threadname_info) / sizeof(DWORD),
                         reinterpret_cast<ULONG_PTR *>(&threadname_info));
    } catch (...) {
    }
}
#endif  // ATOM_OS_WIN

void setCurrentThreadName(const char *name) {
#if defined(ATOM_OS_WIN)
#ifndef __WINRT__
    using PFN_SetThreadDescription = HRESULT(WINAPI *)(HANDLE, PCWSTR);
    static PFN_SetThreadDescription pfn_SetThreadDescription = nullptr;
    static HMODULE kernel32 = nullptr;

    if (!kernel32) {
        kernel32 = LoadLibraryW(L"kernel32.dll");
        if (kernel32) {
            pfn_SetThreadDescription = (PFN_SetThreadDescription)GetProcAddress(
                kernel32, "SetThreadDescription");
        }
    }

    if (pfn_SetThreadDescription != nullptr) {
        auto wstr = utf8_decode(name);
        if (!wstr.empty()) {
            auto hr =
                pfn_SetThreadDescription(::GetCurrentThread(), wstr.c_str());
            if (SUCCEEDED(hr)) {
                return;
            }
        }
    }
#endif

    setCurrentThreadNameX(name);
#elif defined(ATOM_OS_LINUX) || defined(ATOM_OS_ANDROID)
    prctl(PR_SET_NAME, reinterpret_cast<unsigned long>(name));
#elif defined(ATOM_OS_MAC) || defined(ATOM_OS_IOS)
    pthread_setname_np(name);
#endif
}

ATOM_NS_END

#ifdef ATOM_OS_WIN
ATOM_NS_BEGIN
LPFN_CONNECTEX connect_ex = nullptr;
LPFN_ACCEPTEX accept_ex = nullptr;
LPFN_CANCELIOEX cancel_io_ex = nullptr;
ATOM_NS_END

ATOM_NS_USING

void kev_init() {
    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(1, 1);
    int nResult = WSAStartup(wVersionRequested, &wsaData);
    if (nResult != 0) {
        return;
    }

    auto sock = socket(AF_INET, SOCK_STREAM, 0);
    GUID guid = WSAID_CONNECTEX;
    DWORD bytes = 0;
    if (::WSAIoctl(sock, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid,
                   sizeof(guid), &connect_ex, sizeof(connect_ex), &bytes, 0,
                   0) != 0) {
        connect_ex = nullptr;
    }

    guid = WSAID_ACCEPTEX;
    bytes = 0;
    if (::WSAIoctl(sock, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid,
                   sizeof(guid), &accept_ex, sizeof(accept_ex), &bytes, 0,
                   0) != 0) {
        accept_ex = nullptr;
    }
    closesocket(sock);
    cancel_io_ex = (LPFN_CANCELIOEX)GetProcAddress(GetModuleHandle(L"KERNEL32"),
                                                   "CancelIoEx");
}

void kev_fini() { WSACleanup(); }

class KMInitializer {
    static KMInitializer s_singleton;

    KMInitializer() { kev_init(); }

public:
    ~KMInitializer() { kev_fini(); }
};
KMInitializer KMInitializer::s_singleton;

#pragma comment(lib, "ws2_32.lib")

#endif  // ATOM_OS_WIN
