/*
 * ErrorHandle.cpp
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

Date: 2023-7-13

Description: Error Handle (404 or 500)

**************************************************/

#include "ErrorHandler.hpp"

ErrorHandler::ErrorHandler(const std::shared_ptr<oatpp::data::mapping::ObjectMapper> &objectMapper)
    : m_objectMapper(objectMapper)
{
}

std::shared_ptr<ErrorHandler::OutgoingResponse>
ErrorHandler::handleError(const Status &status, const oatpp::String &message, const Headers &headers)
{
    auto error = StatusDto::createShared();
    error->status = "ERROR";
    error->code = status.code;
    error->message = message;

    if (status.code == 404)
    {
        auto response = ResponseFactory::createResponse(Status::CODE_404, R"(
            <!DOCTYPE html>
            <html lang="en">
            <head>
            <meta charset="UTF-8">
            <title>404 Not Found</title>
            <style>
                /* CSS样式 */
                body {
                background-color: #f1f1f1;
                font-family: Arial, sans-serif;
                text-align: center;
                padding: 150px;
                }
                
                h1 {
                font-size: 60px;
                color: #555;
                }
                
                p {
                font-size: 18px;
                color: #777;
                }
                
                a {
                color: #06c;
                text-decoration: none;
                }
                
                .container {
                max-width: 600px;
                margin: auto;
                background-color: #fff;
                border-radius: 5px;
                padding: 40px;
                box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
                }
                
                .image {
                margin-bottom: 30px;
                }
            </style>
            <script>
                // JavaScript代码
                document.addEventListener("DOMContentLoaded", function() {
                var backButton = document.getElementById("backButton");
                backButton.addEventListener("click", function(e) {
                    e.preventDefault();
                    history.back();
                });
                });
            </script>
            </head>
            <body>
            <div class="container">
                <h1>Oops! 404</h1>
                <p>抱歉，页面未找到。</p>
                <p><a href="#" id="backButton">返回上一页</a></p>
            </div>
            </body>
            </html>
        )");
        return response;
    }
    else
    {
        auto response = ResponseFactory::createResponse(status, error, m_objectMapper);

        for (const auto &pair : headers.getAll())
        {
            response->putHeader(pair.first.toString(), pair.second.toString());
        }
        return response;
    }
}