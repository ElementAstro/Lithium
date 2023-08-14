/*
 * iphoto.cpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
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

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-3-29

Description: Photo type definition

**************************************************/

#include "iphoto.hpp"
#include "uuid.hpp"

IPhoto::IPhoto()
{
    UUID::UUIDGenerator generator;
    message_uuid = generator.generateUUIDWithFormat();
}

const std::string IPhoto::toJson() const
{
    std::stringstream ss;
    ss << "{";
    ss << "\"device_name\":\"" << device_name << "\",";
    ss << "\"device_uuid\":\"" << device_uuid << "\",";
    ss << "\"message_uuid\":\"" << message_uuid << "\",";
    ss << "\"name\":\"" << name << "\",";
    ss << "\"value\":{";
    ss << "\"width\":" << width << ",";
    ss << "\"height\":" << height << ",";
    ss << "\"depth\":" << depth << ",";
    ss << "\"gain\":" << gain << ",";
    ss << "\"iso\":" << iso << ",";
    ss << "\"offset\":" << offset << ",";
    ss << "\"binning\":" << binning << ",";
    ss << "\"duration\":" << duration << ",";
    ss << "\"is_color\":" << std::boolalpha << is_color << ",";
    ss << "\"center_ra\":\"" << center_ra << "\",";
    ss << "\"center_dec\":\"" << center_dec << "\",";
    ss << "\"author\":\"" << author << "\",";
    ss << "\"time\":\"" << time << "\",";
    ss << "\"software\":\"" << software << "\"";
    ss << "}";
    ss << "}";
    return ss.str();
}

const std::string IPhoto::toXml() const
{
    std::stringstream ss;
    ss << "<message>";
    ss << "<device_name>" << device_name << "</device_name>";
    ss << "<device_uuid>" << device_uuid << "</device_uuid>";
    ss << "<message_uuid>" << message_uuid << "</message_uuid>";
    ss << "<name>" << name << "</name>";
    ss << "<value>";
    ss << "<width>" << width << "</width>";
    ss << "<height>" << height << "</height>";
    ss << "<depth>" << depth << "</depth>";
    ss << "<gain>" << gain << "</gain>";
    ss << "<iso>" << iso << "</iso>";
    ss << "<offset>" << offset << "</offset>";
    ss << "<binning>" << binning << "</binning>";
    ss << "<duration>" << duration << "</duration>";
    ss << "<is_color>" << std::boolalpha << is_color << "</is_color>";
    ss << "<center_ra>" << center_ra << "</center_ra>";
    ss << "<center_dec>" << center_dec << "</center_dec>";
    ss << "<author>" << author << "</author>";
    ss << "<time>" << time << "</time>";
    ss << "<software>" << software << "</software>";
    ss << "</value>";
    ss << "</message>";
    return ss.str();
}