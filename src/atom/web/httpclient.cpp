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

HttpClient::HttpClient(const std::string &host, int port)
    : host_(host), port_(port), ssl_enabled_(false)
{
    DLOG_F(INFO, "Initializing HttpClient for {}:%d", host_, port_);
}

bool HttpClient::SendGetRequest(const std::string &path, const std::map<std::string, std::string> &params, json &response, std::string &err)
{
    Client client(host_, port_);
    if (ssl_enabled_)
    {
        client.enable_server_certificate_verification(true);
        client.set_ca_cert_path(ca_cert_path_);
        if (!client_cert_path_.empty() && !client_key_path_.empty())
        {
            // client.set_client_cert_and_key(client_cert_path_, client_key_path_);
        }
    }

    auto res = client.Get(path);
    if (!res || res->status != 200)
    {
        err = res ? res->body : "Unknown error";
        LOG_F(ERROR, "Failed to send GET request to {}{}. Error message: {}", host_, path, err);
        return false;
    }

    try
    {
        response = json::parse(res->body);
        DLOG_F(INFO, "Received response from {}{}: {}", host_, path, response.dump());
    }
    catch (const std::exception &e)
    {
        LOG_F(ERROR, "Failed to parse response from {}{}. Error message: {}", host_, path, e.what());
        return false;
    }

    return true;
}

bool HttpClient::SendPostRequest(const std::string &path, const std::map<std::string, std::string> &params, const json &data, json &response, std::string &err)
{
    Client client(host_, port_);
    if (ssl_enabled_)
    {
        // client.enable_server_certificate_verification(true);
        client.set_ca_cert_path(ca_cert_path_);
        if (!client_cert_path_.empty() && !client_key_path_.empty())
        {
            // client.set_client_cert_and_key(client_cert_path_, client_key_path_);
        }
    }

    auto res = client.Post(path);
    if (!res || res->status != 200)
    {
        err = res ? res->body : "Unknown error";
        LOG_F(ERROR, "Failed to send POST request to {}{}, data {}. Error message: {}", host_, path, data.dump(), err);
        return false;
    }

    try
    {
        response = json::parse(res->body);
        DLOG_F(INFO, "Received response from {}{}: {}", host_, path, response.dump());
    }
    catch (const std::exception &e)
    {
        LOG_F(ERROR, "Failed to parse response from {}{}. Error message: {}", host_, path, e.what());
        return false;
    }

    return true;
}

bool HttpClient::SendPutRequest(const std::string &path, const std::map<std::string, std::string> &params, const json &data, json &response, std::string &err)
{
    Client client(host_, port_);
    if (ssl_enabled_)
    {
        // client.enable_server_certificate_verification(true);
        client.set_ca_cert_path(ca_cert_path_);
        if (!client_cert_path_.empty() && !client_key_path_.empty())
        {
            // client.set_client_cert_and_key(client_cert_path_, client_key_path_);
        }
    }

    auto res = client.Put(path);
    if (!res || res->status != 200)
    {
        err = res ? res->body : "Unknown error";
        LOG_F(ERROR, "Failed to send PUT request to {}{}, data {}. Error message: {}", host_, path, data.dump(), err);
        return false;
    }

    try
    {
        response = json::parse(res->body);
        DLOG_F(INFO, "Received response from {}{}: {}", host_, path, response.dump());
    }
    catch (const std::exception &e)
    {
        LOG_F(ERROR, "Failed to parse response from {}{}. Error message: {}", host_, path, e.what());
        return false;
    }

    return true;
}

bool HttpClient::SendDeleteRequest(const std::string &path, const std::map<std::string, std::string> &params, json &response, std::string &err)
{
    Client client(host_, port_);
    if (ssl_enabled_)
    {
        // client.enable_server_certificate_verification(true);
        client.set_ca_cert_path(ca_cert_path_);
        if (!client_cert_path_.empty() && !client_key_path_.empty())
        {
            // client.set_client_cert_and_key(client_cert_path_, client_key_path_);
        }
    }

    auto res = client.Delete(path);
    if (!res || res->status != 200)
    {
        err = res ? res->body : "Unknown error";
        LOG_F(ERROR, "Failed to send DELETE request to {}{}, data {}. Error message: {}", host_, path, res->body, err);
        return false;
    }

    try
    {
        response = json::parse(res->body);
        DLOG_F(INFO, "Received response from {}{}: {}", host_, path, response.dump());
    }
    catch (const std::exception &e)
    {
        LOG_F(ERROR, "Failed to parse response from {}{}. Error message: {}", host_, path, e.what());
        return false;
    }

    return true;
}

void HttpClient::SetSslEnabled(bool enabled)
{
    ssl_enabled_ = enabled;
}

void HttpClient::SetCaCertPath(const std::string &path)
{
    ca_cert_path_ = path;
}

void HttpClient::SetClientCertPath(const std::string &path)
{
    client_cert_path_ = path;
}

void HttpClient::SetClientKeyPath(const std::string &path)
{
    client_key_path_ = path;
}

bool HttpClient::ScanPort(int start_port, int end_port, std::vector<int> &open_ports)
{
    DLOG_F(INFO, "Scanning ports from %d to %d on {}:%d", start_port, end_port, host_, port_);

    open_ports.clear();
    Client client(host_, port_);

    for (int port = start_port; port <= end_port; port++)
    {
#if __cplusplus >= 202002L
        auto res = client.Head(std::format("/{}", port));
#else
        auto path = "/" + std::to_string(port);
        auto res = client.Head(path);
#endif

        if (res && res->status == 200)
        {
            open_ports.push_back(port);
            DLOG_F(INFO, "Port %d is open on {}:%d", port, host_, port_);
        }
    }

    return true;
}

bool HttpClient::CheckServerStatus(std::string &status)
{
    DLOG_F(INFO, "Checking server status on {}:%d", host_, port_);
    Client client(host_, port_);
    auto res = client.Head("/");
    if (!res || res->status != 200)
    {
        status = res ? std::to_string(res->status) : "Unknown error";
        LOG_F(ERROR, "Failed to check server status on {}:%d with error message: {}", host_, port_, status);
        return false;
    }

    status = "Running";
    return true;
}

HttpClient::~HttpClient()
{
}