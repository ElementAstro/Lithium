/*
 * httplite.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-3

Description: Simple Http Client

**************************************************/

#ifndef ATOM_WEB_HTTPLITE_HPP
#define ATOM_WEB_HTTPLITE_HPP

#include <chrono>
#include <cstring>
#include <functional>
#include <iostream>
#include <map>
#include <string>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#undef DELETE
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#define SOCKET int
#endif

namespace atom::web {

enum class HttpMethod { GET, POST, PUT, DELETE };

struct HttpResponse {
    std::string body;
    int statusCode;
    std::string statusMessage;
};

/**
 * @brief HttpClient 类用于与服务器建立连接并发送请求接收响应。
 */
class HttpClient {
public:
    /**
     * @brief HttpClient 构造函数
     */
    HttpClient();

    /**
     * @brief HttpClient 析构函数
     */
    ~HttpClient();

    /**
     * @brief 设置错误处理函数
     * @param errorHandler 错误处理函数
     */
    void setErrorHandler(std::function<void(const std::string &)> errorHandler);

    /**
     * @brief 初始化 HttpClient
     * @return 初始化是否成功
     */
    bool initialize();

    /**
     * @brief 连接到服务器
     * @param host 服务器主机名
     * @param port 服务器端口号
     * @param useHttps 是否使用 HTTPS
     * @return 连接是否成功
     */
    bool connectToServer(const std::string &host, int port, bool useHttps);

    /**
     * @brief 发送请求到服务器
     * @param request 请求内容
     * @return 是否成功发送请求
     */
    bool sendRequest(const std::string &request);

    /**
     * @brief 接收服务器响应
     * @return 服务器响应
     */
    HttpResponse receiveResponse();

private:
    /**
     * @brief 关闭 socket 连接
     */
    void closeSocket();

private:
    SOCKET socketfd; /**< socket 文件描述符 */
    std::function<void(const std::string &)> errorHandler; /**< 错误处理函数 */
};

/**
 * @brief HttpRequestBuilder 类用于构建 HTTP 请求。
 */
class HttpRequestBuilder {
public:
    /**
     * @brief HttpRequestBuilder 构造函数
     * @param method HTTP 请求方法
     * @param url 请求 URL
     */
    HttpRequestBuilder(HttpMethod method, const std::string &url);

    /**
     * @brief 设置请求体
     * @param bodyText 请求体内容
     * @return 当前 HttpRequestBuilder 实例
     */
    HttpRequestBuilder &setBody(const std::string &bodyText);

    /**
     * @brief 设置请求内容类型
     * @param contentTypeValue 内容类型值
     * @return 当前 HttpRequestBuilder 实例
     */
    HttpRequestBuilder &setContentType(const std::string &contentTypeValue);

    /**
     * @brief 设置超时时间
     * @param timeoutValue 超时时间
     * @return 当前 HttpRequestBuilder 实例
     */
    HttpRequestBuilder &setTimeout(std::chrono::seconds timeoutValue);

    /**
     * @brief 添加请求头
     * @param key 请求头键
     * @param value 请求头值
     * @return 当前 HttpRequestBuilder 实例
     */
    HttpRequestBuilder &addHeader(const std::string &key,
                                  const std::string &value);

    /**
     * @brief 发送 HTTP 请求
     * @return HTTP 响应
     */
    HttpResponse send();

private:
    /**
     * @brief 构建请求字符串
     * @param host 主机名
     * @param path 路径
     * @return 构建的请求字符串
     */
    std::string buildRequestString(const std::string &host,
                                   const std::string &path);

private:
    HttpMethod method;                          /**< HTTP 请求方法 */
    std::string url;                            /**< 请求 URL */
    std::string body;                           /**< 请求体 */
    std::string contentType;                    /**< 内容类型 */
    std::chrono::seconds timeout;               /**< 超时时间 */
    std::map<std::string, std::string> headers; /**< 请求头映射 */
};
}  // namespace atom::web

#endif
