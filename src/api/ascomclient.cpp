/*
 * ascomclient.cpp
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

#include "ascomclient.hpp"

ASCOMHttpClient::ASCOMHttpClient()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);

    curlHandle_ = curl_easy_init();
    if (!curlHandle_)
    {
        throw std::runtime_error("Failed to initialize curl");
    }

    logger_ = spdlog::stdout_color_mt("ASCOMHttpClient");
}

ASCOMHttpClient::~ASCOMHttpClient()
{
    stopReadThread_ = true;
    if (readThread_ && readThread_->joinable())
    {
        readThread_->join();
    }

    if (curlHandle_)
    {
        curl_easy_cleanup(curlHandle_);
    }

    curl_global_cleanup();
}

void ASCOMHttpClient::setServer(const std::string &host, int port)
{
    host_ = host;
    port_ = port;
    url_ = "http://" + host_ + ":" + std::to_string(port_) + "/ascomremoteserver/api/";
}

void ASCOMHttpClient::setResponseCallback(const ResponseCallback &callback)
{
    responseCallback_ = callback;
}

void ASCOMHttpClient::sendRequest(const std::string &method, const json &params)
{
    json request = {
        {"method", method},
        {"params", params},
        {"jsonrpc", "2.0"},
        {"id", 1}};

    std::string requestString = request.dump();

    curl_easy_setopt(curlHandle_, CURLOPT_URL, url_.c_str());

    curl_easy_setopt(curlHandle_, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t)requestString.size());
    curl_easy_setopt(curlHandle_, CURLOPT_POSTFIELDS, requestString.c_str());

    responseData_.clear();
    responseReady_ = false;

    curl_easy_setopt(curlHandle_, CURLOPT_WRITEFUNCTION, &writeCallback);
    curl_easy_setopt(curlHandle_, CURLOPT_WRITEDATA, this);

    curl_multi_perform(curlMultiHandle_, &runningHandles_);

    handleResponse();
}

size_t ASCOMHttpClient::writeCallback(const char *ptr, size_t size, size_t nmemb, void *userdata)
{
    auto *self = static_cast<ASCOMHttpClient *>(userdata);

    self->responseData_ += std::string_view(ptr, size * nmemb);

    // Check if the response data ends with a complete JSON object (indicated by the "}" character)
    if (!self->responseReady_ && !self->responseData_.empty() && self->responseData_.back() == '}')
    {
        try
        {
            json response = json::parse(self->responseData_);
            self->responseReady_ = true;

            if (self->responseCallback_)
            {
                self->responseCallback_(response);
            }
        }
        catch (const std::exception &e)
        {
            std::string errorMessage = std::string("Failed to parse response as JSON: ") + e.what();

            if (self->logger_)
            {
                self->logger_->error(errorMessage);
            }
            else
            {
                std::cerr << errorMessage << std::endl;
            }
        }
    }

    return size * nmemb;
}

void ASCOMHttpClient::handleResponse()
{
    while (runningHandles_)
    {
        int numEvents;
        curl_multi_wait(curlMultiHandle_, NULL, 0, 1000, &numEvents);

        if (numEvents > 0)
        {
            curl_multi_perform(curlMultiHandle_, &runningHandles_);
        }

        CURLMsg *msg;
        while ((msg = curl_multi_info_read(curlMultiHandle_, &numMsgs_)))
        {
            if (msg->msg == CURLMSG_DONE)
            {
                long statusCode;
                curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &statusCode);

                if (statusCode >= 400)
                {
                    std::string errorMessage = std::string("HTTP request failed with status code ") + std::to_string(statusCode);

                    if (logger_)
                    {
                        logger_->error(errorMessage);
                    }
                    else
                    {
                        std::cerr << errorMessage << std::endl;
                    }
                }

                curl_multi_remove_handle(curlMultiHandle_, msg->easy_handle);
                curl_easy_cleanup(msg->easy_handle);

                if (--runningHandles_ == 0)
                {
                    curl_multi_cleanup(curlMultiHandle_);
                    curlMultiHandle_ = NULL;
                }

                responseCv_.notify_one();
            }
        }
    }
}

void ASCOMHttpClient::waitUntilDisconnected()
{
    std::unique_lock<std::mutex> lock{responseMutex_};

    responseCv_.wait(lock, [this]
                     { return !runningHandles_; });
}

bool ASCOMHttpClient::isConnected() const
{
    return connected_;
}

void ASCOMHttpClient::readLoop()
{
    while (!stopReadThread_)
    {
        curlMultiHandle_ = curl_multi_init();

        curl_easy_setopt(curlHandle_, CURLOPT_URL, url_.c_str());
        curl_multi_add_handle(curlMultiHandle_, curlHandle_);

        runningHandles_ = 0;
        curl_multi_perform(curlMultiHandle_, &runningHandles_);

        handleResponse();
    }
}