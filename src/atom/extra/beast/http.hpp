#ifndef HTTP_CLIENT_HPP
#define HTTP_CLIENT_HPP

#include <atomic>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;
using json = nlohmann::json;

class HttpClient {
public:
    /**
     * @brief Constructs an HttpClient with the given I/O context.
     * @param ioc The I/O context to use for asynchronous operations.
     */
    explicit HttpClient(net::io_context& ioc);

    /**
     * @brief Sets a default header for all requests.
     * @param key The header key.
     * @param value The header value.
     */
    void setDefaultHeader(const std::string& key, const std::string& value);

    /**
     * @brief Sets the timeout duration for the HTTP operations.
     * @param timeout The timeout duration in seconds.
     */
    void setTimeout(std::chrono::seconds timeout);

    /**
     * @brief Sends a synchronous HTTP request.
     * @tparam Body The type of the request body.
     * @param method The HTTP method (verb).
     * @param host The server host.
     * @param port The server port.
     * @param target The target URI.
     * @param version The HTTP version (default is 11).
     * @param content_type The content type of the request body.
     * @param body The request body.
     * @param headers Additional headers to include in the request.
     * @return The HTTP response.
     */
    template <class Body = http::string_body>
    auto request(http::verb method, const std::string& host,
                 const std::string& port, const std::string& target,
                 int version = 11, const std::string& content_type = "",
                 const std::string& body = "",
                 const std::unordered_map<std::string, std::string>& headers =
                     {}) -> http::response<Body>;

    /**
     * @brief Sends an asynchronous HTTP request.
     * @tparam Body The type of the request body.
     * @tparam ResponseHandler The type of the handler to call when the
     * operation completes.
     * @param method The HTTP method (verb).
     * @param host The server host.
     * @param port The server port.
     * @param target The target URI.
     * @param handler The handler to call when the operation completes.
     * @param version The HTTP version (default is 11).
     * @param content_type The content type of the request body.
     * @param body The request body.
     * @param headers Additional headers to include in the request.
     */
    template <class Body = http::string_body, class ResponseHandler>
    void asyncRequest(
        http::verb method, const std::string& host, const std::string& port,
        const std::string& target, ResponseHandler&& handler, int version = 11,
        const std::string& content_type = "", const std::string& body = "",
        const std::unordered_map<std::string, std::string>& headers = {});

    /**
     * @brief Sends a synchronous HTTP request with a JSON body and returns a
     * JSON response.
     * @param method The HTTP method (verb).
     * @param host The server host.
     * @param port The server port.
     * @param target The target URI.
     * @param json_body The JSON body of the request.
     * @param headers Additional headers to include in the request.
     * @return The JSON response.
     */
    auto jsonRequest(http::verb method, const std::string& host,
                     const std::string& port, const std::string& target,
                     const json& json_body = {},
                     const std::unordered_map<std::string, std::string>&
                         headers = {}) -> json;

    /**
     * @brief Sends an asynchronous HTTP request with a JSON body and returns a
     * JSON response.
     * @tparam ResponseHandler The type of the handler to call when the
     * operation completes.
     * @param method The HTTP method (verb).
     * @param host The server host.
     * @param port The server port.
     * @param target The target URI.
     * @param handler The handler to call when the operation completes.
     * @param json_body The JSON body of the request.
     * @param headers Additional headers to include in the request.
     */
    template <class ResponseHandler>
    void asyncJsonRequest(
        http::verb method, const std::string& host, const std::string& port,
        const std::string& target, ResponseHandler&& handler,
        const json& json_body = {},
        const std::unordered_map<std::string, std::string>& headers = {});

    /**
     * @brief Uploads a file to the server.
     * @param host The server host.
     * @param port The server port.
     * @param target The target URI.
     * @param filepath The path to the file to upload.
     * @param field_name The field name for the file (default is "file").
     * @return The HTTP response.
     */
    auto uploadFile(const std::string& host, const std::string& port,
                    const std::string& target, const std::string& filepath,
                    const std::string& field_name = "file")
        -> http::response<http::string_body>;

    /**
     * @brief Downloads a file from the server.
     * @param host The server host.
     * @param port The server port.
     * @param target The target URI.
     * @param filepath The path to save the downloaded file.
     */
    void downloadFile(const std::string& host, const std::string& port,
                      const std::string& target, const std::string& filepath);

    /**
     * @brief Sends a synchronous HTTP request with retry logic.
     * @tparam Body The type of the request body.
     * @param method The HTTP method (verb).
     * @param host The server host.
     * @param port The server port.
     * @param target The target URI.
     * @param retry_count The number of retry attempts (default is 3).
     * @param version The HTTP version (default is 11).
     * @param content_type The content type of the request body.
     * @param body The request body.
     * @param headers Additional headers to include in the request.
     * @return The HTTP response.
     */
    template <class Body = http::string_body>
    auto requestWithRetry(
        http::verb method, const std::string& host, const std::string& port,
        const std::string& target, int retry_count = 3, int version = 11,
        const std::string& content_type = "", const std::string& body = "",
        const std::unordered_map<std::string, std::string>& headers = {})
        -> http::response<Body>;

