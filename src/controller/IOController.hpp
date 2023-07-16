#ifndef Lithium_IOCONTROLLER_HPP
#define Lithium_IOCONTROLLER_HPP

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
        OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper)
    )
    {
        return std::make_shared<IOController>(objectMapper);
    }
};

#include OATPP_CODEGEN_END(ApiController) //<- End Codegen

#endif // Lithium_IOCONTROLLER_HPP