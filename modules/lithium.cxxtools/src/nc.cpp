// nc.cpp
#include <array>
#include <asio.hpp>
#include <chrono>
#include <format>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>

#include "atom/log/loguru.hpp"

using asio::ip::tcp;
using asio::ip::udp;

constexpr std::size_t MAX_LENGTH = 1024;
constexpr int ARG_COUNT_MIN = 4;
constexpr int ARG_COUNT_MAX = 6;
constexpr int DEFAULT_TIMEOUT_SECONDS = 10;

// Function: Send file over TCP
void sendFileTcp(tcp::socket& socket, const std::string& filename) {
    LOG_F(INFO, "Attempting to send file over TCP: {}", filename);
    try {
        std::ifstream file(filename, std::ios::binary);
        if (!file) {
            LOG_F(ERROR, "Failed to open file: {}", filename);
            return;
        }

        std::array<char, MAX_LENGTH> buffer;
        while (file.read(buffer.data(), buffer.size())) {
            asio::write(socket, asio::buffer(buffer, file.gcount()));
            LOG_F(INFO, "Sent {} bytes over TCP", file.gcount());
        }
        if (file.gcount() > 0) {
            asio::write(socket, asio::buffer(buffer, file.gcount()));
            LOG_F(INFO, "Sent remaining {} bytes over TCP", file.gcount());
        }

        LOG_F(INFO, "File successfully sent: {}", filename);
    } catch (const std::ios_base::failure& e) {
        LOG_F(ERROR, "File I/O error while sending file over TCP: {}",
              e.what());
    } catch (const asio::system_error& e) {
        LOG_F(ERROR, "ASIO system error while sending file over TCP: {}",
              e.what());
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Unexpected error while sending file over TCP: {}",
              e.what());
    }
}

// Function: Send file over UDP
void sendFileUdp(udp::socket& socket, const udp::endpoint& endpoint,
                 const std::string& filename) {
    LOG_F(INFO, "Attempting to send file over UDP: {}", filename);
    try {
        std::ifstream file(filename, std::ios::binary);
        if (!file) {
            LOG_F(ERROR, "Failed to open file: {}", filename);
            return;
        }

        std::array<char, MAX_LENGTH> buffer;
        while (file.read(buffer.data(), buffer.size())) {
            socket.send_to(asio::buffer(buffer, file.gcount()), endpoint);
            LOG_F(INFO, "Sent {} bytes over UDP", file.gcount());
        }
        if (file.gcount() > 0) {
            socket.send_to(asio::buffer(buffer, file.gcount()), endpoint);
            LOG_F(INFO, "Sent remaining {} bytes over UDP", file.gcount());
        }

        LOG_F(INFO, "File successfully sent: {}", filename);
    } catch (const std::ios_base::failure& e) {
        LOG_F(ERROR, "File I/O error while sending file over UDP: {}",
              e.what());
    } catch (const asio::system_error& e) {
        LOG_F(ERROR, "ASIO system error while sending file over UDP: {}",
              e.what());
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Unexpected error while sending file over UDP: {}",
              e.what());
    }
}

// TCP mode client
void runTcpClient(const std::string& host, const std::string& port,
                  int timeoutSeconds,
                  const std::optional<std::string>& filename = std::nullopt) {
    LOG_F(INFO, "Running TCP client with host: {}, port: {}, timeout: {}", host,
          port, timeoutSeconds);
    try {
        asio::io_context ioContext;
        tcp::resolver resolver(ioContext);
        auto endpoints = resolver.resolve(host, port);
        tcp::socket socket(ioContext);

        LOG_F(INFO, "Attempting to connect to {}:{}", host, port);
        asio::connect(socket, endpoints);
        LOG_F(INFO, "Connected to {}:{}", host, port);

        if (filename.has_value()) {
            sendFileTcp(socket, filename.value());
        } else {
            asio::steady_timer timer(ioContext);
            while (true) {
                std::string message;
                std::getline(std::cin, message);

                if (message.empty()) {
                    LOG_F(INFO,
                          "Empty message received, terminating TCP client.");
                    break;
                }

                timer.expires_after(std::chrono::seconds(timeoutSeconds));
                timer.async_wait([&socket](const asio::error_code& errorCode) {
                    if (!errorCode) {
                        LOG_F(ERROR, "Operation timed out. Closing socket.");
                        socket.close();
                    }
                });

                asio::write(socket, asio::buffer(message + "\n"));
                LOG_F(INFO, "Sent message over TCP: {}", message);

                std::array<char, MAX_LENGTH> reply;
                std::error_code error;
                size_t replyLength =
                    asio::read(socket, asio::buffer(reply), error);

                if (error) {
                    LOG_F(ERROR, "Read error on TCP socket: {}",
                          error.message());
                    break;
                }

                std::string replyStr(reply.data(), replyLength);
                LOG_F(INFO, "Received reply over TCP: {}", replyStr);
                std::cout << "Reply: " << replyStr << std::endl;
                timer.cancel();
            }
        }
    } catch (const asio::system_error& e) {
        LOG_F(ERROR, "ASIO system error in TCP client: {}", e.what());
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception in TCP client: {}", e.what());
    } catch (...) {
        LOG_F(ERROR, "Unknown exception occurred in TCP client");
    }
}

