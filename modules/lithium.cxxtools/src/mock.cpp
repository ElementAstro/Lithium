// mock.cpp
#include <asio.hpp>
#include <asio/ssl.hpp>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>

#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"

using asio::ip::tcp;
using json = nlohmann::json;
namespace fs = std::filesystem;

class MockServer : public std::enable_shared_from_this<MockServer> {
public:
    MockServer(asio::io_context& ioContext, unsigned short port,
               const fs::path& configFile)
        : acceptor_(ioContext, tcp::endpoint(tcp::v4(), port)),
          sslContext_(asio::ssl::context::tlsv12_server) {
        loadConfig(configFile);
        configureSSL();
        startAccept();
        LOG_F(INFO, "MockServer initialized on port {}", port);
    }

private:
    struct Endpoint {
        std::string method;
        int code;
        std::string body;
        int delay_ms;  // milliseconds
        std::unordered_map<std::string, std::string> headers;
    };

    struct Response {
        std::string text;
        int delay_ms;  // milliseconds
    };

    void loadConfig(const fs::path& configFile) {
        std::ifstream file(configFile);
        if (!file) {
            LOG_F(ERROR, "Failed to open configuration file: {}",
                  configFile.string());
            throw std::runtime_error("Failed to open configuration file");
        }

        json configJson;
        try {
            file >> configJson;
        } catch (const json::parse_error& e) {
            LOG_F(ERROR, "JSON parse error in configuration file {}: {}",
                  configFile.string(), e.what());
            throw;
        }

        if (!configJson.contains("endpoints") ||
            !configJson["endpoints"].is_array()) {
            LOG_F(ERROR,
                  "Invalid configuration file: 'endpoints' array missing or "
                  "not an array");
            throw std::runtime_error("Invalid configuration file structure");
        }

        for (const auto& endpointJson : configJson["endpoints"]) {
            if (!endpointJson.contains("path") ||
                !endpointJson.contains("request_method") ||
                !endpointJson.contains("response_code") ||
                !endpointJson.contains("response_body")) {
                LOG_F(WARNING, "Endpoint missing required fields: {}",
                      endpointJson.dump());
                continue;  // Skip invalid endpoint
            }

            Endpoint endpoint;
            endpoint.method = endpointJson["request_method"].get<std::string>();
            endpoint.code = endpointJson["response_code"].get<int>();
            endpoint.body = endpointJson["response_body"].get<std::string>();
            endpoint.delay_ms = endpointJson.value("response_delay_ms", 0);

            if (endpointJson.contains("headers") &&
                endpointJson["headers"].is_object()) {
                for (auto& [key, value] : endpointJson["headers"].items()) {
                    if (value.is_string()) {
                        endpoint.headers.emplace(key, value.get<std::string>());
                    }
                }
            }

            std::string path = endpointJson["path"].get<std::string>();
            endpoints_.emplace(std::move(path), std::move(endpoint));
            LOG_F(INFO, "Loaded endpoint: {} {}", endpoint.method, path);
        }
    }

    void configureSSL() {
        try {
            sslContext_.set_options(asio::ssl::context::default_workarounds |
                                    asio::ssl::context::no_sslv2 |
                                    asio::ssl::context::single_dh_use);

            // Use a self-signed certificate and private key for demonstration
            // purposes
            sslContext_.use_certificate_chain_file("server.crt");
            sslContext_.use_private_key_file("server.key",
                                             asio::ssl::context::pem);

            LOG_F(INFO, "SSL context configured successfully.");
        } catch (const std::exception& e) {
            LOG_F(ERROR, "Failed to configure SSL context: {}", e.what());
            throw;
        }
    }

    void startAccept() {
        auto sslSocket = std::make_shared<asio::ssl::stream<tcp::socket>>(
            acceptor_.get_executor().context(), sslContext_);

        acceptor_.async_accept(
            sslSocket->lowest_layer(), [this, sslSocket](std::error_code ec) {
                if (!ec) {
                    try {
                        auto remoteEndpoint =
                            sslSocket->lowest_layer().remote_endpoint();
                        LOG_F(INFO, "Accepted connection from {}",
                              remoteEndpoint);
                    } catch (const std::exception& e) {
                        LOG_F(WARNING, "Failed to get remote endpoint: {}",
                              e.what());
                    }

                    sslSocket->async_handshake(
                        asio::ssl::stream_base::server,
                        [this, sslSocket](const std::error_code& handshake_ec) {
                            if (!handshake_ec) {
                                try {
                                    auto remoteEndpoint =
                                        sslSocket->lowest_layer()
                                            .remote_endpoint();
                                    LOG_F(INFO,
                                          "SSL handshake successful with {}",
                                          remoteEndpoint);
                                } catch (const std::exception& e) {
                                    LOG_F(WARNING,
                                          "Failed to get remote endpoint after "
                                          "handshake: {}",
                                          e.what());
                                }
                                handleClient(sslSocket);
                            } else {
                                LOG_F(ERROR, "SSL handshake failed: {}",
                                      handshake_ec.message());
                            }
                        });
                } else {
                    LOG_F(ERROR, "Accept error: {}", ec.message());
                }
                // Continue accepting new connections
                startAccept();
            });
    }