    /**
     * @brief Sends multiple synchronous HTTP requests in a batch.
     * @tparam Body The type of the request body.
     * @param requests A vector of tuples containing the HTTP method, host,
     * port, and target for each request.
     * @param headers Additional headers to include in each request.
     * @return A vector of HTTP responses.
     */
    template <class Body = http::string_body>
    std::vector<http::response<Body>> batchRequest(
        const std::vector<std::tuple<http::verb, std::string, std::string,
                                     std::string>>& requests,
        const std::unordered_map<std::string, std::string>& headers = {});

    /**
     * @brief Sends multiple asynchronous HTTP requests in a batch.
     * @tparam ResponseHandler The type of the handler to call when the
     * operation completes.
     * @param requests A vector of tuples containing the HTTP method, host,
     * port, and target for each request.
     * @param handler The handler to call when the operation completes.
     * @param headers Additional headers to include in each request.
     */
    template <class ResponseHandler>
    void asyncBatchRequest(
        const std::vector<std::tuple<http::verb, std::string, std::string,
                                     std::string>>& requests,
        ResponseHandler&& handler,
        const std::unordered_map<std::string, std::string>& headers = {});

    /**
     * @brief Runs the I/O context with a thread pool.
     * @param num_threads The number of threads in the pool.
     */
    void runWithThreadPool(size_t num_threads);

    /**
     * @brief Asynchronously downloads a file from the server.
     * @tparam ResponseHandler The type of the handler to call when the
     * operation completes.
     * @param host The server host.
     * @param port The server port.
     * @param target The target URI.
     * @param filepath The path to save the downloaded file.
     * @param handler The handler to call when the operation completes.
     */
    template <class ResponseHandler>
    void asyncDownloadFile(const std::string& host, const std::string& port,
                           const std::string& target,
                           const std::string& filepath,
                           ResponseHandler&& handler);

private:
    tcp::resolver resolver_;    ///< The resolver for DNS lookups.
    beast::tcp_stream stream_;  ///< The TCP stream for HTTP communication.
    std::unordered_map<std::string, std::string>
        default_headers_;  ///< Default headers for all requests.
    std::chrono::seconds timeout_{
        30};  ///< The timeout duration for HTTP operations.
};

template <class Body>
auto HttpClient::request(http::verb method, const std::string& host,
                         const std::string& port, const std::string& target,
                         int version, const std::string& content_type,
                         const std::string& body,
                         const std::unordered_map<std::string, std::string>&
                             headers) -> http::response<Body> {
    http::request<http::string_body> req{method, target, version};
    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    for (const auto& [key, value] : default_headers_) {
        req.set(key, value);
    }

    for (const auto& [key, value] : headers) {
        req.set(key, value);
    }

    if (!content_type.empty()) {
        req.set(http::field::content_type, content_type);
    }

    if (!body.empty()) {
        req.body() = body;
        req.prepare_payload();
    }

    auto const results = resolver_.resolve(host, port);
    stream_.connect(results);

    stream_.expires_after(timeout_);

    http::write(stream_, req);

    beast::flat_buffer buffer;
    http::response<Body> res;
    http::read(stream_, buffer, res);

    beast::error_code ec;
    stream_.socket().shutdown(tcp::socket::shutdown_both, ec);

    return res;
}

template <class Body, class ResponseHandler>
void HttpClient::asyncRequest(
    http::verb method, const std::string& host, const std::string& port,
    const std::string& target, ResponseHandler&& handler, int version,
    const std::string& content_type, const std::string& body,
    const std::unordered_map<std::string, std::string>& headers) {
    auto req = std::make_shared<http::request<http::string_body>>(
        method, target, version);
    req->set(http::field::host, host);
    req->set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    for (const auto& [key, value] : default_headers_) {
        req->set(key, value);
    }

    for (const auto& [key, value] : headers) {
        req->set(key, value);
    }

    if (!content_type.empty()) {
        req->set(http::field::content_type, content_type);
    }

    if (!body.empty()) {
        req->body() = body;
        req->prepare_payload();
    }

    resolver_.async_resolve(
        host, port,
        [this, req, handler = std::forward<ResponseHandler>(handler)](
            beast::error_code ec, tcp::resolver::results_type results) {
            if (ec) {
                return handler(ec, {});
            }

            stream_.async_connect(
                results, [this, req, handler = std::move(handler)](
                             beast::error_code ec,
                             tcp::resolver::results_type::endpoint_type) {
                    if (ec) {
                        return handler(ec, {});
                    }

                    stream_.expires_after(timeout_);

                    http::async_write(
                        stream_, *req,
                        [this, req, handler = std::move(handler)](
                            beast::error_code ec, std::size_t) {
                            if (ec) {
                                return handler(ec, {});
                            }

                            auto res = std::make_shared<http::response<Body>>();
                            auto buffer =
                                std::make_shared<beast::flat_buffer>();

                            http::async_read(
                                stream_, *buffer, *res,
                                [this, res, buffer,
                                 handler = std::move(handler)](
                                    beast::error_code ec, std::size_t) {
                                    stream_.socket().shutdown(
                                        tcp::socket::shutdown_both, ec);
                                    handler(ec, std::move(*res));
                                });
                        });
                });
        });
}

