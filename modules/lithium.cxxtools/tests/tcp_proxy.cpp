/*
 * test_tcp_proxy.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-1

Description: Unit tests for Tcp proxy server

*************************************************/

#include "tcp_proxy.cpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>


#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

using ::testing::_;
using ::testing::Return;

class TcpProxyTest : public ::testing::Test {
protected:
    void SetUp() override {
#ifdef _WIN32
        WSADATA wsData;
        WSAStartup(MAKEWORD(2, 2), &wsData);
#endif
    }

    void TearDown() override {
#ifdef _WIN32
        WSACleanup();
#endif
    }
};

TEST_F(TcpProxyTest, TestMainFunctionWithDefaultArgs) {
    char *argv[] = {const_cast<char *>("tcp_proxy"), nullptr};
    int argc = 1;
    EXPECT_EQ(main(argc, argv), 0);
}

TEST_F(TcpProxyTest, TestMainFunctionWithCustomArgs) {
    char *argv[] = {const_cast<char *>("tcp_proxy"),   const_cast<char *>("-s"),
                    const_cast<char *>("192.168.1.1"), const_cast<char *>("-p"),
                    const_cast<char *>("8080"),        const_cast<char *>("-d"),
                    const_cast<char *>("192.168.1.2"), const_cast<char *>("-o"),
                    const_cast<char *>("9090"),        nullptr};
    int argc = 9;
    EXPECT_EQ(main(argc, argv), 0);
}

TEST_F(TcpProxyTest, TestSignalHandling) {
    signalHandler(SIGINT);
    // Expect the program to exit, so we can't directly test it here.
    // Instead, we can check if the log message is printed.
}

TEST_F(TcpProxyTest, TestSocketCreationFailure) {
    // Mock socket creation to fail
    EXPECT_CALL(socket(_, _, _)).WillOnce(Return(-1));
    EXPECT_THROW(startProxyServer("127.0.0.1", 12345, "127.0.0.1", 54321),
                 std::runtime_error);
}

TEST_F(TcpProxyTest, TestSocketConnectionFailure) {
    // Mock socket connection to fail
    EXPECT_CALL(connect(_, _, _)).WillOnce(Return(-1));
    EXPECT_THROW(startProxyServer("127.0.0.1", 12345, "127.0.0.1", 54321),
                 std::runtime_error);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
