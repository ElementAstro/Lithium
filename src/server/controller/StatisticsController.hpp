#ifndef StatisticsController_hpp
#define StatisticsController_hpp

#include "dto/Config.hpp"
#include "utils/Statistics.hpp"

#include "oatpp/network/ConnectionHandler.hpp"
#include "oatpp/web/server/api/ApiController.hpp"

#include "oatpp/macro/codegen.hpp"
#include "oatpp/macro/component.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)  /// <-- Begin Code-Gen

class StatisticsController : public oatpp::web::server::api::ApiController {
private:
    typedef StatisticsController __ControllerType;

private:
    OATPP_COMPONENT(oatpp::Object<ConfigDto>, m_appConfig);
    OATPP_COMPONENT(std::shared_ptr<Statistics>, m_statistics);

public:
    StatisticsController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>,
                                         objectMapper))
        : oatpp::web::server::api::ApiController(objectMapper) {}

public:
    ENDPOINT_ASYNC("GET", m_appConfig->statisticsUrl, Stats){

        ENDPOINT_ASYNC_INIT(Stats)

            Action act()
                override{auto json = controller->m_statistics->getJsonData();
    auto response = controller->createResponse(Status::CODE_200, json);
    response->putHeader(Header::CONTENT_TYPE, "application/json");
    return _return(response);
}
}
;
}
;

#include OATPP_CODEGEN_END(ApiController)  /// <-- End Code-Gen

#endif /* StatisticsController_hpp */
