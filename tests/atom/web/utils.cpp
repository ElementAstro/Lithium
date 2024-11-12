#include <gtest/gtest.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>

#include "atom/web/utils.hpp"

using namespace atom::web;

// Test isPortInUse function
TEST(UtilsTest, IsPortInUse) {
    int port = 8080;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    // Bind the socket to the port
    bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));

    // Check if the port is in use
    EXPECT_TRUE(isPortInUse(port));

    // Close the socket
    close(sockfd);

    // Check if the port is not in use
    EXPECT_FALSE(isPortInUse(port));
}

// Test checkAndKillProgramOnPort function
TEST(UtilsTest, CheckAndKillProgramOnPort) {
    int port = 8080;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    // Bind the socket to the port
    bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));

    // Check and kill the program on the port
    EXPECT_TRUE(checkAndKillProgramOnPort(port));

    // Close the socket
    close(sockfd);

    // Check if no program is running on the port
    EXPECT_FALSE(checkAndKillProgramOnPort(port));
}

#if defined(__linux__) || defined(__APPLE__)
// Test dumpAddrInfo function
TEST(UtilsTest, DumpAddrInfo) {
    struct addrinfo src;
    struct addrinfo* dst = nullptr;

    src.ai_family = AF_INET;
    src.ai_socktype = SOCK_STREAM;
    src.ai_protocol = IPPROTO_TCP;
    src.ai_addrlen = sizeof(struct sockaddr_in);
    src.ai_addr = (struct sockaddr*)malloc(sizeof(struct sockaddr_in));
    src.ai_canonname = nullptr;
    src.ai_next = nullptr;

    EXPECT_EQ(dumpAddrInfo(&dst, &src), 0);

    free(src.ai_addr);
    freeAddrInfo(dst);
}

// Test addrInfoToString function
TEST(UtilsTest, AddrInfoToString) {
    struct addrinfo addrInfo;
    addrInfo.ai_family = AF_INET;
    addrInfo.ai_socktype = SOCK_STREAM;
    addrInfo.ai_protocol = IPPROTO_TCP;
    addrInfo.ai_addrlen = sizeof(struct sockaddr_in);
    addrInfo.ai_addr = (struct sockaddr*)malloc(sizeof(struct sockaddr_in));
    addrInfo.ai_canonname = nullptr;
    addrInfo.ai_next = nullptr;

    std::string addrStr = addrInfoToString(&addrInfo, true);
    EXPECT_FALSE(addrStr.empty());

    free(addrInfo.ai_addr);
}

// Test getAddrInfo function
TEST(UtilsTest, GetAddrInfo) {
    struct addrinfo* addrInfo = getAddrInfo("www.google.com", "http");
    EXPECT_NE(addrInfo, nullptr);
    freeAddrInfo(addrInfo);
}

// Test freeAddrInfo function
TEST(UtilsTest, FreeAddrInfo) {
    struct addrinfo* addrInfo = getAddrInfo("www.google.com", "http");
    EXPECT_NE(addrInfo, nullptr);
    freeAddrInfo(addrInfo);
    // No assertion needed as we are just testing if the method runs without
    // error
}

// Test compareAddrInfo function
TEST(UtilsTest, CompareAddrInfo) {
    struct addrinfo* addrInfo1 = getAddrInfo("www.google.com", "http");
    struct addrinfo* addrInfo2 = getAddrInfo("www.google.com", "http");
    EXPECT_TRUE(compareAddrInfo(addrInfo1, addrInfo2));
    freeAddrInfo(addrInfo1);
    freeAddrInfo(addrInfo2);
}

// Test filterAddrInfo function
TEST(UtilsTest, FilterAddrInfo) {
    struct addrinfo* addrInfo = getAddrInfo("www.google.com", "http");
    struct addrinfo* filtered = filterAddrInfo(addrInfo, AF_INET);
    EXPECT_NE(filtered, nullptr);
    freeAddrInfo(addrInfo);
    freeAddrInfo(filtered);
}

// Test sortAddrInfo function
TEST(UtilsTest, SortAddrInfo) {
    struct addrinfo* addrInfo = getAddrInfo("www.google.com", "http");
    struct addrinfo* sorted = sortAddrInfo(addrInfo);
    EXPECT_NE(sorted, nullptr);
    freeAddrInfo(addrInfo);
    freeAddrInfo(sorted);
}
#endif
