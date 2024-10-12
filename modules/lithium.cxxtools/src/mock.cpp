#include <asio.hpp>
#include <asio/ssl.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>
#include <unordered_map>

using asio::ip::tcp;
using json = nlohmann::json;
namespace fs = std::filesystem;

class MockServer : public std::enable_shared_from_this<MockServer> {
public:
    MockServer(asio::io_context& ioContext, short port,
               const std::string& configFile)
        : acceptor_(ioContext, tcp::endpoint(tcp::v4(), port)) {
        loadConfig(configFile);
        configureSSL();
        accept();
    }

private:
    struct Endpoint {
        std::string method;
        int code;
        std::string body;
        int delay;
        std::unordered_map<std::string, std::string> headers;
    };

    void loadConfig(const std::string& configFile) {
        std::ifstream file(configFile);
        if (!file) {
            std::cerr << "Failed to open configuration file: " << configFile
                      << std::endl;
            return;
        }

        json configJson;
        file >> configJson;

        for (const auto& endpoint : configJson["endpoints"]) {
            std::string path = endpoint["path"];
            std::string method = endpoint["request_method"];
            int code = endpoint["response_code"];
            std::string body = endpoint["response_body"];
            int delay = endpoint.value("response_delay_ms", 0);
            std::unordered_map<std::string, std::string> headers;

            if (endpoint.contains("headers")) {
                headers =
                    endpoint["headers"]
                        .get<std::unordered_map<std::string, std::string>>();
            }

            endpoints_[path] = {method, code, body, delay, headers};
        }
    }

    void configureSSL() {
        sslContext_.set_options(asio::ssl::context::default_workarounds |
                                asio::ssl::context::no_sslv2 |
                                asio::ssl::context::single_dh_use);

        // Use a self-signed certificate and private key for demonstration
        // purposes
        sslContext_.use_certificate_chain_file("server.crt");
        sslContext_.use_private_key_file("server.key", asio::ssl::context::pem);
    }

    void accept() {
        sslSocket_ = std::make_shared<asio::ssl::stream<tcp::socket>>(
            acceptor_.get_executor().context(), sslContext_);
        acceptor_.async_accept(
            sslSocket_->lowest_layer(), [this](std::error_code errorCode) {
                if (!errorCode) {
                    sslSocket_->async_handshake(
                        asio::ssl::stream_base::server,
                        [this](const std::error_code& error) {
                            if (!error) {
                                handleClient(sslSocket_);
                            }
                        });
                }
                accept();
            });
    }

    void handleClient(
        std::shared_ptr<asio::ssl::stream<tcp::socket>> sslSocket) {
        auto self(shared_from_this());
        asio::async_read_until(
            *sslSocket, asio::dynamic_buffer(request_), "\r\n\r\n",
            [this, self, sslSocket](std::error_code errorCode,
                                    std::size_t /*length*/) {
                if (!errorCode) {
                    std::istringstream requestStream(request_);
                    std::string method;
                    std::string path;
                    std::string protocol;
                    requestStream >> method >> path >> protocol;
                    logRequest(method, path);

                    auto response = handleRequest(method, path, requestStream);
                    std::this_thread::sleep_for(
                        std::chrono::milliseconds(response.delay));

                    asio::async_write(
                        *sslSocket, asio::buffer(response.text),
                        [sslSocket](std::error_code ec, std::size_t) {
                            if (!ec) {
                                sslSocket->shutdown();
                            }
                        });
                }
            });
    }

    struct Response {
        std::string text;
        int delay;
    };

    auto handleRequest(const std::string& method, const std::string& path,
                       std::istringstream& /*requestStream*/) -> Response {
        auto endpointIterator = endpoints_.find(path);

        if (endpointIterator != endpoints_.end() &&
            endpointIterator->second.method == method) {
            return generateResponse(endpointIterator->second);
        }

        // Serve static files default path
        std::string staticFileDir = "static";
        if (fs::exists(staticFileDir + path) &&
            fs::is_regular_file(staticFileDir + path)) {
            return serveStaticFile(staticFileDir + path);
        }

        // Default 404 response
        return {
            "HTTP/1.1 404 Not Found\r\nContent-Type: "
            "text/plain\r\nContent-Length: 0\r\n\r\n",
            0};
    }

    static auto generateResponse(const Endpoint& endpoint) -> Response {
        std::ostringstream response;

        response << "HTTP/1.1 " << endpoint.code << " OK\r\n";
        for (const auto& header : endpoint.headers) {
            response << header.first << ": " << header.second << "\r\n";
        }
        response << "Content-Length: " << endpoint.body.size() << "\r\n\r\n";
        response << endpoint.body;

        return {response.str(), endpoint.delay};
    }

    static auto serveStaticFile(const std::string& filePath) -> Response {
        std::ifstream file(filePath, std::ios::binary);

        if (!file) {
            return {
                "HTTP/1.1 500 Internal Server Error\r\nContent-Type: "
                "text/plain\r\nContent-Length: 0\r\n\r\n",
                0};
        }

        std::ostringstream body;
        body << file.rdbuf();
        std::string content = body.str();

        std::ostringstream response;
        response << "HTTP/1.1 200 OK\r\n";
        response << "Content-Type: application/octet-stream\r\n";
        response << "Content-Length: " << content.size() << "\r\n\r\n";
        response << content;

        return {response.str(), 0};
    }

    void logRequest(const std::string& method, const std::string& path) {
        std::cout << "Received request: " << method << " " << path << std::endl;

        std::scoped_lock lock(logMutex_);
        std::ofstream logFile("request_log.txt", std::ios::app);
        logFile << method << " " << path << std::endl;
    }

    tcp::acceptor acceptor_;
    asio::ssl::context sslContext_{asio::ssl::context::tlsv12};
    std::shared_ptr<asio::ssl::stream<tcp::socket>> sslSocket_;
    std::string request_;
    std::unordered_map<std::string, Endpoint> endpoints_;
    std::mutex logMutex_;
};

auto main(int argc, char* argv[]) -> int {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <config_file>"
                  << std::endl;
        return 1;
    }

    try {
        asio::io_context ioContext;
        MockServer server(ioContext,
                          static_cast<short>(std::strtol(argv[1], nullptr, 10)),
                          argv[2]);
        ioContext.run();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}