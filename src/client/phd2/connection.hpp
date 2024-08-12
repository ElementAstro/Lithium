#ifndef LITHIUM_CLIENT_PHD2_CONNECTION_HPP
#define LITHIUM_CLIENT_PHD2_CONNECTION_HPP

#include <curl/curl.h>
#include <atomic>
#include <deque>
#include <optional>
#include <sstream>
#include <thread>

class GuiderConnection {
public:
    GuiderConnection();
    ~GuiderConnection();
    auto connect(const char *hostname, unsigned short port) -> bool;
    void disconnect();
    auto isConnected() const -> bool { return m_curl_ != nullptr; }
    auto readLine() -> std::optional<std::string>;
    auto writeLine(const std::string &s) -> bool;
    void terminate();

private:
    CURL *m_curl_{};
    curl_socket_t m_sockfd_{};
    std::deque<std::string> m_dq_;
    std::ostringstream m_os_;
    std::jthread m_thread_;
    std::atomic_bool m_terminate_;

    auto waitReadable(std::stop_token st) const -> bool;
    auto waitWritable(std::stop_token st) const -> bool;
};

#endif