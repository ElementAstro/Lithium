#ifndef WS_CLIENT_HPP
#define WS_CLIENT_HPP

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <chrono>
#include <functional>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>

namespace beast = boost::beast;
namespace net = boost::asio;
namespace websocket = beast::websocket;
using tcp = boost::asio::ip::tcp;
using json = nlohmann::json;

class WSClient {
public:
    explicit WSClient(net::io_context& ioc);

    void setTimeout(std::chrono::seconds timeout);
    void setReconnectOptions(int retries, std::chrono::seconds interval);
    void setPingInterval(std::chrono::seconds interval);

    void connect(const std::string& host, const std::string& port);
    void send(const std::string& message);
    std::string receive();
    void close();

    template <class ConnectHandler>
    void asyncConnect(const std::string& host, const std::string& port,
                      ConnectHandler&& handler);

    template <class WriteHandler>
    void asyncSend(const std::string& message, WriteHandler&& handler);

    template <class ReadHandler>
    void asyncReceive(ReadHandler&& handler);

    template <class CloseHandler>
    void asyncClose(CloseHandler&& handler);

    void asyncSendJson(
        const json& jdata,
        std::function<void(beast::error_code, std::size_t)> handler);

    template <class JsonHandler>
    void asyncReceiveJson(JsonHandler&& handler);

private:
    void startPing();
    template <class ConnectHandler>
    void handleConnectError(beast::error_code ec, ConnectHandler&& handler);

    tcp::resolver resolver_;
    websocket::stream<tcp::socket> ws_;
    net::steady_timer ping_timer_;
    std::chrono::seconds timeout_{30};
    std::chrono::seconds ping_interval_{10};
    std::chrono::seconds reconnect_interval_{5};
    int max_retries_ = 3;
    int retry_count_ = 0;
};

template <class ConnectHandler>
void WSClient::asyncConnect(const std::string& host, const std::string& port,
                            ConnectHandler&& handler) {
    retry_count_ = 0;
    resolver_.async_resolve(
        host, port,
        [this, handler = std::forward<ConnectHandler>(handler)](
            beast::error_code ec, tcp::resolver::results_type results) {
            if (ec) {
                handleConnectError(ec, handler);
                return;
            }

            beast::get_lowest_layer(ws_).async_connect(
                results, [this, handler = std::move(handler), results](
                             beast::error_code ec,
                             tcp::resolver::results_type::endpoint_type) {
                    if (ec) {
                        handleConnectError(ec, handler);
                        return;
                    }

                    ws_.async_handshake(results->host_name(), "/",
                                        [this, handler = std::move(handler)](
                                            beast::error_code ec) {
                                            if (!ec) {
                                                startPing();
                                            }
                                            handler(ec);
                                        });
                });
        });
}

template <class WriteHandler>
void WSClient::asyncSend(const std::string& message, WriteHandler&& handler) {
    ws_.async_write(net::buffer(message),
                    [handler = std::forward<WriteHandler>(handler)](
                        beast::error_code ec, std::size_t bytes_transferred) {
                        handler(ec, bytes_transferred);
                    });
}

template <class ReadHandler>
void WSClient::asyncReceive(ReadHandler&& handler) {
    auto buffer = std::make_shared<beast::flat_buffer>();
    ws_.async_read(
        *buffer, [buffer, handler = std::forward<ReadHandler>(handler)](
                     beast::error_code ec, std::size_t bytes_transferred) {
            if (ec) {
                handler(ec, "");
            } else {
                handler(ec, beast::buffers_to_string(buffer->data()));
            }
        });
}

template <class CloseHandler>
void WSClient::asyncClose(CloseHandler&& handler) {
    ws_.async_close(websocket::close_code::normal,
                    [handler = std::forward<CloseHandler>(handler)](
                        beast::error_code ec) { handler(ec); });
}

template <class JsonHandler>
void WSClient::asyncReceiveJson(JsonHandler&& handler) {
    asyncReceive([handler = std::forward<JsonHandler>(handler)](
                     beast::error_code ec, const std::string& message) {
        if (ec) {
            handler(ec, {});
        } else {
            try {
                auto jdata = json::parse(message);
                handler(ec, jdata);
            } catch (const json::parse_error&) {
                handler(beast::error_code{}, {});
            }
        }
    });
}

#endif  // WS_CLIENT_HPP
