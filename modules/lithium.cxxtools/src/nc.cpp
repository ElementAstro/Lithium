#include <array>
#include <asio.hpp>
#include <chrono>
#include <fstream>
#include <iostream>


using asio::ip::tcp;
using asio::ip::udp;

constexpr std::size_t MAX_LENGTH = 1024;
constexpr int ARG_COUNT_MIN = 4;
constexpr int ARG_COUNT_MAX = 6;
constexpr int DEFAULT_TIMEOUT_SECONDS = 10;

// Function: Send file over TCP
void sendFileTcp(tcp::socket& socket, const std::string& filename) {
    try {
        std::ifstream file(filename, std::ios::binary);
        if (!file) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return;
        }

        std::array<char, MAX_LENGTH> buffer;
        while (file.read(buffer.data(), buffer.size())) {
            asio::write(socket,
                        asio::buffer(buffer, static_cast<std::streamsize>(
                                                 file.gcount())));
        }
        if (file.gcount() > 0) {
            asio::write(socket,
                        asio::buffer(buffer, static_cast<std::streamsize>(
                                                 file.gcount())));
        }

        std::cout << "File sent: " << filename << std::endl;
    } catch (const std::ios_base::failure& e) {
        std::cerr << "File I/O error: " << e.what() << std::endl;
    } catch (const asio::system_error& e) {
        std::cerr << "ASIO error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error sending file over TCP: " << e.what() << std::endl;
    }
}

// Function: Send file over UDP
void sendFileUdp(udp::socket& socket, const udp::endpoint& endpoint,
                 const std::string& filename) {
    try {
        std::ifstream file(filename, std::ios::binary);
        if (!file) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return;
        }

        std::array<char, MAX_LENGTH> buffer;
        while (file.read(buffer.data(), buffer.size())) {
            socket.send_to(asio::buffer(buffer, static_cast<std::streamsize>(
                                                    file.gcount())),
                           endpoint);
        }
        if (file.gcount() > 0) {
            socket.send_to(asio::buffer(buffer, static_cast<std::streamsize>(
                                                    file.gcount())),
                           endpoint);
        }

        std::cout << "File sent: " << filename << std::endl;
    } catch (const std::ios_base::failure& e) {
        std::cerr << "File I/O error: " << e.what() << std::endl;
    } catch (const asio::system_error& e) {
        std::cerr << "ASIO error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error sending file over UDP: " << e.what() << std::endl;
    }
}

// TCP mode client
void runTcpClient(const std::string& host, const std::string& port,
                  int timeoutSeconds, const std::string& filename = "") {
    try {
        asio::io_context ioContext;
        tcp::resolver resolver(ioContext);
        auto endpoints = resolver.resolve(host, port);
        tcp::socket socket(ioContext);

        asio::connect(socket, endpoints);
        std::cout << "Connected to " << host << ":" << port << std::endl;

        if (!filename.empty()) {
            sendFileTcp(socket, filename);
        } else {
            asio::steady_timer timer(ioContext);
            while (true) {
                std::string message;
                std::getline(std::cin, message);

                if (message.empty()) {
                    break;
                }

                timer.expires_after(std::chrono::seconds(timeoutSeconds));
                timer.async_wait([&socket](const asio::error_code& errorCode) {
                    if (errorCode) {
                        std::cerr << "Timeout error: " << errorCode.message()
                                  << std::endl;
                        socket.close();
                    }
                });

                asio::write(socket, asio::buffer(message + "\n"));

                std::array<char, MAX_LENGTH> reply;
                std::error_code error;
                size_t replyLength =
                    asio::read(socket, asio::buffer(reply), error);

                if (error) {
                    std::cerr << "Read error: " << error.message() << std::endl;
                    break;
                }

                std::cout << "Reply: " << std::string(reply.data(), replyLength)
                          << std::endl;
                timer.cancel();  // Cancel the timer
            }
        }
    } catch (const asio::system_error& e) {
        std::cerr << "ASIO error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown exception occurred in TCP client" << std::endl;
    }
}

// UDP mode client
void runUdpClient(const std::string& host, const std::string& port,
                  const std::string& filename = "") {
    try {
        asio::io_context ioContext;
        udp::resolver resolver(ioContext);
        udp::resolver::results_type endpoints =
            resolver.resolve(udp::v4(), host, port);
        udp::socket socket(ioContext);

        socket.open(udp::v4());

        if (!filename.empty()) {
            sendFileUdp(socket, *endpoints.begin(), filename);
        } else {
            while (true) {
                std::string message;
                std::getline(std::cin, message);

                if (message.empty()) {
                    break;
                }

                socket.send_to(asio::buffer(message), *endpoints.begin());

                std::array<char, MAX_LENGTH> reply;
                udp::endpoint senderEndpoint;
                std::error_code error;
                size_t replyLength = socket.receive_from(
                    asio::buffer(reply), senderEndpoint, 0, error);

                if (error && error != asio::error::message_size) {
                    std::cerr << "Receive error: " << error.message()
                              << std::endl;
                    break;
                }

                std::cout << "Reply from " << senderEndpoint << ": "
                          << std::string(reply.data(), replyLength)
                          << std::endl;
            }
        }
    } catch (const asio::system_error& e) {
        std::cerr << "ASIO error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown exception occurred in UDP client" << std::endl;
    }
}

auto main(int argc, char* argv[]) -> int {
    try {
        if (argc < ARG_COUNT_MIN || argc > ARG_COUNT_MAX) {
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
        std::string filename = (argc == 6) ? argv[5] : "";

        if (protocol == "tcp") {
            runTcpClient(host, port, timeoutSeconds, filename);
        } else if (protocol == "udp") {
            runUdpClient(host, port, filename);
        } else {
            std::cerr << "Unknown protocol: " << protocol << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Unhandled exception in main: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown exception in main" << std::endl;
    }

    return 0;
}
