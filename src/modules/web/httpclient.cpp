/*
 * httpclient.cpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-7-25

Description: Http Client

**************************************************/

#include "httpclient.hpp"

#include "cpp_httplib/httplib.h"

#include "loguru/loguru.hpp"

#if __cplusplus >= 202002L
#include <format>
#endif

using namespace httplib;

HttpClient::HttpClient(const std::string &host, int port)
    : host_(host), port_(port), ssl_enabled_(false)
{
    LOG_F(INFO, "Initializing HttpClient for %s:%d", host_.c_str(), port_);
}

bool HttpClient::SendGetRequest(const std::string &path, const std::map<std::string, std::string> &params, json &response, std::string &err)
{
    Client client(host_.c_str(), port_);
    if (ssl_enabled_)
    {
        client.enable_server_certificate_verification(true);
        client.set_ca_cert_path(ca_cert_path_.c_str());
        if (!client_cert_path_.empty() && !client_key_path_.empty())
        {
            // client.set_client_cert_and_key(client_cert_path_.c_str(), client_key_path_.c_str());
        }
    }

    auto res = client.Get(path.c_str());
    if (!res || res->status != 200)
    {
        err = res ? res->body : "Unknown error";
        LOG_F(ERROR, "Failed to send GET request to %s%s. Error message: %s", host_.c_str(), path.c_str(), err.c_str());
        return false;
    }

    try
    {
        response = json::parse(res->body);
        LOG_F(INFO, "Received response from %s%s: %s", host_.c_str(), path.c_str(), response.dump().c_str());
    }
    catch (const std::exception &e)
    {
        LOG_F(ERROR, "Failed to parse response from %s%s. Error message: %s", host_.c_str(), path.c_str(), e.what());
        return false;
    }

    return true;
}

bool HttpClient::SendPostRequest(const std::string &path, const std::map<std::string, std::string> &params, const json &data, json &response, std::string &err)
{
    Client client(host_.c_str(), port_);
    if (ssl_enabled_)
    {
        client.enable_server_certificate_verification(true);
        client.set_ca_cert_path(ca_cert_path_.c_str());
        if (!client_cert_path_.empty() && !client_key_path_.empty())
        {
            // client.set_client_cert_and_key(client_cert_path_.c_str(), client_key_path_.c_str());
        }
    }

    auto res = client.Post(path.c_str());
    if (!res || res->status != 200)
    {
        err = res ? res->body : "Unknown error";
        LOG_F(ERROR, "Failed to send POST request to %s%s, data %s. Error message: %s", host_.c_str(), path.c_str(), data.dump().c_str(), err.c_str());
        return false;
    }

    try
    {
        response = json::parse(res->body);
        LOG_F(INFO, "Received response from %s%s: %s", host_.c_str(), path.c_str(), response.dump().c_str());
    }
    catch (const std::exception &e)
    {
        LOG_F(ERROR, "Failed to parse response from %s%s. Error message: %s", host_.c_str(), path.c_str(), e.what());
        return false;
    }

    return true;
}

bool HttpClient::SendPutRequest(const std::string &path, const std::map<std::string, std::string> &params, const json &data, json &response, std::string &err)
{
    Client client(host_.c_str(), port_);
    if (ssl_enabled_)
    {
        client.enable_server_certificate_verification(true);
        client.set_ca_cert_path(ca_cert_path_.c_str());
        if (!client_cert_path_.empty() && !client_key_path_.empty())
        {
            // client.set_client_cert_and_key(client_cert_path_.c_str(), client_key_path_.c_str());
        }
    }

    auto res = client.Put(path.c_str());
    if (!res || res->status != 200)
    {
        err = res ? res->body : "Unknown error";
        LOG_F(ERROR, "Failed to send PUT request to %s%s, data %s. Error message: %s", host_.c_str(), path.c_str(), data.dump().c_str(), err.c_str());
        return false;
    }

    try
    {
        response = json::parse(res->body);
        LOG_F(INFO, "Received response from %s%s: %s", host_.c_str(), path.c_str(), response.dump().c_str());
    }
    catch (const std::exception &e)
    {
        LOG_F(ERROR, "Failed to parse response from %s%s. Error message: %s", host_.c_str(), path.c_str(), e.what());
        return false;
    }

    return true;
}

bool HttpClient::SendDeleteRequest(const std::string &path, const std::map<std::string, std::string> &params, json &response, std::string &err)
{
    Client client(host_.c_str(), port_);
    if (ssl_enabled_)
    {
        client.enable_server_certificate_verification(true);
        client.set_ca_cert_path(ca_cert_path_.c_str());
        if (!client_cert_path_.empty() && !client_key_path_.empty())
        {
            // client.set_client_cert_and_key(client_cert_path_.c_str(), client_key_path_.c_str());
        }
    }

    auto res = client.Delete(path.c_str());
    if (!res || res->status != 200)
    {
        err = res ? res->body : "Unknown error";
        LOG_F(ERROR, "Failed to send DELETE request to %s%s, data %s. Error message: %s", host_.c_str(), path.c_str(), res->body.c_str(), err.c_str());
        return false;
    }

    try
    {
        response = json::parse(res->body);
        LOG_F(INFO, "Received response from %s%s: %s", host_.c_str(), path.c_str(), response.dump().c_str());
    }
    catch (const std::exception &e)
    {
        LOG_F(ERROR, "Failed to parse response from %s%s. Error message: %s", host_.c_str(), path.c_str(), e.what());
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
    LOG_F(INFO, "Scanning ports from %d to %d on %s:%d", start_port, end_port, host_.c_str(), port_);

    open_ports.clear();
    Client client(host_.c_str(), port_);

    for (int port = start_port; port <= end_port; port++)
    {
#if __cplusplus >= 202002L
        auto res = client.Head(std::format("/{}", port).c_str());
#else
        auto path = "/" + std::to_string(port);
        auto res = client.Head(path.c_str());
#endif

        if (res && res->status == 200)
        {
            open_ports.push_back(port);
            LOG_F(INFO, "Port %d is open on %s:%d", port, host_.c_str(), port_);
        }
    }

    return true;
}

bool HttpClient::CheckServerStatus(std::string &status)
{
    LOG_F(INFO, "Checking server status on %s:%d", host_.c_str(), port_);
    Client client(host_.c_str(), port_);
    auto res = client.Head("/");
    if (!res || res->status != 200)
    {
        status = res ? std::to_string(res->status) : "Unknown error";
        LOG_F(ERROR, "Failed to check server status on %s:%d with error message: %s", host_.c_str(), port_, status.c_str());
        return false;
    }

    status = "Running";
    return true;
}

HttpClient::~HttpClient()
{
}