// UDP mode client
void runUdpClient(const std::string& host, const std::string& port,
                  const std::optional<std::string>& filename = std::nullopt) {
    LOG_F(INFO, "Running UDP client with host: {}, port: {}", host, port);
    try {
        asio::io_context ioContext;
        udp::resolver resolver(ioContext);
        udp::resolver::results_type endpoints =
            resolver.resolve(udp::v4(), host, port);
        udp::socket socket(ioContext);

        socket.open(udp::v4());
        LOG_F(INFO, "UDP socket opened.");

        if (filename.has_value()) {
            sendFileUdp(socket, *endpoints.begin(), filename.value());
        } else {
            while (true) {
                std::string message;
                std::getline(std::cin, message);

                if (message.empty()) {
                    LOG_F(INFO,
                          "Empty message received, terminating UDP client.");
                    break;
                }

                socket.send_to(asio::buffer(message), *endpoints.begin());
                LOG_F(INFO, "Sent message over UDP: {}", message);

                std::array<char, MAX_LENGTH> reply;
                udp::endpoint senderEndpoint;
                std::error_code error;
                size_t replyLength = socket.receive_from(
                    asio::buffer(reply), senderEndpoint, 0, error);

                if (error && error != asio::error::message_size) {
                    LOG_F(ERROR, "Receive error on UDP socket: {}",
                          error.message());
                    break;
                }

                std::string replyStr(reply.data(), replyLength);
                LOG_F(INFO, "Received reply from {}: {}", senderEndpoint,
                      replyStr);
                std::cout << "Reply from " << senderEndpoint << ": " << replyStr
                          << std::endl;
            }
        }
    } catch (const asio::system_error& e) {
        LOG_F(ERROR, "ASIO system error in UDP client: {}", e.what());
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception in UDP client: {}", e.what());
    } catch (...) {
        LOG_F(ERROR, "Unknown exception occurred in UDP client");
    }
}

int main(int argc, char* argv[]) {
    loguru::init(argc, argv);
    LOG_F(INFO, "Application started.");

    try {
        if (argc < ARG_COUNT_MIN || argc > ARG_COUNT_MAX) {
            LOG_F(ERROR, "Invalid number of arguments.");
            std::cerr
                << "Usage: " << argv[0]
                << " <tcp|udp> <host> <port> [timeout_seconds] [file_to_send]"
                << std::endl;
            return 1;
        }

        std::string protocol = argv[1];
        std::string host = argv[2];
        std::string port = argv[3];
        int timeoutSeconds =
            (argc >= 5) ? std::stoi(argv[4]) : DEFAULT_TIMEOUT_SECONDS;
        std::optional<std::string> filename =
            (argc == 6) ? std::make_optional<std::string>(argv[5])
                        : std::nullopt;

        LOG_F(INFO, "Protocol: {}, Host: {}, Port: {}, Timeout: {}, File: {}",
              protocol, host, port, timeoutSeconds, filename.value_or("None"));

        if (protocol == "tcp") {
            runTcpClient(host, port, timeoutSeconds, filename);
        } else if (protocol == "udp") {
            runUdpClient(host, port, filename);
        } else {
            LOG_F(ERROR, "Unknown protocol: {}", protocol);
            std::cerr << "Unknown protocol: " << protocol << std::endl;
            return 1;
        }
    } catch (const std::invalid_argument& e) {
        LOG_F(ERROR, "Invalid argument: {}", e.what());
        std::cerr << "Invalid argument: " << e.what() << std::endl;
        return 1;
    } catch (const std::out_of_range& e) {
        LOG_F(ERROR, "Argument out of range: {}", e.what());
        std::cerr << "Argument out of range: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Unhandled exception in main: {}", e.what());
        std::cerr << "Unhandled exception in main: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        LOG_F(ERROR, "Unknown exception in main");
        std::cerr << "Unknown exception in main" << std::endl;
        return 1;
    }

    LOG_F(INFO, "Application terminated successfully.");
    return 0;
}