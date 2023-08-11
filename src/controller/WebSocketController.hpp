
#ifndef WebSocketController_hpp
#define WebSocketController_hpp

#include "config.h"

#include "oatpp-websocket/Handshaker.hpp"

#include "oatpp/web/server/api/ApiController.hpp"

#include "oatpp/network/ConnectionHandler.hpp"

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"

#include <vector>
#include <algorithm>

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
	OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, websocketDeviceConnectionHandler, "websocket-device");
	OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, websocketScriptConnectionHandler, "websocket-script");
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

	ENDPOINT_ASYNC("GET", "/ws/device/{device-hub}/{device-name}", wsDevice)
	{
		ENDPOINT_ASYNC_INIT(wsDevice)
		Action act() override 
		{
			auto deviceName = request->getPathVariable("device-name");
			auto deviceHub = request->getPathVariable("device-hub");
			std::vector<std::string> available_plugins = {"camera", "telescope", "focuser", "filterwheel","solver","guider"};
			auto it = std::find(available_plugins.begin(), available_plugins.end(), deviceHub);
			OATPP_ASSERT_HTTP(it != available_plugins.end(), Status::CODE_500, "Invalid device type");
			auto response = oatpp::websocket::Handshaker::serversideHandshake(request->getHeaders(), controller->websocketDeviceConnectionHandler);
			auto parameters = std::make_shared<oatpp::network::ConnectionHandler::ParameterMap>();
			(*parameters)["deviceName"] = deviceName;
			(*parameters)["deviceHub"] = deviceHub;
			response->setConnectionUpgradeParameters(parameters);
			return _return(response);
		}
	};

	ENDPOINT_ASYNC("GET", "/ws/plugin/{hub-name}/{name}", WsPlugin)
	{
		ENDPOINT_ASYNC_INIT(WsPlugin)
		Action act() override 
		{
			oatpp::String hubName = request->getPathVariable("hub-name");
			oatpp::String pluginName = request->getPathVariable("name");
			std::vector<std::string> available_plugins = {"script", "exe", "liscript"};
			auto it = std::find(available_plugins.begin(), available_plugins.end(), hubName->c_str());
			OATPP_ASSERT_HTTP(it != available_plugins.end(), Status::CODE_500, "Invalid plugin type");
			auto response = oatpp::websocket::Handshaker::serversideHandshake(request->getHeaders(), controller->websocketScriptConnectionHandler);
			auto parameters = std::make_shared<oatpp::network::ConnectionHandler::ParameterMap>();
			(*parameters)["pluginName"] = pluginName;
			(*parameters)["pluginHub"] = hubName;
			response->setConnectionUpgradeParameters(parameters);
			return _return(response);
		}
	};
#else
	ENDPOINT("GET", "/ws", ws, REQUEST(std::shared_ptr<IncomingRequest>, request))
	{
		return oatpp::websocket::Handshaker::serversideHandshake(request->getHeaders(), websocketConnectionHandler);
	};

	ENDPOINT("GET", "/ws/device/{device-hub}/{device-name}", wsDevice, REQUEST(std::shared_ptr<IncomingRequest>, request))
	{
		auto deviceName = request->getPathVariable("device-name");
		auto deviceHub = request->getPathVariable("device-hub");
		std::vector<std::string> available_plugins = {"camera", "telescope", "focuser", "filterwheel","solver","guider"};
		auto it = std::find(available_plugins.begin(), available_plugins.end(), deviceHub);
		OATPP_ASSERT_HTTP(it != available_plugins.end(), Status::CODE_500, "Invalid device type");
		auto response = oatpp::websocket::Handshaker::serversideHandshake(request->getHeaders(), controller->websocketDeviceConnectionHandler);
		auto parameters = std::make_shared<oatpp::network::ConnectionHandler::ParameterMap>();
		(*parameters)["deviceName"] = deviceName;
		(*parameters)["deviceHub"] = deviceHub;
		response->setConnectionUpgradeParameters(parameters);
		return response;
	};

	ENDPOINT("GET", "/ws/plugin/{hub-name}/{name}", WsPlugin, REQUEST(std::shared_ptr<IncomingRequest>, request))
	{
		oatpp::String hubName = request->getPathVariable("hub-name");
		oatpp::String pluginName = request->getPathVariable("name");
		std::vector<std::string> available_plugins = {"script", "exe", "liscript"};
		auto it = std::find(available_plugins.begin(), available_plugins.end(), hubName->c_str());
		OATPP_ASSERT_HTTP(it != available_plugins.end(), Status::CODE_500, "Invalid plugin type");
		auto response = oatpp::websocket::Handshaker::serversideHandshake(request->getHeaders(), controller->websocketScriptConnectionHandler);
		auto parameters = std::make_shared<oatpp::network::ConnectionHandler::ParameterMap>();
		(*parameters)["pluginName"] = pluginName;
		(*parameters)["pluginHub"] = hubName;
		response->setConnectionUpgradeParameters(parameters);
		return response;
	};
#endif
};

#include OATPP_CODEGEN_END(ApiController) //<-- codegen end

#endif /* WebSocketController_hpp */
