
#ifndef AppComponent_hpp
#define AppComponent_hpp

#include "components/SwaggerComponent.hpp"
#include "components/DatabaseComponent.hpp"

#include "ErrorHandler.hpp"

#include "oatpp/web/server/interceptor/AllowCorsGlobal.hpp"
#include "interceptor/AuthInterceptor.hpp"

#include "oatpp/web/server/HttpConnectionHandler.hpp"
#include "oatpp/web/server/HttpRouter.hpp"
#include "oatpp/network/tcp/server/ConnectionProvider.hpp"

#include "oatpp/parser/json/mapping/ObjectMapper.hpp"

#include "oatpp/core/macro/component.hpp"

/**
 *  Class which creates and holds Application components and registers components in oatpp::base::Environment
 *  Order of components initialization is from top to bottom
 */
class AppComponent
{
public:
    /**
     *  Swagger component
     */
    SwaggerComponent swaggerComponent;

    /**
     * Database component
     */
    DatabaseComponent databaseComponent;

    OATPP_CREATE_COMPONENT(std::shared_ptr<JWT>, jwt)
    ([]
     { return std::make_shared<JWT>("<my-secret>", "<my-issuer>"); }());

    /**
     * Create ObjectMapper component to serialize/deserialize DTOs in Contoller's API
     */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, apiObjectMapper)
    ([]
     {
    auto objectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
    objectMapper->getDeserializer()->getConfig()->allowUnknownFields = false;
    return objectMapper; }());

    /**
     *  Create ConnectionProvider component which listens on the port
     */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, serverConnectionProvider)
    ([]
     { return oatpp::network::tcp::server::ConnectionProvider::createShared({"0.0.0.0", 8000, oatpp::network::Address::IP_4}); }());

    /**
     *  Create Router component
     */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, httpRouter)
    ([]
     { return oatpp::web::server::HttpRouter::createShared(); }());

    /**
     *  Create ConnectionHandler component which uses Router component to route requests
     */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, serverConnectionHandler)
    ([]
     {
         OATPP_COMPONENT(std::shared_ptr<JWT>, jwt);                                         // get JWT component
         OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);           // get Router component
         OATPP_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, objectMapper); // get ObjectMapper component

         auto connectionHandler = oatpp::web::server::HttpConnectionHandler::createShared(router);

         connectionHandler->setErrorHandler(std::make_shared<ErrorHandler>(objectMapper));

         connectionHandler->addRequestInterceptor(std::make_shared<oatpp::web::server::interceptor::AllowOptionsGlobal>());
         connectionHandler->addRequestInterceptor(std::make_shared<AuthInterceptor>(jwt));

         connectionHandler->addResponseInterceptor(std::make_shared<oatpp::web::server::interceptor::AllowCorsGlobal>());

         return connectionHandler;
     }());
};

#endif /* AppComponent_hpp */
