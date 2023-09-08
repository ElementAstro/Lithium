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
#if ENABLE_ASYNC
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
#else
    ENDPOINT("GET", "/", root)
    {
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
        auto response = createResponse(Status::CODE_200, html);
        response->putHeader(Header::CONTENT_TYPE, "text/html");
        return response;
    }
#endif

#if ENABLE_ASYNC
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
            auto buffer = loadResource(path.getValue(""), {".json",".js",".css",".html",".jpg",".png",".robot"});
            OATPP_ASSERT_HTTP(buffer != "", Status::CODE_500, "Can't read file");

            return _return(controller->createResponse(Status::CODE_200, buffer));
        }
        
    };
#else

#endif

#include OATPP_CODEGEN_END(ApiController) //<- End Codegen

};

#endif // Lithium_STATICCONTROLLER_HPP