    void handleClient(
        std::shared_ptr<asio::ssl::stream<tcp::socket>> sslSocket) {
        auto self = shared_from_this();
        auto buffer = std::make_shared<asio::streambuf>();

        asio::async_read_until(
            *sslSocket, *buffer, "\r\n\r\n",
            [this, self, sslSocket, buffer](std::error_code ec,
                                            std::size_t bytes_transferred) {
                if (!ec) {
                    std::istream requestStream(buffer.get());
                    std::string requestLine;
                    std::getline(requestStream, requestLine);
                    if (!requestStream) {
                        LOG_F(WARNING, "Failed to read request line.");
                        return;
                    }

                    // Remove possible '\r' at the end of request line
                    if (!requestLine.empty() && requestLine.back() == '\r') {
                        requestLine.pop_back();
                    }

                    std::istringstream requestLineStream(requestLine);
                    std::string method, path, protocol;
                    requestLineStream >> method >> path >> protocol;
                    LOG_F(INFO,
                          "Received request: Method={}, Path={}, Protocol={}",
                          method, path, protocol);

                    // Consume remaining headers
                    std::string header;
                    while (std::getline(requestStream, header) &&
                           header != "\r") {
                        // Log each header
                        LOG_F(INFO, "Header: {}", header);
                    }

                    auto response = handleRequest(method, path);
                    LOG_F(INFO, "Prepared response for {} {}", method, path);

                    if (response.delay_ms > 0) {
                        LOG_F(INFO, "Delaying response by {} ms",
                              response.delay_ms);
                        std::this_thread::sleep_for(
                            std::chrono::milliseconds(response.delay_ms));
                    }

                    asio::async_write(
                        *sslSocket, asio::buffer(response.text),
                        [sslSocket, self](std::error_code write_ec,
                                          std::size_t bytes_written) {
                            if (!write_ec) {
                                try {
                                    auto remoteEndpoint =
                                        sslSocket->lowest_layer()
                                            .remote_endpoint();
                                    LOG_F(INFO,
                                          "Sent {} bytes in response to {}",
                                          bytes_written, remoteEndpoint);
                                } catch (const std::exception& e) {
                                    LOG_F(WARNING,
                                          "Failed to get remote endpoint after "
                                          "write: {}",
                                          e.what());
                                }
                            } else {
                                LOG_F(ERROR, "Failed to send response: {}",
                                      write_ec.message());
                            }

                            // Gracefully shutdown the connection
                            sslSocket->async_shutdown(
                                [sslSocket,
                                 self](const std::error_code& shutdown_ec) {
                                    if (!shutdown_ec) {
                                        try {
                                            auto remoteEndpoint =
                                                sslSocket->lowest_layer()
                                                    .remote_endpoint();
                                            LOG_F(INFO,
                                                  "Connection with {} closed "
                                                  "gracefully.",
                                                  remoteEndpoint);
                                        } catch (const std::exception& e) {
                                            LOG_F(WARNING,
                                                  "Failed to get remote "
                                                  "endpoint after shutdown: {}",
                                                  e.what());
                                        }
                                    } else {
                                        LOG_F(ERROR, "Shutdown failed: {}",
                                              shutdown_ec.message());
                                    }
                                });
                        });
                } else {
                    LOG_F(ERROR, "Error reading request: {}", ec.message());
                }
            });
    }

    Response handleRequest(const std::string& method,
                           const std::string& path) const {
        auto it = endpoints_.find(path);
        if (it != endpoints_.end() && it->second.method == method) {
            LOG_F(INFO, "Handling configured endpoint: {} {}", method, path);
            return generateResponse(it->second);
        }

        // Serve static files by default
        fs::path staticFileDir = "static";
        fs::path requestedPath = path.starts_with('/')
                                     ? path.substr(1)
                                     : path;  // Remove leading '/' if present
        fs::path filePath = fs::canonical(staticFileDir) / requestedPath;

        // Prevent path traversal
        try {
            fs::path canonicalStaticDir = fs::canonical(staticFileDir);
            fs::path canonicalFilePath = fs::canonical(filePath);

            if (canonicalFilePath.string().find(canonicalStaticDir.string()) !=
                0) {
                LOG_F(WARNING, "Path traversal attempt detected: {}", path);
                std::string forbiddenBody = "403 Forbidden";
                std::ostringstream forbiddenResponse;
                forbiddenResponse << "HTTP/1.1 403 Forbidden\r\n"
                                  << "Content-Type: text/plain\r\n"
                                  << "Content-Length: " << forbiddenBody.size()
                                  << "\r\n\r\n"
                                  << forbiddenBody;
                return {forbiddenResponse.str(), 0};
            }
        } catch (const fs::filesystem_error& e) {
            LOG_F(ERROR, "Filesystem error while resolving path: {}", e.what());
            // Return 500 Internal Server Error
            std::string serverErrorBody = "500 Internal Server Error";
            std::ostringstream serverErrorResponse;
            serverErrorResponse << "HTTP/1.1 500 Internal Server Error\r\n"
                                << "Content-Type: text/plain\r\n"
                                << "Content-Length: " << serverErrorBody.size()
                                << "\r\n\r\n"
                                << serverErrorBody;
            return {serverErrorResponse.str(), 0};
        }

        if (fs::exists(filePath) && fs::is_regular_file(filePath)) {
            LOG_F(INFO, "Serving static file: {}", filePath.string());
            return serveStaticFile(filePath);
        }

        // Default 404 Not Found response
        LOG_F(INFO, "Endpoint not found: {} {}, returning 404", method, path);
        std::string notFoundBody = "404 Not Found";
        std::ostringstream notFoundResponse;
        notFoundResponse << "HTTP/1.1 404 Not Found\r\n"
                         << "Content-Type: text/plain\r\n"
                         << "Content-Length: " << notFoundBody.size()
                         << "\r\n\r\n"
                         << notFoundBody;
        return {notFoundResponse.str(), 0};
    }

