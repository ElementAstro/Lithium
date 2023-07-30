#ifndef Lithium_IOCONTROLLER_HPP
#define Lithium_IOCONTROLLER_HPP

#include "config.h"

#include "modules/io/io.hpp"
#include "modules/io/file.hpp"
#include "modules/io/compress.hpp"

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"

#include "nlohmann/json.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController) //<- Begin Codegen

class IOController : public oatpp::web::server::api::ApiController
{
public:
    IOController(const std::shared_ptr<ObjectMapper> &objectMapper)
        : oatpp::web::server::api::ApiController(objectMapper)
    {
    }

public:
    static std::shared_ptr<IOController> createShared(
        OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
    {
        return std::make_shared<IOController>(objectMapper);
    }

    ENDPOINT_INFO(root)
    {
        info->summary = "'Root' endpoint. Without any params";
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
            return _return(controller->createResponse(Status::CODE_200, html));
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
};

#include OATPP_CODEGEN_END(ApiController) //<- End Codegen

#endif // Lithium_IOCONTROLLER_HPP