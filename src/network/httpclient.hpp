#pragma once

#include <string>
#include <map>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#define CPPHTTPLIB_OPENSSL_SUPPORT

class HttpClient
{
public:
    explicit HttpClient(const std::string &host, int port = 11111);
    ~HttpClient();

    bool SendGetRequest(const std::string &path, const std::map<std::string, std::string> &params, json &response, std::string &err);

    bool SendPostRequest(const std::string &path, const std::map<std::string, std::string> &params, const json &data, json &response, std::string &err);

    bool SendPutRequest(const std::string &path, const std::map<std::string, std::string> &params, const json &data, json &response, std::string &err);

    bool SendDeleteRequest(const std::string &path, const std::map<std::string, std::string> &params, json &response, std::string &err);

    void SetSslEnabled(bool enabled);
    void SetCaCertPath(const std::string &path);
    void SetClientCertPath(const std::string &path);
    void SetClientKeyPath(const std::string &path);

    bool ScanPort(int start_port, int end_port, std::vector<int> &open_ports);
    bool CheckServerStatus(std::string &status);

private:
    std::string host_;
    int port_;
    bool ssl_enabled_;
    std::string ca_cert_path_;
    std::string client_cert_path_;
    std::string client_key_path_;
};
