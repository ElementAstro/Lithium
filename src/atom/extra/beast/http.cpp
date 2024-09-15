#include "http.hpp"

#include <boost/asio/thread_pool.hpp>

HttpClient::HttpClient(net::io_context& ioc)
    : resolver_(net::make_strand(ioc)), stream_(net::make_strand(ioc)) {}

void HttpClient::setDefaultHeader(const std::string& key,
                                  const std::string& value) {
    default_headers_[key] = value;
}

void HttpClient::setTimeout(std::chrono::seconds timeout) {
    timeout_ = timeout;
}

auto HttpClient::uploadFile(
    const std::string& host, const std::string& port, const std::string& target,
    const std::string& filepath,
    const std::string& field_name) -> http::response<http::string_body> {
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open file: " + filepath);
    }
    std::string fileContent((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());

    std::string boundary =
        "-------------------------" + std::to_string(std::time(nullptr));

    std::string body = "--" + boundary + "\r\n";
    body += "Content-Disposition: form-data; name=\"" + field_name +
            "\"; filename=\"" +
            std::filesystem::path(filepath).filename().string() + "\"\r\n";
    body += "Content-Type: application/octet-stream\r\n\r\n";
    body += fileContent + "\r\n";
    body += "--" + boundary + "--\r\n";

    std::string contentType = "multipart/form-data; boundary=" + boundary;

    return request(http::verb::post, host, port, target, 11, contentType, body);
}

void HttpClient::downloadFile(const std::string& host, const std::string& port,
                              const std::string& target,
                              const std::string& filepath) {
    auto res = request(http::verb::get, host, port, target);
    std::ofstream outFile(filepath, std::ios::binary);
    outFile << res.body();
}

void HttpClient::runWithThreadPool(size_t num_threads) {
    net::thread_pool pool(num_threads);

    for (size_t i = 0; i < num_threads; ++i) {
        net::post(pool, [this] {
            // Example task: send a request in a thread from the pool
            auto res = request(http::verb::get, "example.com", "80", "/");
            std::cout << "Response in thread pool: " << res << std::endl;
        });
    }

    pool.join();  // Wait for all threads to finish
}
