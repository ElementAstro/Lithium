
#ifndef WebSocketController_hpp
#define WebSocketController_hpp

#include "config.h"

#include "oatpp-websocket/Handshaker.hpp"

#include "oatpp/web/server/api/ApiController.hpp"

#include "oatpp/network/ConnectionHandler.hpp"

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController) //<-- codegen begin

/**
 * Controller with WebSocket-connect endpoint.
 */
class WebSocketController : public oatpp::web::server::api::ApiController
{
#if ENABLE_ASYNC
private:
  typedef WebSocketController __ControllerType;
#endif
private:
	OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, websocketConnectionHandler, "websocket");

public:
	WebSocketController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
		: oatpp::web::server::api::ApiController(objectMapper)
	{
	}

public:
	static std::shared_ptr<WebSocketController> createShared(
		OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
	{
		return std::make_shared<WebSocketController>(objectMapper);
	}

#if ENABLE_ASYNC
	ENDPOINT_ASYNC("GET", "ws", ws)
	{
		ENDPOINT_ASYNC_INIT(ws)
		Action act() override 
		{
			auto response = oatpp::websocket::Handshaker::serversideHandshake(request->getHeaders(), controller->websocketConnectionHandler);
			return _return(response);
		}
	};
#else
	ENDPOINT("GET", "ws", ws, REQUEST(std::shared_ptr<IncomingRequest>, request))
	{
		return oatpp::websocket::Handshaker::serversideHandshake(request->getHeaders(), websocketConnectionHandler);
	};
#endif
};

#include OATPP_CODEGEN_END(ApiController) //<-- codegen end

#endif /* WebSocketController_hpp */
