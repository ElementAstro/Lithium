/*
 * httpclient.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-25

Description: Http Client

**************************************************/

#ifndef ATOM_WEB_HTTPCLIENT_HPP
#define ATOM_WEB_HTTPCLIENT_HPP

#include <string>
#include <map>
#include <vector>
#include <memory>
#include "atom/type/json.hpp"

using json = nlohmann::json;

#define CPPHTTPLIB_OPENSSL_SUPPORT

namespace Atom::Web
{
    /**
     * @brief Class for making HTTP requests using HttpClientImpl.
     */
    class HttpClient
    {
    public:
        /**
         * @brief Constructor for HttpClient.
         * @param host The host to connect to.
         * @param port The port to connect to.
         * @param sslEnabled Whether SSL is enabled.
         */
        explicit HttpClient(const std::string &host, int port, bool sslEnabled);

        /**
         * @brief Destructor for HttpClient.
         */
        ~HttpClient();

        /**
         * @brief Send HTTP request.
         * @param path The path to send the request to.
         * @param params The parameters to send with the request.
         * @param response The response from the server.
         * @param err The error message.
         * @return Whether the request was successful.
         */
        bool sendGetRequest(const std::string &path, const std::map<std::string, std::string> &params, nlohmann::json &response, std::string &err);
        
        /**
         * @brief Send HTTP request.
         * @param path The path to send the request to.
         * @param params The parameters to send with the request.
         * @param data The data to send with the request.
         * @param response The response from the server.
         * @param err The error message.
         * @return Whether the request was successful.
         */
        bool sendPostRequest(const std::string &path, const std::map<std::string, std::string> &params, const nlohmann::json &data, nlohmann::json &response, std::string &err);
        
        /**
         * @brief Send HTTP request.
         * @param path The path to send the request to.
         * @param params The parameters to send with the request.
         * @param data The data to send with the request.
         * @param response The response from the server.
         * @param err The error message.
         * @return Whether the request was successful.
         */
        bool sendPutRequest(const std::string &path, const std::map<std::string, std::string> &params, const nlohmann::json &data, nlohmann::json &response, std::string &err);
        
        /**
         * @brief Send HTTP request.
         * @param path The path to send the request to.
         * @param params The parameters to send with the request.
         * @param response The response from the server.
         * @param err The error message.
         * @return Whether the request was successful.
         */
        bool sendDeleteRequest(const std::string &path, const std::map<std::string, std::string> &params, nlohmann::json &response, std::string &err);

        /**
         * @brief Set SSL enabled.
         * @param enabled Whether SSL is enabled.
         */
        void setSslEnabled(bool enabled);

        /**
         * @brief Set the CA cert path.
         * @param path The path to the CA cert.
         */
        void setCaCertPath(const std::string &path);

        /**
         * @brief Set the client cert path.
         * @param path The path to the client cert.
         */
        void setClientCertPath(const std::string &path);

        /**
         * @brief Set the client key path.
         * @param path The path to the client key.
         */
        void setClientKeyPath(const std::string &path);

        /**
         * @brief Scan ports.
         * @param startPort The start port.
         * @param endPort The end port.
         * @param openPorts The open ports.
         * @return Whether the scan was successful.
         */
        bool scanPort(int startPort, int endPort, std::vector<int> &openPorts);

        /**
         * @brief Check server status.
         * @param status The server status.
         * @return Whether the check was successful.
         */
        bool checkServerStatus(std::string &status);

    private:
        class HttpClientImpl;
        std::unique_ptr<HttpClientImpl> m_impl;
    };

}

#endif
