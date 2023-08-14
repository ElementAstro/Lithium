#ifndef SOCKETCLIENT_H_
#define SOCKETCLIENT_H_

#include <functional>
#include <string>
#include <nlohmann/json.hpp>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#endif

using json = nlohmann::json;

class SocketClient
{
public:
    SocketClient();
    ~SocketClient();

    bool Connect(const std::string &serverIP, int serverPort);
    void Disconnect();
    void Send(const std::string &message);
    void SetMessageHandler(std::function<void(const json &)> handler);
    bool IsConnected() const;
    void StopReceiveThread();

private:
    SOCKET socket_;
    std::thread receiveThread_;
    std::function<void(const json &)> messageHandler_;
    bool isRunning_;
    std::mutex mutex_;
    std::condition_variable cv_;

    void ReceiveThread();
};

#endif /* SOCKETCLIENT_H_ */
