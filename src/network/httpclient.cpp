#include "httpclient.hpp"
#include <iostream>
#include "cpp_httplib/httplib.h"
#include "spdlog/spdlog.h"

using namespace httplib;

HttpClient::HttpClient(const std::string &host, int port)
    : host_(host), port_(port), ssl_enabled_(false)
{
    spdlog::info("Initializing HttpClient for {}:{}", host_, port_);
}

bool HttpClient::SendGetRequest(const std::string &path, const std::map<std::string, std::string> &params, json &response, std::string &err)
{
    //spdlog::info("Sending GET request to {}{} with parameters: {}", host_, path, params);

    Client client(host_.c_str(), port_);
    if (ssl_enabled_)
    {
        client.enable_server_certificate_verification(true);
        client.set_ca_cert_path(ca_cert_path_.c_str());
        if (!client_cert_path_.empty() && !client_key_path_.empty())
        {
            //client.set_client_cert_and_key(client_cert_path_.c_str(), client_key_path_.c_str());
        }
    }

    auto res = client.Get(path.c_str());
    if (!res || res->status != 200)
    {
        err = res ? res->body : "Unknown error";
        //spdlog::error("Failed to send GET request to {}{} with parameters {}. Error message: {}", host_, path, params, err);
        return false;
    }

    try
    {
        response = json::parse(res->body);
        spdlog::info("Received response from {}{}: {}", host_, path, response.dump());
    }
    catch (const std::exception &e)
    {
        err = e.what();
        spdlog::error("Failed to parse response from {}{}. Error message: {}", host_, path, err);
        return false;
    }

    return true;
}

bool HttpClient::SendPostRequest(const std::string &path, const std::map<std::string, std::string> &params, const json &data, json &response, std::string &err)
{
    //spdlog::info("Sending POST request to {}{} with parameters: {}, data: {}", host_, path, params, data.dump());

    Client client(host_.c_str(), port_);
    if (ssl_enabled_)
    {
        client.enable_server_certificate_verification(true);
        client.set_ca_cert_path(ca_cert_path_.c_str());
        if (!client_cert_path_.empty() && !client_key_path_.empty())
        {
            //client.set_client_cert_and_key(client_cert_path_.c_str(), client_key_path_.c_str());
        }
    }

    auto res = client.Post(path.c_str());
    if (!res || res->status != 200)
    {
        err = res ? res->body : "Unknown error";
        //spdlog::error("Failed to send POST request to {}{} with parameters {}, data {}. Error message: {}", host_, path, params, data.dump(), err);
        return false;
    }

    try
    {
        response = json::parse(res->body);
        spdlog::info("Received response from {}{}: {}", host_, path, response.dump());
    }
    catch (const std::exception &e)
    {
        err = e.what();
        spdlog::error("Failed to parse response from {}{}. Error message: {}", host_, path, err);
        return false;
    }

    return true;
}

bool HttpClient::SendPutRequest(const std::string &path, const std::map<std::string, std::string> &params, const json &data, json &response, std::string &err)
{
    //spdlog::info("Sending PUT request to {}{} with parameters: {}, data: {}", host_, path, params, data.dump());

    Client client(host_.c_str(), port_);
    if (ssl_enabled_)
    {
        client.enable_server_certificate_verification(true);
        client.set_ca_cert_path(ca_cert_path_.c_str());
        if (!client_cert_path_.empty() && !client_key_path_.empty())
        {
            //client.set_client_cert_and_key(client_cert_path_.c_str(), client_key_path_.c_str());
        }
    }

    auto res = client.Put(path.c_str());
    if (!res || res->status != 200)
    {
        err = res ? res->body : "Unknown error";
        //spdlog::error("Failed to send PUT request to {}{} with parameters {}, data {}. Error message: {}", host_, path, params, data.dump(), err);
        return false;
    }

    try
    {
        response = json::parse(res->body);
        spdlog::info("Received response from {}{}: {}", host_, path, response.dump());
    }
    catch (const std::exception &e)
    {
        err = e.what();
        spdlog::error("Failed to parse response from {}{}. Error message: {}", host_, path, err);
        return false;
    }

    return true;
}

bool HttpClient::SendDeleteRequest(const std::string &path, const std::map<std::string, std::string> &params, json &response, std::string &err)
{
    //spdlog::info("Sending DELETE request to {}{} with parameters: {}", host_, path, params);

    Client client(host_.c_str(), port_);
    if (ssl_enabled_)
    {
        client.enable_server_certificate_verification(true);
        client.set_ca_cert_path(ca_cert_path_.c_str());
        if (!client_cert_path_.empty() && !client_key_path_.empty())
        {
            //client.set_client_cert_and_key(client_cert_path_.c_str(), client_key_path_.c_str());
        }
    }

    auto res = client.Delete(path.c_str());
    if (!res || res->status != 200)
    {
        err = res ? res->body : "Unknown error";
        //spdlog::error("Failed to send DELETE request to {}{} with parameters {}. Error message: {}", host_, path, params, err);
        return false;
    }

    try
    {
        response = json::parse(res->body);
        spdlog::info("Received response from {}{}: {}", host_, path, response.dump());
    }
    catch (const std::exception &e)
    {
        err = e.what();
        spdlog::error("Failed to parse response from {}{}. Error message: {}", host_, path, err);
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
    spdlog::info("Scanning ports from {} to {} on {}:{}", start_port, end_port, host_, port_);

    open_ports.clear();
    Client client(host_.c_str(), port_);

    for (int port = start_port; port <= end_port; port++)
    {
        auto res = client.Head(fmt::format("/{}", port).c_str());
        if (res && res->status == 200)
        {
            open_ports.push_back(port);
            spdlog::info("Port {} is open on {}:{}", port, host_, port_);
        }
    }

    return true;
}

bool HttpClient::CheckServerStatus(std::string &status)
{
    spdlog::info("Checking server status on {}:{}", host_, port_);

    Client client(host_.c_str(), port_);
    auto res = client.Head("/");
    if (!res || res->status != 200)
    {
        status = res ? std::to_string(res->status) : "Unknown error";
        spdlog::error("Failed to check server status on {}:{} with error message: {}", host_, port_, status);
        return false;
    }

    status = "Running";
    return true;
}

HttpClient::~HttpClient()
{
    spdlog::info("Destroying HttpClient");
}