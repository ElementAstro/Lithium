#include "phd2client.hpp"
#include <iostream>
#include <sstream>
#include <vector>
#include <loguru/loguru.hpp>

using json = nlohmann::json;

SocketClient::SocketClient()
    : socket_(INVALID_SOCKET), isRunning_(false)
{
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        LOG_F(ERROR, "Failed to initialize Winsock");
        throw std::runtime_error("Failed to initialize Winsock");
    }
#endif
}

SocketClient::~SocketClient()
{
    Disconnect();
}

bool SocketClient::Connect(const std::string &serverIP, int serverPort)
{
#ifdef _WIN32
    socket_ = socket(AF_INET, SOCK_STREAM, 0);
#else
    socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#endif
    if (socket_ == INVALID_SOCKET)
    {
        LOG_F(ERROR, "Failed to create socket");
#ifdef _WIN32
        WSACleanup();
#endif
        throw std::runtime_error("Failed to create socket");
    }

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverPort);

#ifdef _WIN32
    if (InetPton(AF_INET, serverIP.c_str(), &(serverAddress.sin_addr)) <= 0)
    {
        LOG_F(ERROR, "Invalid server IP address");
        closesocket(socket_);
        WSACleanup();
        throw std::runtime_error("Invalid server IP address");
    }
#else
    if (inet_pton(AF_INET, serverIP.c_str(), &(serverAddress.sin_addr)) <= 0)
    {
        LOG_F(ERROR, "Invalid server IP address");
        throw std::runtime_error("Invalid server IP address");
    }
#endif

    if (connect(socket_, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        LOG_F(ERROR, "Failed to connect to server");
        throw std::runtime_error("Failed to connect to server");
    }

    isRunning_ = true;
    receiveThread_ = std::thread([&]()
                                 { ReceiveThread(); });

    return true;
}

void SocketClient::Disconnect()
{
    if (socket_ != INVALID_SOCKET)
    {
#ifdef _WIN32
        closesocket(socket_);
#else
        close(socket_);
#endif
        socket_ = INVALID_SOCKET;
    }

    if (isRunning_)
    {
        isRunning_ = false;
        if (receiveThread_.joinable())
        {
            receiveThread_.join();
        }
    }
}

void SocketClient::Send(const std::string &message)
{
    if (socket_ == INVALID_SOCKET)
    {
        LOG_F(ERROR, "Not connected to server");
        return;
    }

    if (send(socket_, message.c_str(), message.length(), 0) < 0)
    {
        LOG_F(ERROR, "Failed to send data");
        throw std::runtime_error("Failed to send data");
    }
}

void SocketClient::SetMessageHandler(std::function<void(const json &)> handler)
{
    messageHandler_ = std::move(handler);
}

bool SocketClient::IsConnected() const
{
    return socket_ != INVALID_SOCKET;
}

void SocketClient::StopReceiveThread()
{
    if (isRunning_)
    {
        isRunning_ = false;

        // 等待接收线程退出
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock);
    }
}

void SocketClient::ReceiveThread()
{
    while (isRunning_)
    {
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));

        int bytesRead = recv(socket_, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead <= 0)
        {
            if (bytesRead < 0)
            {
                LOG_F(ERROR, "Failed to receive data: %d", bytesRead);
            }
            else
            {
                LOG_F(INFO, "Connection closed by server");
            }
            break;
        }

        std::string receivedData(buffer);
        std::istringstream iss(receivedData);
        std::string line;

        while (std::getline(iss, line))
        {
            json jsonData;
            try
            {
                jsonData = json::parse(line);
            }
            catch (const std::exception &e)
            {
                std::cerr << "Failed to parse JSON data: " << e.what() << std::endl;
                continue;
            }

            // 调用消息处理函数
            if (messageHandler_)
            {
                messageHandler_(jsonData);
            }
        }
    }

    // 停止接收线程后通知等待的条件变量
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.notify_all();
}
