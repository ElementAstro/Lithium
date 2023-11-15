/*
 * WsFocuserInstance.hpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.	If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-10-20

Description: WebSocket Device Instance (each device each instance)

**************************************************/

#ifndef WSFOCUSERINSTANCE_HPP
#define WSFOCUSERINSTANCE_HPP

#include "WsDeviceInstance.hpp"

/**
 * @brief Class representing an instance of a WebSocket Focuser
 *
 */
class WsFocuserInstance : public WsDeviceInstance
{

public:
	/**
	 * @brief Construct a new WsFocuserInstance object.
	 *
	 * @param socket Shared pointer to the AsyncWebSocket object.
	 * @param hub Shared pointer to the WsDeviceHub object.
	 * @param device_name Name of the device.
	 * @param userId Id of the user.
	 */
	WsFocuserInstance(const std::shared_ptr<AsyncWebSocket> &socket,
					 const std::shared_ptr<WsDeviceHub> &hub,
					 const oatpp::String &device_name,
					 v_int32 userId);

	/**
	 * @brief Destroy the WsFocuserInstance object.
	 *
	 */
	~WsFocuserInstance();

public:

};

#endif // WSFOCUSERINSTANCE_HPP
