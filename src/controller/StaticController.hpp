/*
 * StaticController.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
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

Date: 2023-7-13

Description: Static Route

**************************************************/

#ifndef Lithium_STATICCONTROLLER_HPP
#define Lithium_STATICCONTROLLER_HPP

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"

#include "config.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_set>

class StaticController : public oatpp::web::server::api::ApiController
{
public:
    StaticController(const std::shared_ptr<ObjectMapper> &objectMapper)
        : oatpp::web::server::api::ApiController(objectMapper)
    {
    }

public:
    static std::shared_ptr<StaticController> createShared(
        OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper) // Inject objectMapper component here as default parameter
    )
    {
        return std::make_shared<StaticController>(objectMapper);
    }

    static std::string loadResource(const std::string &path, const std::unordered_set<std::string> &allowedExtensions)
    {
        std::string fullPath;
        if (std::filesystem::path(path).is_absolute())
        {
            fullPath = path;
        }
        else
        {
            // 获取当前路径
            std::filesystem::path currentPath = std::filesystem::current_path();
            fullPath = (currentPath / path).string();
        }

        std::string fileExtension = fullPath.substr(fullPath.find_last_of(".") + 1);

        if (allowedExtensions.find(fileExtension) == allowedExtensions.end())
        {
            OATPP_LOGE("StaticFileManager", "File type not allowed: %s", fileExtension.c_str());
            return "";
        }

        std::ifstream file(fullPath);

        if (!file.is_open())
        {
            OATPP_LOGE("StaticFileManager", "Failed to open file: %s", fullPath.c_str());
            return "";
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

#include OATPP_CODEGEN_BEGIN(ApiController) //<- Begin Codegen

    ENDPOINT_INFO(root)
    {
        info->summary = "'Root' endpoint";
    }
    ENDPOINT_ASYNC("GET", "/", root)
    {
        ENDPOINT_ASYNC_INIT(root)
        const char *html =
            "<html lang='en'>"
            "  <head>"
            "    <meta charset=utf-8/>"
            "  </head>"
            "  <body>"
            "    <p>Hello Lithium example project!</p>"
            "    <a href='swagger/ui'>Checkout Swagger-UI page</a>"
            "  </body>"
            "</html>";
        Action act() override
        {
            auto response = controller->createResponse(Status::CODE_200, html);
            response->putHeader(Header::CONTENT_TYPE, "text/html");
            return _return(response);
        }
    };

    ENDPOINT_ASYNC("GET", "/static/*", Static) {
    
        ENDPOINT_ASYNC_INIT(Static)
        
        Action act() override
        {
            auto tail = request->getPathTail();
            OATPP_ASSERT_HTTP(tail, Status::CODE_400, "Empty filename");

            oatpp::parser::Caret caret(tail);

            auto pathLabel = caret.putLabel();
            caret.findChar('?');

            auto path = pathLabel.toString();
            auto buffer = loadResource(path.getValue(""), {"json","js","css","html","jpg","png","robot"});
            OATPP_ASSERT_HTTP(buffer != "", Status::CODE_500, "Can't read file");

            return _return(controller->createResponse(Status::CODE_200, buffer));
        }
        
    };
    
#include OATPP_CODEGEN_END(ApiController) //<- End Codegen

};

#endif // Lithium_STATICCONTROLLER_HPP