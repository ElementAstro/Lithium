/*
 * utils.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-17

Description: Network Utils

**************************************************/

#ifndef ATOM_WEB_UTILS_HPP
#define ATOM_WEB_UTILS_HPP

#include <string>

#if defined(__linux__) || defined(__APPLE__)
#include <arpa/inet.h>
#include <netdb.h>
#endif

namespace atom::web {

/**
 * @brief Check if a port is in use.
 * 检查端口是否正在使用。
 *
 * This function checks if a port is in use by attempting to bind a socket to
 * the port. If the socket can be bound, the port is not in use.
 * 该函数通过尝试将套接字绑定到端口来检查端口是否正在使用。如果套接字可以绑定，则端口未被使用。
 *
 * @param port The port number to check. 要检查的端口号。
 * @return `true` if the port is in use, `false` otherwise.
 * 如果端口正在使用，则返回`true`，否则返回`false`。
 *
 * @code
 * if (atom::web::isPortInUse(8080)) {
 *     std::cout << "Port 8080 is in use." << std::endl;
 * } else {
 *     std::cout << "Port 8080 is available." << std::endl;
 * }
 * @endcode
 */
auto isPortInUse(int port) -> bool;

/**
 * @brief Check if there is any program running on the specified port and kill
 * it if found. 检查指定端口上是否有程序正在运行，如果找到则终止该程序。
 *
 * This function checks if there is any program running on the specified port by
 * querying the system. If a program is found, it will be terminated.
 * 该函数通过查询系统检查指定端口上是否有程序正在运行。如果找到程序，将终止它。
 *
 * @param port The port number to check. 要检查的端口号。
 * @return `true` if a program was found and terminated, `false` otherwise.
 * 如果找到并终止了程序，则返回`true`；否则返回`false`。
 *
 * @code
 * if (atom::web::checkAndKillProgramOnPort(8080)) {
 *     std::cout << "Program on port 8080 was terminated." << std::endl;
 * } else {
 *     std::cout << "No program running on port 8080." << std::endl;
 * }
 * @endcode
 */
auto checkAndKillProgramOnPort(int port) -> bool;

#if defined(__linux__) || defined(__APPLE__)
/**
 * @brief Dump address information from source to destination.
 * 将地址信息从源转储到目标。
 *
 * This function copies address information from the source to the destination.
 * 该函数将地址信息从源复制到目标。
 *
 * @param dst Destination address information. 目标地址信息。
 * @param src Source address information. 源地址信息。
 * @return `0` on success, `-1` on failure. 成功返回`0`，失败返回`-1`。
 *
 * @code
 * struct addrinfo* src = ...;
 * struct addrinfo* dst = nullptr;
 * if (atom::web::dumpAddrInfo(&dst, src) == 0) {
 *     std::cout << "Address information dumped successfully." << std::endl;
 * } else {
 *     std::cout << "Failed to dump address information." << std::endl;
 * }
 * @endcode
 */
auto dumpAddrInfo(struct addrinfo** dst, struct addrinfo* src) -> int;

/**
 * @brief Convert address information to string.
 * 将地址信息转换为字符串。
 *
 * This function converts address information to a string representation.
 * 该函数将地址信息转换为字符串表示。
 *
 * @param addrInfo Address information. 地址信息。
 * @param jsonFormat If `true`, output in JSON format.
 * 如果为`true`，则以JSON格式输出。
 * @return String representation of address information. 地址信息的字符串表示。
 *
 * @code
 * struct addrinfo* addrInfo = ...;
 * std::string addrStr = atom::web::addrInfoToString(addrInfo, true);
 * std::cout << addrStr << std::endl;
 * @endcode
 */
auto addrInfoToString(struct addrinfo* addrInfo,
                      bool jsonFormat = false) -> std::string;

/**
 * @brief Get address information for a given hostname and service.
 * 获取给定主机名和服务的地址信息。
 *
 * This function retrieves address information for a given hostname and service.
 * 该函数检索给定主机名和服务的地址信息。
 *
 * @param hostname The hostname to resolve. 要解析的主机名。
 * @param service The service to resolve. 要解析的服务。
 * @return Pointer to the address information. 地址信息的指针。
 *
 * @code
 * struct addrinfo* addrInfo = atom::web::getAddrInfo("www.google.com", "http");
 * if (addrInfo) {
 *     std::cout << "Address information retrieved successfully." << std::endl;
 *     atom::web::freeAddrInfo(addrInfo);
 * } else {
 *     std::cout << "Failed to retrieve address information." << std::endl;
 * }
 * @endcode
 */
auto getAddrInfo(const std::string& hostname,
                 const std::string& service) -> struct addrinfo*;

/**
 * @brief Free address information.
 * 释放地址信息。
 *
 * This function frees the memory allocated for address information.
 * 该函数释放为地址信息分配的内存。
 *
 * @param addrInfo Pointer to the address information to free.
 * 要释放的地址信息的指针。
 *
 * @code
 * struct addrinfo* addrInfo = ...;
 * atom::web::freeAddrInfo(addrInfo);
 * @endcode
 */
void freeAddrInfo(struct addrinfo* addrInfo);

/**
 * @brief Compare two address information structures.
 * 比较两个地址信息结构。
 *
 * This function compares two address information structures for equality.
 * 该函数比较两个地址信息结构是否相等。
 *
 * @param addrInfo1 First address information structure. 第一个地址信息结构。
 * @param addrInfo2 Second address information structure. 第二个地址信息结构。
 * @return `true` if the structures are equal, `false` otherwise.
 * 如果结构相等，则返回`true`，否则返回`false`。
 *
 * @code
 * struct addrinfo* addrInfo1 = ...;
 * struct addrinfo* addrInfo2 = ...;
 * if (atom::web::compareAddrInfo(addrInfo1, addrInfo2)) {
 *     std::cout << "Address information structures are equal." << std::endl;
 * } else {
 *     std::cout << "Address information structures are not equal." <<
 * std::endl;
 * }
 * @endcode
 */
auto compareAddrInfo(const struct addrinfo* addrInfo1,
                     const struct addrinfo* addrInfo2) -> bool;

/**
 * @brief Filter address information by family.
 * 按家庭过滤地址信息。
 *
 * This function filters address information by the specified family.
 * 该函数按指定的家庭过滤地址信息。
 *
 * @param addrInfo Address information to filter. 要过滤的地址信息。
 * @param family The family to filter by (e.g., AF_INET).
 * 要过滤的家庭（例如，AF_INET）。
 * @return Filtered address information. 过滤后的地址信息。
 *
 * @code
 * struct addrinfo* addrInfo = ...;
 * struct addrinfo* filtered = atom::web::filterAddrInfo(addrInfo, AF_INET);
 * if (filtered) {
 *     std::cout << "Filtered address information retrieved successfully." <<
 * std::endl; atom::web::freeAddrInfo(filtered); } else { std::cout << "No
 * address information matched the filter." << std::endl;
 * }
 * @endcode
 */
auto filterAddrInfo(struct addrinfo* addrInfo, int family) -> struct addrinfo*;

/**
 * @brief Sort address information by family.
 * 按家庭排序地址信息。
 *
 * This function sorts address information by family.
 * 该函数按家庭排序地址信息。
 *
 * @param addrInfo Address information to sort. 要排序的地址信息。
 * @return Sorted address information. 排序后的地址信息。
 *
 * @code
 * struct addrinfo* addrInfo = ...;
 * struct addrinfo* sorted = atom::web::sortAddrInfo(addrInfo);
 * if (sorted) {
 *     std::cout << "Sorted address information retrieved successfully." <<
 * std::endl; atom::web::freeAddrInfo(sorted); } else { std::cout << "Failed to
 * sort address information." << std::endl;
 * }
 * @endcode
 */
auto sortAddrInfo(struct addrinfo* addrInfo) -> struct addrinfo*;
#endif

}  // namespace atom::web

#endif  // ATOM_WEB_UTILS_HPP
