#include "ws.hpp"

#if __has_include("atom/log/loguru.hpp")
#include "atom/log/loguru.hpp"
#else
#include <loguru.hpp>
#endif

WSClient::WSClient(net::io_context& ioc)
    : resolver_(net::make_strand(ioc)),
      ws_(net::make_strand(ioc)),
      ping_timer_(ioc) {}

void WSClient::setTimeout(std::chrono::seconds timeout) { timeout_ = timeout; }

void WSClient::setReconnectOptions(int retries, std::chrono::seconds interval) {
    max_retries_ = retries;
    reconnect_interval_ = interval;
}

void WSClient::setPingInterval(std::chrono::seconds interval) {
    ping_interval_ = interval;
}

void WSClient::connect(const std::string& host, const std::string& port) {
    auto const results = resolver_.resolve(host, port);
    beast::get_lowest_layer(ws_).connect(results->endpoint());
    ws_.handshake(host, "/");
    startPing();
}

void WSClient::send(const std::string& message) {
    ws_.write(net::buffer(message));
}

std::string WSClient::receive() {
    beast::flat_buffer buffer;
    ws_.read(buffer);
    return beast::buffers_to_string(buffer.data());
}

void WSClient::close() { ws_.close(websocket::close_code::normal); }

void WSClient::startPing() {
    if (ping_interval_.count() > 0) {
        ping_timer_.expires_after(ping_interval_);
        ping_timer_.async_wait([this](beast::error_code ec) {
            if (!ec) {
                ws_.async_ping({}, [this](beast::error_code ec) {
                    if (!ec) {
                        startPing();
                    }
                });
            }
        });
    }
}

template <class ConnectHandler>
void WSClient::handleConnectError(beast::error_code ec,
                                  ConnectHandler&& handler) {
    if (retry_count_ < max_retries_) {
        ++retry_count_;
        LOG_F(ERROR, "Failed to connect: {}. Retrying in {} seconds...",
              ec.message(), reconnect_interval_.count());
        ws_.next_layer().close();
        ping_timer_.expires_after(reconnect_interval_);
        ping_timer_.async_wait([this, handler = std::forward<ConnectHandler>(
                                          handler)](beast::error_code ec) {
            if (!ec) {
                asyncConnect("example.com", "80",
                             std::forward<ConnectHandler>(handler));
            }
        });
    } else {
        LOG_F(ERROR, "Failed to connect: {}. Giving up.", ec.message());
        handler(ec);
    }
}
