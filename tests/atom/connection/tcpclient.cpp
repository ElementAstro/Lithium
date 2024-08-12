#include "atom/connection/tcpclient.hpp"
#include <gtest/gtest.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <future>
#include <thread>

using namespace atom::connection;

class MockServer {
public:
    MockServer(int port) : port_(port), serverSocket_(-1), clientSocket_(-1) {}

    ~MockServer() { stop(); }

    void start() { serverThread_ = std::thread(&MockServer::run, this); }

    void stop() {
        if (serverThread_.joinable()) {
            stop_ = true;
            serverThread_.join();
        }

        if (clientSocket_ != -1) {
#ifdef _WIN32
            closesocket(clientSocket_);
#else
            close(clientSocket_);
#endif
        }

        if (serverSocket_ != -1) {
#ifdef _WIN32
            closesocket(serverSocket_);
            WSACleanup();
#else
            close(serverSocket_);
#endif
        }
    }

private:
    void run() {
#ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
        serverSocket_ = socket(AF_INET, SOCK_STREAM, 0);
        ASSERT_NE(serverSocket_, -1) << "Failed to create server socket";

        struct sockaddr_in serverAddr {};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(port_);

        int opt = 1;
        setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, (char*)&opt,
                   sizeof(opt));

        int result = bind(serverSocket_, (struct sockaddr*)&serverAddr,
                          sizeof(serverAddr));
        ASSERT_EQ(result, 0) << "Bind failed";

        result = listen(serverSocket_, 1);
        ASSERT_EQ(result, 0) << "Listen failed";

        while (!stop_) {
            struct sockaddr_in clientAddr {};
            socklen_t clientLen = sizeof(clientAddr);

            clientSocket_ = accept(serverSocket_, (struct sockaddr*)&clientAddr,
                                   &clientLen);
            if (clientSocket_ < 0) {
                if (stop_)
                    break;
                continue;
            }

            char buffer[1024];
            int bytesRead = recv(clientSocket_, buffer, sizeof(buffer), 0);
            if (bytesRead > 0) {
                send(clientSocket_, buffer, bytesRead, 0);
            }

#ifdef _WIN32
            closesocket(clientSocket_);
#else
            close(clientSocket_);
#endif
            clientSocket_ = -1;
        }
    }

    int port_;
    int serverSocket_;
    int clientSocket_;
    bool stop_ = false;
    std::thread serverThread_;
};

class TcpClientTest : public ::testing::Test {
protected:
    void SetUp() override { mockServer_.start(); }

    void TearDown() override { mockServer_.stop(); }

    MockServer mockServer_{8080};
    TcpClient client_;
};

TEST_F(TcpClientTest, ConnectToServer) {
    ASSERT_TRUE(
        client_.connect("127.0.0.1", 8080, std::chrono::milliseconds(5000)));
    ASSERT_TRUE(client_.isConnected());
}

TEST_F(TcpClientTest, SendData) {
    ASSERT_TRUE(
        client_.connect("127.0.0.1", 8080, std::chrono::milliseconds(5000)));
    std::string message = "Hello, server!";
    ASSERT_TRUE(
        client_.send(std::vector<char>(message.begin(), message.end())));
}

TEST_F(TcpClientTest, ReceiveData) {
    ASSERT_TRUE(
        client_.connect("127.0.0.1", 8080, std::chrono::milliseconds(5000)));
    std::string message = "Hello, server!";
    ASSERT_TRUE(
        client_.send(std::vector<char>(message.begin(), message.end())));

    auto futureData = client_.receive(1024);
    auto data = futureData.get();

    ASSERT_EQ(std::string(data.begin(), data.end()), message);
}

TEST_F(TcpClientTest, DisconnectFromServer) {
    ASSERT_TRUE(
        client_.connect("127.0.0.1", 8080, std::chrono::milliseconds(5000)));
    client_.disconnect();
    ASSERT_FALSE(client_.isConnected());
}

TEST_F(TcpClientTest, Callbacks) {
    bool connected = false;
    bool disconnected = false;
    std::string receivedData;
    std::string errorMessage;

    client_.setOnConnectedCallback([&]() { connected = true; });
    client_.setOnDisconnectedCallback([&]() { disconnected = true; });
    client_.setOnDataReceivedCallback([&](const std::vector<char>& data) {
        receivedData = std::string(data.begin(), data.end());
    });
    client_.setOnErrorCallback(
        [&](const std::string& error) { errorMessage = error; });

    ASSERT_TRUE(
        client_.connect("127.0.0.1", 8080, std::chrono::milliseconds(5000)));
    ASSERT_TRUE(connected);

    std::string message = "Hello, server!";
    ASSERT_TRUE(
        client_.send(std::vector<char>(message.begin(), message.end())));

    std::this_thread::sleep_for(
        std::chrono::seconds(1));  // Give some time to receive the message

    ASSERT_EQ(receivedData, message);

    client_.disconnect();
    ASSERT_TRUE(disconnected);
}
