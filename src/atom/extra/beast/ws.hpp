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

/**
 * @class WSClient
 * @brief A WebSocket client class for managing WebSocket connections and
 * communication.
 */
class WSClient {
public:
    /**
     * @brief Constructs a WSClient with the given I/O context.
     * @param ioc The I/O context to use for asynchronous operations.
     */
    explicit WSClient(net::io_context& ioc);

    /**
     * @brief Sets the timeout duration for the WebSocket operations.
     * @param timeout The timeout duration in seconds.
     */
    void setTimeout(std::chrono::seconds timeout);

    /**
     * @brief Sets the reconnection options.
     * @param retries The number of reconnection attempts.
     * @param interval The interval between reconnection attempts in seconds.
     */
    void setReconnectOptions(int retries, std::chrono::seconds interval);

    /**
     * @brief Sets the interval for sending ping messages.
     * @param interval The ping interval in seconds.
     */
    void setPingInterval(std::chrono::seconds interval);

    /**
     * @brief Connects to the WebSocket server.
     * @param host The server host.
     * @param port The server port.
     */
    void connect(const std::string& host, const std::string& port);

    /**
     * @brief Sends a message to the WebSocket server.
     * @param message The message to send.
     */
    void send(const std::string& message);

    /**
     * @brief Receives a message from the WebSocket server.
     * @return The received message.
     */
    std::string receive();

    /**
     * @brief Closes the WebSocket connection.
     */
    void close();

    /**
     * @brief Asynchronously connects to the WebSocket server.
     * @tparam ConnectHandler The type of the handler to call when the operation
     * completes.
     * @param host The server host.
     * @param port The server port.
     * @param handler The handler to call when the operation completes.
     */
    template <class ConnectHandler>
    void asyncConnect(const std::string& host, const std::string& port,
                      ConnectHandler&& handler);

    /**
     * @brief Asynchronously sends a message to the WebSocket server.
     * @tparam WriteHandler The type of the handler to call when the operation
     * completes.
     * @param message The message to send.
     * @param handler The handler to call when the operation completes.
     */
    template <class WriteHandler>
    void asyncSend(const std::string& message, WriteHandler&& handler);

    /**
     * @brief Asynchronously receives a message from the WebSocket server.
     * @tparam ReadHandler The type of the handler to call when the operation
     * completes.
     * @param handler The handler to call when the operation completes.
     */
    template <class ReadHandler>
    void asyncReceive(ReadHandler&& handler);

    /**
     * @brief Asynchronously closes the WebSocket connection.
     * @tparam CloseHandler The type of the handler to call when the operation
     * completes.
     * @param handler The handler to call when the operation completes.
     */
    template <class CloseHandler>
    void asyncClose(CloseHandler&& handler);

    /**
     * @brief Asynchronously sends a JSON object to the WebSocket server.
     * @param jdata The JSON object to send.
     * @param handler The handler to call when the operation completes.
     */
    void asyncSendJson(
        const json& jdata,
        std::function<void(beast::error_code, std::size_t)> handler);

    /**
     * @brief Asynchronously receives a JSON object from the WebSocket server.
     * @tparam JsonHandler The type of the handler to call when the operation
     * completes.
     * @param handler The handler to call when the operation completes.
     */
    template <class JsonHandler>
    void asyncReceiveJson(JsonHandler&& handler);

private:
    /**
     * @brief Starts the ping timer to send periodic ping messages.
     */
    void startPing();

    /**
     * @brief Handles connection errors and retries if necessary.
     * @tparam ConnectHandler The type of the handler to call when the operation
     * completes.
     * @param ec The error code.
     * @param handler The handler to call when the operation completes.
     */
    template <class ConnectHandler>
    void handleConnectError(beast::error_code ec, ConnectHandler&& handler);

    tcp::resolver resolver_;             ///< The resolver for DNS lookups.
    websocket::stream<tcp::socket> ws_;  ///< The WebSocket stream.
    net::steady_timer ping_timer_;  ///< The timer for sending ping messages.
    std::chrono::seconds timeout_{
        30};  ///< The timeout duration for WebSocket operations.
    std::chrono::seconds ping_interval_{
        10};  ///< The interval for sending ping messages.
    std::chrono::seconds reconnect_interval_{
        5};                ///< The interval between reconnection attempts.
    int max_retries_ = 3;  ///< The maximum number of reconnection attempts.
    int retry_count_ = 0;  ///< The current number of reconnection attempts.
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
