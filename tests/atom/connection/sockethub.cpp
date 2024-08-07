#include <gtest/gtest.h>

#include <mutex>
#include <thread>
#include <chrono>

#include "atom/connection/sockethub.hpp"

#ifdef __linux
#include <arpa/inet.h>
#include <netinet/in.h>
#endif

using namespace atom::connection;

class SocketHubTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Start the SocketHub on a separate thread
        socketHub_ = std::make_unique<SocketHub>();
        socketHub_->addHandler([this](const std::string &message) {
            std::scoped_lock lock(mutex_);
            messages_.push_back(message);
        });
        socketHub_->start(port_);
        std::this_thread::sleep_for(std::chrono::seconds(1)); // Give some time for server to start
    }

    void TearDown() override {
        socketHub_->stop();
        socketHub_.reset();
    }

    std::unique_ptr<SocketHub> socketHub_;
    int port_ = 8080;
    std::vector<std::string> messages_;
    std::mutex mutex_;
};

TEST_F(SocketHubTest, StartAndStop) {
    ASSERT_TRUE(socketHub_->isRunning());
    socketHub_->stop();
    ASSERT_FALSE(socketHub_->isRunning());
}

TEST_F(SocketHubTest, AcceptConnection) {
    int clientSocket = ::socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_NE(clientSocket, -1);

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port_);
    inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr);

    int result = ::connect(clientSocket, (sockaddr *)&serverAddress, sizeof(serverAddress));
    ASSERT_EQ(result, 0);

    ::close(clientSocket);
}

TEST_F(SocketHubTest, SendAndReceiveMessage) {
    int clientSocket = ::socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_NE(clientSocket, -1);

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port_);
    inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr);

    int result = ::connect(clientSocket, (sockaddr *)&serverAddress, sizeof(serverAddress));
    ASSERT_EQ(result, 0);

    std::string message = "Hello, server!";
    result = ::send(clientSocket, message.c_str(), message.size(), 0);
    ASSERT_NE(result, -1);

    std::this_thread::sleep_for(std::chrono::seconds(1)); // Give some time for message to be handled

    {
        std::scoped_lock lock(mutex_);
        ASSERT_EQ(messages_.size(), 1);
        ASSERT_EQ(messages_[0], message);
    }

    ::close(clientSocket);
}

TEST_F(SocketHubTest, HandleMultipleClients) {
    const int clientCount = 5;
    std::vector<int> clientSockets(clientCount);

    for (int i = 0; i < clientCount; ++i) {
        clientSockets[i] = ::socket(AF_INET, SOCK_STREAM, 0);
        ASSERT_NE(clientSockets[i], -1);

        sockaddr_in serverAddress{};
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(port_);
        inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr);

        int result = ::connect(clientSockets[i], (sockaddr *)&serverAddress, sizeof(serverAddress));
        ASSERT_EQ(result, 0);
    }

    std::string message = "Hello, server!";
    for (int i = 0; i < clientCount; ++i) {
        int result = ::send(clientSockets[i], message.c_str(), message.size(), 0);
        ASSERT_NE(result, -1);
    }

    std::this_thread::sleep_for(std::chrono::seconds(1)); // Give some time for messages to be handled

    {
        std::scoped_lock lock(mutex_);
        ASSERT_EQ(messages_.size(), clientCount);
        for (const auto &msg : messages_) {
            ASSERT_EQ(msg, message);
        }
    }

    for (int i = 0; i < clientCount; ++i) {
        ::close(clientSockets[i]);
    }
}
