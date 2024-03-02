/*
 * utils.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-10

Description: Utils

*************************************************/

#ifndef ATOM_EVENT_UITLS_HPP
#define ATOM_EVENT_UITLS_HPP

#include "../kevdefs.hpp"

#ifdef ATOM_OS_WIN
#include <Ws2tcpip.h>
#else  // ATOM_OS_WIN
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#if defined(ATOM_OS_LINUX) && !defined(ATOM_OS_ANDROID)
#include <sys/syscall.h>
#endif
#endif  // ATOM_OS_WIN

#include <algorithm>
#include <chrono>
#include <memory>
#include <sstream>
#include <string>

struct addrinfo;
struct sockaddr;
struct sockaddr_storage;

ATOM_NS_BEGIN

#ifdef ATOM_OS_WIN
#define snprintf(d, dl, fmt, ...) \
    _snprintf_s(d, dl, _TRUNCATE, fmt, ##__VA_ARGS__)
#define vsnprintf(d, dl, fmt, ...) \
    _vsnprintf_s(d, dl, _TRUNCATE, fmt, ##__VA_ARGS__)
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#define getCurrentThreadId() GetCurrentThreadId()
using LPFN_CANCELIOEX = BOOL(WINAPI *)(HANDLE, LPOVERLAPPED);
#elif defined(ATOM_OS_MAC)
#define getCurrentThreadId() pthread_mach_thread_np(pthread_self())
#elif defined(ATOM_OS_ANDROID)
#define getCurrentThreadId() gettid()
#elif defined(ATOM_OS_LINUX)
#define getCurrentThreadId() syscall(__NR_gettid)
#else
#define getCurrentThreadId() pthread_self()
#endif

#ifdef ATOM_OS_WIN
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#define strncpy_s(d, dl, s, c)                   \
    do {                                         \
        if (0 == dl)                             \
            break;                               \
        size_t sz = (dl - 1) > c ? c : (dl - 1); \
        strncpy(d, s, sz);                       \
        d[sz] = '\0';                            \
    } while (0)
#endif

#ifndef TICK_COUNT_TYPE
#define TICK_COUNT_TYPE uint64_t
#endif

#define UNUSED(x) (void)(x)

#if __cplusplus > 201402L
// c++17
#define FALLTHROUGH [[fallthrough]];
#else
#define FALLTHROUGH
#endif

template <typename T, size_t N>
char (&ArraySizeHelper(const T (&array)[N]))[N];
#define ARRAY_SIZE(array) (sizeof(Atom::Event::ArraySizeHelper(array)))

int set_nonblocking(SOCKET_FD fd);
int set_tcpnodelay(SOCKET_FD fd);
int find_first_set(uint32_t b);
int find_first_set(uint64_t b);
TICK_COUNT_TYPE get_tick_count_ms();
TICK_COUNT_TYPE calc_time_elapse_delta_ms(TICK_COUNT_TYPE now_tick,
                                          TICK_COUNT_TYPE &start_tick);

bool is_equal(const char *str1, const char *str2);
bool is_equal(const std::string &str1, const std::string &str2);
bool is_equal(const char *str1, const std::string &str2);
bool is_equal(const std::string &str1, const char *str2);
bool is_equal(const char *str1, const char *str2, int n);
bool is_equal(const std::string &str1, const std::string &str2, int n);
bool is_equal(const char *str1, const std::string &str2, int n);
bool is_equal(const std::string &str1, const char *str2, int n);
char *trim_left(char *str, char c);
char *trim_right(char *str, char c);
char *trim_right(char *str, char *str_end, char c);
std::string &trim_left(std::string &str, char c);
std::string &trim_right(std::string &str, char c);

std::string getExecutablePath();
std::string getModuleFullPath(const void *addr_in_module);
std::string getCurrentModulePath();

template <typename LAMBDA>  // (std::string &token) -> bool
void for_each_token(const std::string &tokens, char delim, LAMBDA &&func) {
    std::stringstream ss;
    ss.str(tokens);
    std::string token;
    while (std::getline(ss, token, delim)) {
        trim_left(token, ' ');
        trim_right(token, ' ');
        if (!func(token)) {
            break;
        }
    }
}
bool contains_token(const std::string &tokens, const std::string &token,
                    char delim);
bool remove_token(std::string &tokens, const std::string &token, char delim);

size_t random_bytes(void *buf, size_t len);

int km_resolve_2_ip(const char *host_name, char *ip_buf, size_t ip_buf_len,
                    int ipv = 0);
int km_parse_address(const char *addr, char *proto, size_t proto_len,
                     char *host, size_t host_len, unsigned short *port);
int km_set_sock_addr(const char *addr, unsigned short port, addrinfo *hints,
                     sockaddr *sk_addr, size_t sk_addr_len);
int km_get_sock_addr(const sockaddr *sk_addr, size_t sk_addr_len, char *addr,
                     size_t addr_len, unsigned short *port);
bool km_is_ipv6_address(const char *addr);
bool km_is_ip_address(const char *addr);
bool km_is_mcast_address(const char *addr);

int km_get_sock_addr(const sockaddr *addr, size_t addr_len, std::string &ip,
                     uint16_t *port);
int km_get_sock_addr(const sockaddr_storage &addr, std::string &ip,
                     uint16_t *port);
int km_set_addr_port(uint16_t port, sockaddr_storage &addr);
size_t km_get_addr_length(const sockaddr_storage &addr);

inline bool km_is_fatal_error(Result err) {
    return err != Result::OK && err != Result::AGAIN;
}

std::string toString(const std::chrono::system_clock::time_point &time,
                     bool utc);
std::string getDateTimeString(bool utc);
void setCurrentThreadName(const char *name);

ATOM_NS_END

#endif  // ATOM_EVENT_UITLS_HPP
