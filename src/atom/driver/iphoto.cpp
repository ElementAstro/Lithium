/*
 * iphoto.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: Photo type definition

**************************************************/

#include "iphoto.hpp"
#include "atom/utils/uuid.hpp"

IPhoto::IPhoto()
{
    message_uuid = Atom::Property::UUIDGenerator::generateUUIDWithFormat();
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