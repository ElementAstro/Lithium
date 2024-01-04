/*
 * WebSocketController.cpp
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

Description: Websocket Route

**************************************************/

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

	ENDPOINT_ASYNC("GET", "ws", ws)
	{
		ENDPOINT_ASYNC_INIT(ws)
		Action act() override 
		{
			auto response = oatpp::websocket::Handshaker::serversideHandshake(request->getHeaders(), controller->websocketConnectionHandler);
			return _return(response);
		}
	};

	ENDPOINT_ASYNC("GET", "/ws/{hub-type}/{hub-name}/{instance-name}", wsConsole)
	{
		ENDPOINT_ASYNC_INIT(wsConsole)
		Action act() override 
		{
			auto hubType = request->getPathVariable("hub-type");
			auto hubName = request->getPathVariable("hub-name");
			auto instanceName = request->getPathVariable("instance-name");
			
			if (const std::string hub_type = hubType.getValue("");hub_type == "device")
			{
				std::vector<std::string> available_device_types = {"camera", "telescope", "focuser", "filterwheel","solver","guider"};
				auto it = std::find(available_device_types.begin(), available_device_types.end(), hubName.getValue(""));
				OATPP_ASSERT_HTTP(it != available_device_types.end(), Status::CODE_500, "Invalid device type");
				auto response = oatpp::websocket::Handshaker::serversideHandshake(request->getHeaders(), controller->websocketDeviceConnectionHandler);
				auto parameters = std::make_shared<oatpp::network::ConnectionHandler::ParameterMap>();
				(*parameters)["deviceName"] = hubName;
				(*parameters)["deviceHub"] = instanceName;
				response->setConnectionUpgradeParameters(parameters);
				return _return(response);
			}
			else if(hub_type == "plugin")
			{
				oatpp::String hubName = request->getPathVariable("hub-name");
				oatpp::String pluginName = request->getPathVariable("instance-name");
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
			else
			{
				OATPP_ASSERT_HTTP(false,Status::CODE_500,"Unknown tyoe of the Webwoskcet Instance or Hub");
			}
		}
	};
};

#include OATPP_CODEGEN_END(ApiController) //<-- codegen end

#endif /* WebSocketController_hpp */
