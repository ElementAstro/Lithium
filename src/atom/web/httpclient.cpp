/*
 * httpclient.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-25

Description: Http Client

**************************************************/

#include "httpclient.hpp"

#include "cpp_httplib/httplib.h"

#include "atom/log/loguru.hpp"

#if __cplusplus >= 202002L
#include <format>
#endif

using namespace httplib;

namespace atom::web {
class HttpClient::HttpClientImpl {
public:
    explicit HttpClientImpl(const std::string &host, int port,
                            bool ssl_enabled);
    ~HttpClientImpl();

    template <typename RequestT>
    bool sendRequest(const std::string &method, const std::string &path,
                     const std::map<std::string, std::string> &params,
                     const json &data, json &response, std::string &err);

    bool scanPort(int startPort, int endPort, std::vector<int> &openPorts);
    bool checkServerStatus(std::string &status);

    void setSslEnabled(bool enabled);
    void setCaCertPath(const std::string &path);
    void setClientCertPath(const std::string &path);
    void setClientKeyPath(const std::string &path);

private:
    std::string host_;
    int port_;
    bool sslEnabled_;
    std::string caCertPath_;
    std::string clientCertPath_;
    std::string clientKeyPath_;
};

HttpClient::HttpClientImpl::HttpClientImpl(const std::string &host, int port,
                                           bool ssl_enabled) {
    DLOG_F(INFO, "Initializing HttpClient for {}: {}", host_, port_);
}

HttpClient::HttpClientImpl::~HttpClientImpl() {
    DLOG_F(INFO, "Destroying HttpClient for {}: {}", host_, port_);
}

template <typename RequestT>
bool HttpClient::HttpClientImpl::sendRequest(
    const std::string &method, const std::string &path,
    const std::map<std::string, std::string> &params, const json &data,
    json &response, std::string &err) {
    Client client(host_.c_str(), port_);
    if (sslEnabled_) {
        client.enable_server_certificate_verification(true);
        client.set_ca_cert_path(caCertPath_);
        if (!clientCertPath_.empty() && !clientKeyPath_.empty()) {
            // client.set_client_cert_and_key(clientCertPath_, clientKeyPath_);
        }
    }

    auto res = RequestT::run(client, path.c_str(), params, data.dump(),
                             "application/json");
    if (!res || res->status != 200) {
        err = res ? res->body : "Unknown error";
        LOG_F(ERROR,
              "Failed to send {} request to {}{}, data {}. Error message: {}",
              method, host_, path, data.dump(), err);
        return false;
    }

    try {
        response = json::parse(res->body);
        DLOG_F(INFO, "Received response from {}{}: {}", host_, path,
               response.dump());
    } catch (const std::exception &e) {
        LOG_F(ERROR, "Failed to parse response from {}{}. Error message: {}",
              host_, path, e.what());
        return false;
    }

    return true;
}

void HttpClient::HttpClientImpl::setSslEnabled(bool enabled) {
    sslEnabled_ = enabled;
}

void HttpClient::HttpClientImpl::setCaCertPath(const std::string &path) {
    caCertPath_ = path;
}

void HttpClient::HttpClientImpl::setClientCertPath(const std::string &path) {
    clientCertPath_ = path;
}

void HttpClient::HttpClientImpl::setClientKeyPath(const std::string &path) {
    clientKeyPath_ = path;
}

bool HttpClient::HttpClientImpl::scanPort(int start_port, int end_port,
                                          std::vector<int> &open_ports) {
    DLOG_F(INFO, "Scanning ports from {} to {} on {}:{}", start_port, end_port,
           host_, port_);

    open_ports.clear();
    Client client(host_.c_str(), port_);

    for (int port = start_port; port <= end_port; port++) {
        auto res = client.Head(std::to_string(port).c_str());
        if (res && res->status == 200) {
            open_ports.push_back(port);
            DLOG_F(INFO, "Port {} is open on {}:{}", port, host_, port_);
        }
    }

    return true;
}

bool HttpClient::HttpClientImpl::checkServerStatus(std::string &status) {
    DLOG_F(INFO, "Checking server status on {}: {}", host_, port_);
    Client client(host_.c_str(), port_);
    auto res = client.Head("/");
    if (!res || res->status != 200) {
        status = res ? std::to_string(res->status) : "Unknown error";
        LOG_F(ERROR,
              "Failed to check server status on {}: {} with error message: {}",
              host_, port_, status);
        return false;
    }

    status = "Running";
    return true;
}

HttpClient::HttpClient(const std::string &host, int port, bool sslEnabled) {
    m_impl = std::make_unique<HttpClientImpl>(host, port, sslEnabled);
}

HttpClient::~HttpClient() { m_impl.reset(); }

bool HttpClient::sendGetRequest(
    const std::string &path, const std::map<std::string, std::string> &params,
    json &response, std::string &err) {
    // return m_impl->sendRequest<httplib::Client::Get>("GET", path, params,
    // json(), response, err);
    return false;
}

bool HttpClient::sendPostRequest(
    const std::string &path, const std::map<std::string, std::string> &params,
    const json &data, json &response, std::string &err) {
    // return m_impl->sendRequest<httplib::Client::Post>("POST", path, params,
    // data, response, err);
    return false;
}

bool HttpClient::sendPutRequest(
    const std::string &path, const std::map<std::string, std::string> &params,
    const json &data, json &response, std::string &err) {
    // return m_impl->sendRequest<httplib::Client::Put>("PUT", path, params,
    // data, response, err);
    return false;
}

bool HttpClient::sendDeleteRequest(
    const std::string &path, const std::map<std::string, std::string> &params,
    json &response, std::string &err) {
    // return m_impl->sendRequest<httplib::Client::Delete>("DELETE", path,
    // params, json(), response, err);
    return false;
}

void HttpClient::setSslEnabled(bool enabled) { m_impl->setSslEnabled(enabled); }

void HttpClient::setCaCertPath(const std::string &path) {
    m_impl->setCaCertPath(path);
}

void HttpClient::setClientCertPath(const std::string &path) {
    m_impl->setClientCertPath(path);
}

void HttpClient::setClientKeyPath(const std::string &path) {
    m_impl->setClientKeyPath(path);
}

bool HttpClient::scanPort(int start_port, int end_port,
                          std::vector<int> &open_ports) {
    return m_impl->scanPort(start_port, end_port, open_ports);
}

bool HttpClient::checkServerStatus(std::string &status) {
    return m_impl->checkServerStatus(status);
}
}  // namespace atom::web