template <class ResponseHandler>
void HttpClient::asyncJsonRequest(
    http::verb method, const std::string& host, const std::string& port,
    const std::string& target, ResponseHandler&& handler, const json& json_body,
    const std::unordered_map<std::string, std::string>& headers) {
    asyncRequest<http::string_body>(
        method, host, port, target,
        [handler = std::forward<ResponseHandler>(handler)](
            beast::error_code ec, http::response<http::string_body> res) {
            if (ec) {
                handler(ec, {});
            } else {
                try {
                    auto jv = json::parse(res.body());
                    handler({}, std::move(jv));
                } catch (const json::parse_error& e) {
                    handler(beast::error_code{e.id, beast::generic_category()},
                            {});
                }
            }
        },
        11, "application/json", json_body.empty() ? "" : json_body.dump(),
        headers);
}

template <class Body>
auto HttpClient::requestWithRetry(
    http::verb method, const std::string& host, const std::string& port,
    const std::string& target, int retry_count, int version,
    const std::string& content_type, const std::string& body,
    const std::unordered_map<std::string, std::string>& headers)
    -> http::response<Body> {
    beast::error_code ec;
    http::response<Body> response;
    for (int attempt = 0; attempt < retry_count; ++attempt) {
        try {
            response = request<Body>(method, host, port, target, version,
                                     content_type, body, headers);
            // If no exception was thrown, return the response
            return response;
        } catch (const beast::system_error& e) {
            ec = e.code();
            std::cerr << "Request attempt " << (attempt + 1)
                      << " failed: " << ec.message() << std::endl;
            if (attempt + 1 == retry_count) {
                throw;  // Throw the exception if this was the last retry
            }
        }
    }
    return response;
}

template <class Body>
std::vector<http::response<Body>> HttpClient::batchRequest(
    const std::vector<std::tuple<http::verb, std::string, std::string,
                                 std::string>>& requests,
    const std::unordered_map<std::string, std::string>& headers) {
    std::vector<http::response<Body>> responses;
    for (const auto& [method, host, port, target] : requests) {
        try {
            responses.push_back(
                request<Body>(method, host, port, target, 11, "", "", headers));
        } catch (const std::exception& e) {
            std::cerr << "Batch request failed for " << target << ": "
                      << e.what() << std::endl;
            // Push an empty response if an exception occurs (or handle as
            // needed)
            responses.emplace_back();
        }
    }
    return responses;
}

template <class ResponseHandler>
void HttpClient::asyncBatchRequest(
    const std::vector<std::tuple<http::verb, std::string, std::string,
                                 std::string>>& requests,
    ResponseHandler&& handler,
    const std::unordered_map<std::string, std::string>& headers) {
    auto responses =
        std::make_shared<std::vector<http::response<http::string_body>>>();
    auto remaining = std::make_shared<std::atomic<int>>(requests.size());

    for (const auto& [method, host, port, target] : requests) {
        asyncRequest<http::string_body>(
            method, host, port, target,
            [handler, responses, remaining](
                beast::error_code ec, http::response<http::string_body> res) {
                if (ec) {
                    std::cerr << "Error during batch request: " << ec.message()
                              << std::endl;
                    responses
                        ->emplace_back();  // Empty response in case of error
                } else {
                    responses->emplace_back(std::move(res));
                }

                if (--(*remaining) == 0) {
                    handler(*responses);
                }
            },
            11, "", "", headers);
    }
}

template <class ResponseHandler>
void HttpClient::asyncDownloadFile(const std::string& host,
                                   const std::string& port,
                                   const std::string& target,
                                   const std::string& filepath,
                                   ResponseHandler&& handler) {
    asyncRequest<http::string_body>(
        http::verb::get, host, port, target,
        [filepath, handler = std::forward<ResponseHandler>(handler)](
            beast::error_code ec, http::response<http::string_body> res) {
            if (ec) {
                handler(ec, false);
            } else {
                std::ofstream outFile(filepath, std::ios::binary);
                if (!outFile) {
                    std::cerr << "Failed to open file for writing: " << filepath
                              << std::endl;
                    handler(beast::error_code{}, false);
                    return;
                }
                outFile << res.body();
                handler({}, true);  // Download successful
            }
        });
}

#endif  // HTTP_CLIENT_HPP
