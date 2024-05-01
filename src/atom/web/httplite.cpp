/*
 * httplite.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-3

Description: Simple Http Client

**************************************************/

#include "httplite.hpp"

#include <openssl/err.h>
#include <openssl/ssl.h>

#include "atom/log/loguru.hpp"

namespace atom::web {

HttpClient::HttpClient() : socketfd(0) {}

HttpClient::~HttpClient() { closeSocket(); }

void HttpClient::setErrorHandler(
    std::function<void(const std::string &)> errorHandler) {
    this->errorHandler = errorHandler;
}

bool HttpClient::initialize() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        errorHandler("Failed to initialize Winsock");
        return false;
    }
#endif
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd == -1) {
        errorHandler("Failed to create socket");
        return false;
    }
    return true;
}

bool HttpClient::connectToServer(const std::string &host, int port,
                                 bool useHttps) {
    struct sockaddr_in serverAddr {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

#ifdef _WIN32
    if (InetPton(AF_INET, host.c_str(), &serverAddr.sin_addr) <= 0)
#else
    if (inet_pton(AF_INET, host.c_str(), &(serverAddr.sin_addr)) <= 0)
#endif
    {
        errorHandler("Failed to parse server address");
        closeSocket();
        return false;
    }

    if (connect(socketfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) <
        0) {
        errorHandler("Failed to connect to server");
        closeSocket();
        return false;
    }

    if (useHttps) {
        SSL_load_error_strings();
        SSL_library_init();
        SSL_CTX *ctx = SSL_CTX_new(SSLv23_client_method());
        SSL *ssl = SSL_new(ctx);
        SSL_set_fd(ssl, socketfd);
        if (SSL_connect(ssl) != 1) {
            errorHandler("Failed to establish SSL connection");
            closeSocket();
            return false;
        }
    }

    return true;
}

bool HttpClient::sendRequest(const std::string &request) {
    if (send(socketfd, request.c_str(), request.length(), 0) < 0) {
        errorHandler("Failed to send request");
        closeSocket();
        return false;
    }
    return true;
}

HttpResponse HttpClient::receiveResponse() {
    HttpResponse response;

    char buffer[4096];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytesRead = recv(socketfd, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead <= 0) {
            break;
        }
        response.body += buffer;
    }

    closeSocket();
    return response;
}

void HttpClient::closeSocket() {
    if (socketfd != 0) {
#ifdef _WIN32
        closesocket(socketfd);
        WSACleanup();
#else
        close(socketfd);
#endif
        socketfd = 0;
    }
}

HttpRequestBuilder::HttpRequestBuilder(HttpMethod method,
                                       const std::string &url)
    : method(method), url(url), timeout(10) {}

HttpRequestBuilder &HttpRequestBuilder::setBody(const std::string &bodyText) {
    body = bodyText;
    return *this;
}

HttpRequestBuilder &HttpRequestBuilder::setContentType(
    const std::string &contentTypeValue) {
    contentType = contentTypeValue;
    return *this;
}

HttpRequestBuilder &HttpRequestBuilder::setTimeout(
    std::chrono::seconds timeoutValue) {
    timeout = timeoutValue;
    return *this;
}

HttpRequestBuilder &HttpRequestBuilder::addHeader(const std::string &key,
                                                  const std::string &value) {
    headers[key] = value;
    return *this;
}

HttpResponse HttpRequestBuilder::send() {
    HttpClient client;
    client.setErrorHandler([](const std::string &error) {
        std::cerr << "Error: " << error << std::endl;
    });

    if (!client.initialize()) {
        return HttpResponse{};
    }

    std::string host;
    std::string path;
    bool useHttps = false;
    size_t pos = url.find("://");
    if (pos != std::string::npos) {
        std::string protocol = url.substr(0, pos);
        if (protocol == "https") {
            useHttps = true;
        }
        pos += 3;
        size_t slashPos = url.find('/', pos);
        if (slashPos != std::string::npos) {
            host = url.substr(pos, slashPos - pos);
            path = url.substr(slashPos);
        } else {
            std::cerr << "Invalid URL" << std::endl;
            return HttpResponse{};
        }
    } else {
        std::cerr << "Invalid URL" << std::endl;
        return HttpResponse{};
    }

    if (!client.connectToServer(host, useHttps ? 443 : 80, useHttps)) {
        return HttpResponse{};
    }

    std::string request = buildRequestString(host, path);
    if (!client.sendRequest(request)) {
        return HttpResponse{};
    }

    return client.receiveResponse();
}

std::string HttpRequestBuilder::buildRequestString(const std::string &host,
                                                   const std::string &path) {
    std::string request;
    switch (method) {
        case HttpMethod::GET:
            request = "GET ";
            break;
        case HttpMethod::POST:
            request = "POST ";
            break;
        case HttpMethod::PUT:
            request = "PUT ";
            break;
        case HttpMethod::DELETE:
            request = "DELETE ";
            break;
    }
    request += path + " HTTP/1.1\r\n";
    request += "Host: " + host + "\r\n";
    for (const auto &header : headers) {
        request += header.first + ": " + header.second + "\r\n";
    }
    if (!contentType.empty()) {
        request += "Content-Type: " + contentType + "\r\n";
    }
    if (!body.empty()) {
        request += "Content-Length: " + std::to_string(body.length()) + "\r\n";
    }
    request += "Connection: close\r\n\r\n";
    if (!body.empty()) {
        request += body;
    }

    return request;
}
}  // namespace atom::web
