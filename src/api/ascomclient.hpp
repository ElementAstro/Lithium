/*
 * ascomclient.hpp
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

Date: 2023-5-26

Description: Ascom Remote Http Client

**************************************************/

#pragma once

#include <string>
#include <map>
#include <functional>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <iostream>

#include "nlohmann/json.hpp"
#include "curl/curl.h"
#include "spdlog/spdlog.h"

using json = nlohmann::json;

class ASCOMHttpClient
{
public:
    ASCOMHttpClient();
    ~ASCOMHttpClient();

    void setServer(const std::string &host, int port);
    void setResponseCallback(const ResponseCallback &callback);
    void sendRequest(const std::string &method, const json &params);

    bool isConnected() const;
    void waitUntilDisconnected();
    void readLoop();

private:
    static size_t writeCallback(const char *ptr, size_t size, size_t nmemb, void *userdata);
    void handleResponse();

private:
    std::string host_;
    int port_;
    std::string url_;

    CURL *curlHandle_;
    std::string responseData_;
    bool responseReady_ = false;
    std::mutex responseMutex_;
    std::condition_variable responseCv_;
    ResponseCallback responseCallback_;
    bool stopReadThread_ = false;
    bool connected_ = false;

    std::unique_ptr<std::thread> readThread_;
    std::shared_ptr<spdlog::logger> logger_;
};