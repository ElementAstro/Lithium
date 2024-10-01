#include "async_udpclient.hpp"

#include <asio.hpp>
#include <iostream>
#include <thread>

namespace atom::async::connection {

class UdpClient::Impl {
public:
    Impl() : io_context_(), socket_(io_context_), is_receiving_(false) {}

    bool bind(int port) {
        try {
            asio::ip::udp::endpoint endpoint(asio::ip::udp::v4(), port);
            socket_.open(endpoint.protocol());
            socket_.bind(endpoint);
            return true;
        } catch (...) {
            return false;
        }
    }

    bool send(const std::string& host, int port,
              const std::vector<char>& data) {
        try {
            asio::ip::udp::resolver resolver(io_context_);
            asio::ip::udp::endpoint destination =
                *resolver.resolve(host, std::to_string(port)).begin();
            socket_.send_to(asio::buffer(data), destination);
            return true;
        } catch (...) {
            return false;
        }
    }

    std::vector<char> receive(size_t size, std::string& remoteHost,
                              int& remotePort,
                              std::chrono::milliseconds timeout) {
        std::vector<char> data(size);
        asio::ip::udp::endpoint senderEndpoint;
        asio::error_code ec;
        socket_.receive_from(asio::buffer(data), senderEndpoint, 0, ec);
        if (!ec) {
            remoteHost = senderEndpoint.address().to_string();
            remotePort = senderEndpoint.port();
            return data;
        }
        return {};
    }

    void setOnDataReceivedCallback(const OnDataReceivedCallback& callback) {
        onDataReceivedCallback_ = callback;
    }

    void setOnErrorCallback(const OnErrorCallback& callback) {
        onErrorCallback_ = callback;
    }

    void startReceiving(size_t bufferSize) {
        is_receiving_ = true;
        receive_buffer_.resize(bufferSize);
        doReceive();
        receive_thread_ = std::thread([this] { io_context_.run(); });
    }

    void stopReceiving() {
        is_receiving_ = false;
        socket_.close();
        if (receive_thread_.joinable()) {
            receive_thread_.join();
        }
    }

private:
    void doReceive() {
        if (!is_receiving_)
            return;

        socket_.async_receive_from(
            asio::buffer(receive_buffer_), remote_endpoint_,
            [this](std::error_code ec, std::size_t bytes_recvd) {
                if (!ec && bytes_recvd > 0) {
                    if (onDataReceivedCallback_) {
                        auto data = std::vector<char>(
                            receive_buffer_.begin(),
                            receive_buffer_.begin() + bytes_recvd);
                        onDataReceivedCallback_(
                            data, remote_endpoint_.address().to_string(),
                            remote_endpoint_.port());
                    }
                    doReceive();
                } else {
                    if (onErrorCallback_) {
                        onErrorCallback_("Receive error");
                    }
                }
            });
    }

    asio::io_context io_context_;
    asio::ip::udp::socket socket_;
    asio::ip::udp::endpoint remote_endpoint_;
    std::vector<char> receive_buffer_;
    std::thread receive_thread_;
    bool is_receiving_;
    OnDataReceivedCallback onDataReceivedCallback_;
    OnErrorCallback onErrorCallback_;
};

UdpClient::UdpClient() : impl_(std::make_unique<Impl>()) {}
UdpClient::~UdpClient() = default;

bool UdpClient::bind(int port) { return impl_->bind(port); }

bool UdpClient::send(const std::string& host, int port,
                     const std::vector<char>& data) {
    return impl_->send(host, port, data);
}

std::vector<char> UdpClient::receive(size_t size, std::string& remoteHost,
                                     int& remotePort,
                                     std::chrono::milliseconds timeout) {
    return impl_->receive(size, remoteHost, remotePort, timeout);
}

void UdpClient::setOnDataReceivedCallback(
    const OnDataReceivedCallback& callback) {
    impl_->setOnDataReceivedCallback(callback);
}

void UdpClient::setOnErrorCallback(const OnErrorCallback& callback) {
    impl_->setOnErrorCallback(callback);
}

void UdpClient::startReceiving(size_t bufferSize) {
    impl_->startReceiving(bufferSize);
}

void UdpClient::stopReceiving() { impl_->stopReceiving(); }

}  // namespace atom::async::connection
