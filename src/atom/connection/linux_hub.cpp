#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <cstring>
#include <algorithm>

#include "atom/connection/libipc/ipc.h"

class IPCHub
{
public:
    IPCHub() : running(false) {}

    void start(int port)
    {
        if (running)
        {
            std::cout << "IPCHub is already running." << std::endl;
            return;
        }

        // 创建服务器套接字
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == INVALID_SOCKET)
        {
            std::cerr << "Failed to create server socket." << std::endl;
            cleanupWinsock();
            return;
        }

        // 设置服务器地址和端口
        sockaddr_in serverAddress{};
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = INADDR_ANY;
        serverAddress.sin_port = htons(port);

        // 绑定服务器套接字到服务器地址和端口
        if (bind(serverSocket, reinterpret_cast<sockaddr *>(&serverAddress), sizeof(serverAddress)) == SOCKET_ERROR)
        {
            std::cerr << "Failed to bind server socket." << std::endl;
            cleanupSocket();
            return;
        }

        // 监听连接
        if (listen(serverSocket, maxConnections) == SOCKET_ERROR)
        {
            std::cerr << "Failed to listen on server socket." << std::endl;
            cleanupSocket();
            return;
        }

        running = true;
        std::cout << "IPCHub started on port " << port << std::endl;

        // 启动接受连接的线程
        acceptThread = std::thread(&IPCHub::acceptConnections, this);
    }

    void stop()
    {
        if (!running)
        {
            std::cout << "IPCHub is not running." << std::endl;
            return;
        }

        // 停止接受连接的线程
        acceptThread.join();

        // 关闭所有客户端套接字
        for (const auto &client : clients)
        {
            closeSocket(client);
        }
        clients.clear();

        // 关闭服务器套接字
        closeSocket(serverSocket);

        cleanupSocket();
        cleanupWinsock();

        running = false;
        std::cout << "IPCHub stopped." << std::endl;
    }

private:
    static const int maxConnections = 10;

    bool running;
    SOCKET serverSocket;
    std::vector<SOCKET> clients;
    std::thread acceptThread;

    bool initWinsock()
    {
#ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        {
            std::cerr << "Failed to initialize Winsock." << std::endl;
            return false;
        }
#endif

        return true;
    }

    void cleanupWinsock()
    {
#ifdef _WIN32
        WSACleanup();
#endif
    }

    void closeSocket(SOCKET socket)
    {
#ifdef _WIN32
        closesocket(socket);
#else
        close(socket);
#endif
    }

    void acceptConnections()
    {
        while (running)
        {
            sockaddr_in clientAddress{};
            socklen_t clientAddressLength = sizeof(clientAddress);

            // 接受新连接
            SOCKET clientSocket = accept(serverSocket, reinterpret_cast<sockaddr *>(&clientAddress), &clientAddressLength);
            if (clientSocket == INVALID_SOCKET)
            {
                std::cerr << "Failed to accept client connection." << std::endl;
                continue;
            }

            // 添加到客户端列表
            clients.push_back(clientSocket);

            // 启动处理客户端消息的线程
            std::thread clientThread(&IPCHub::handleClientMessages, this, clientSocket);
            clientThread.detach();
        }
    }

    void handleClientMessages(SOCKET clientSocket)
    {
        char buffer[1024];
        while (running)
        {
            // 接收客户端消息
            memset(buffer, 0, sizeof(buffer));
            int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesRead <= 0)
            {
                // 客户端断开连接
                closeSocket(clientSocket);
                clients.erase(std::remove(clients.begin(), clients.end(), clientSocket), clients.end());
                break;
            }

            // 处理客户端消息
            std::string message(buffer, bytesRead);
            std::cout << "Received message from client: " << message << std::endl;

            // 回复客户端
            std::string reply = "Message received: " + message;
            send(clientSocket, reply.c_str(), reply.length(), 0);
        }
    }

    void cleanupSocket()
    {
        for (const auto &client : clients)
        {
            closeSocket(client);
        }
        clients.clear();

        closeSocket(serverSocket);
    }
};

int main()
{
    IPCHub socketHub;
    socketHub.start(12345);

    std::this_thread::sleep_for(std::chrono::seconds(30));

    socketHub.stop();

    return 0;
}