    Response generateResponse(const Endpoint& endpoint) const {
        std::ostringstream response;
        response << "HTTP/1.1 " << endpoint.code << " ";

        // Map of common HTTP status codes and reasons
        static const std::unordered_map<int, std::string> statusReasons = {
            {200, "OK"},
            {201, "Created"},
            {400, "Bad Request"},
            {401, "Unauthorized"},
            {403, "Forbidden"},
            {404, "Not Found"},
            {500, "Internal Server Error"}
            // Add more as needed
        };

        auto reason_it = statusReasons.find(endpoint.code);
        if (reason_it != statusReasons.end()) {
            response << reason_it->second;
        } else {
            response << "Status";
        }

        response << "\r\n";

        for (const auto& [key, value] : endpoint.headers) {
            response << key << ": " << value << "\r\n";
        }

        response << "Content-Length: " << endpoint.body.size() << "\r\n\r\n"
                 << endpoint.body;

        LOG_F(INFO, "Generated response: {}", response.str());

        return {response.str(), endpoint.delay_ms};
    }

    Response serveStaticFile(const fs::path& filePath) const {
        std::ifstream file(filePath, std::ios::binary);
        if (!file) {
            LOG_F(ERROR, "Failed to open static file: {}", filePath.string());

            std::string serverErrorBody = "500 Internal Server Error";
            std::ostringstream serverErrorResponse;
            serverErrorResponse << "HTTP/1.1 500 Internal Server Error\r\n"
                                << "Content-Type: text/plain\r\n"
                                << "Content-Length: " << serverErrorBody.size()
                                << "\r\n\r\n"
                                << serverErrorBody;
            return {serverErrorResponse.str(), 0};
        }

        std::ostringstream bodyStream;
        bodyStream << file.rdbuf();
        std::string content = bodyStream.str();

        std::ostringstream response;
        response << "HTTP/1.1 200 OK\r\n"
                 << "Content-Type: application/octet-stream\r\n"
                 << "Content-Length: " << content.size() << "\r\n\r\n"
                 << content;

        LOG_F(INFO, "Serving static file response: {}", filePath.string());

        return {response.str(), 0};
    }

    tcp::acceptor acceptor_;
    asio::ssl::context sslContext_;
    std::unordered_map<std::string, Endpoint> endpoints_;
    mutable std::mutex logMutex_;
};

int main(int argc, char* argv[]) {
    loguru::init(argc, argv);
    LOG_F(INFO, "MockServer application started.");

    if (argc != 3) {
        LOG_F(ERROR, "Invalid number of arguments.");
        std::cerr << "Usage: " << argv[0] << " <port> <config_file>"
                  << std::endl;
        return 1;
    }

    try {
        unsigned short port = 0;
        try {
            port = static_cast<unsigned short>(std::stoul(argv[1]));
        } catch (const std::exception& e) {
            LOG_F(ERROR, "Invalid port number '{}': {}", argv[1], e.what());
            std::cerr << "Invalid port number: " << argv[1] << std::endl;
            return 1;
        }

        fs::path configFile = argv[2];
        if (!fs::exists(configFile)) {
            LOG_F(ERROR, "Configuration file does not exist: {}",
                  configFile.string());
            std::cerr << "Configuration file does not exist: " << configFile
                      << std::endl;
            return 1;
        }

        asio::io_context ioContext;

        auto server = std::make_shared<MockServer>(ioContext, port, configFile);
        LOG_F(INFO, "Starting IO context.");
        ioContext.run();
        LOG_F(INFO, "IO context stopped.");
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception in main: {}", e.what());
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        LOG_F(ERROR, "Unknown exception in main.");
        std::cerr << "Unknown exception." << std::endl;
        return 1;
    }

    LOG_F(INFO, "MockServer application terminated gracefully.");
    return 0;
}