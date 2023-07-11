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
    /**
     * @brief 构造函数，创建HTTP客户端对象。
     * @param host 服务器主机名或IP地址。
     * @param port 服务器端口号，默认为11111。
     */
    explicit HttpClient(const std::string &host, int port = 11111);

    /**
     * @brief 析构函数，释放资源。
     */
    ~HttpClient();

    /**
     * @brief 发送GET请求到服务器。
     * @param path 请求路径。
     * @param params 请求参数。
     * @param response 存储接收到的响应数据。
     * @param err 存储错误信息。
     * @return 请求是否成功。
     *
     * 发送GET请求到服务器。如果请求成功，将响应数据存储在response中并返回true，
     * 否则将错误信息存储在err中并返回false。
     */
    bool SendGetRequest(const std::string &path, const std::map<std::string, std::string> &params,
                        json &response, std::string &err);

    /**
     * @brief 发送POST请求到服务器。
     * @param path 请求路径。
     * @param params 请求参数。
     * @param data 请求数据。
     * @param response 存储接收到的响应数据。
     * @param err 存储错误信息。
     * @return 请求是否成功。
     *
     * 发送POST请求到服务器。如果请求成功，将响应数据存储在response中并返回true，
     * 否则将错误信息存储在err中并返回false。
     */
    bool SendPostRequest(const std::string &path, const std::map<std::string, std::string> &params,
                         const json &data, json &response, std::string &err);

    /**
     * @brief 发送PUT请求到服务器。
     * @param path 请求路径。
     * @param params 请求参数。
     * @param data 请求数据。
     * @param response 存储接收到的响应数据。
     * @param err 存储错误信息。
     * @return 请求是否成功。
     *
     * 发送PUT请求到服务器。如果请求成功，将响应数据存储在response中并返回true，
     * 否则将错误信息存储在err中并返回false。
     */
    bool SendPutRequest(const std::string &path, const std::map<std::string, std::string> &params,
                        const json &data, json &response, std::string &err);

    /**
     * @brief 发送DELETE请求到服务器。
     * @param path 请求路径。
     * @param params 请求参数。
     * @param response 存储接收到的响应数据。
     * @param err 存储错误信息。
     * @return 请求是否成功。
     *
     * 发送DELETE请求到服务器。如果请求成功，将响应数据存储在response中并返回true，
     * 否则将错误信息存储在err中并返回false。
     */
    bool SendDeleteRequest(const std::string &path, const std::map<std::string, std::string> &params,
                           json &response, std::string &err);

    /**
     * @brief 设置是否启用SSL。
     * @param enabled 是否启用。
     *
     * 设置是否启用SSL。
     */
    void SetSslEnabled(bool enabled);

    /**
     * @brief 设置CA证书路径。
     * @param path 路径。
     *
     * 设置CA证书路径。
     */
    void SetCaCertPath(const std::string &path);

    /**
     * @brief 设置客户端证书路径。
     * @param path 路径。
     *
     * 设置客户端证书路径。
     */
    void SetClientCertPath(const std::string &path);

    /**
     * @brief 设置客户端密钥路径。
     * @param path 路径。
     *
     * 设置客户端密钥路径。
     */
    void SetClientKeyPath(const std::string &path);

    /**
     * @brief 扫描指定范围内的端口是否开放。
     * @param start_port 起始端口号。
     * @param end_port 结束端口号。
     * @param open_ports 存储开放的端口号。
     * @return 是否扫描成功。
     *
     * 扫描指定范围内的端口是否开放，并将开放的端口号存储在open_ports中。
     */
    bool ScanPort(int start_port, int end_port, std::vector<int> &open_ports);

    /**
     * @brief 检查服务器状态。
     * @param status 存储服务器状态。如果服务器返回的数据不是JSON格式，则存储整个响应数据。
     * @return 是否检查成功。
     *
     * 向服务器发送请求，检查服务器状态。如果请求成功，将服务器状态存储在status中并返回true，
     * 否则将整个响应数据存储在status中并返回false。
     */
    bool CheckServerStatus(std::string &status);

private:
    std::string host_;
    int port_;
    bool ssl_enabled_;
    std::string ca_cert_path_;
    std::string client_cert_path_;
    std::string client_key_path_;
};
