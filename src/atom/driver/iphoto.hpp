/*
 * iphoto.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: Photo type definition

**************************************************/

#pragma once

#include <string>

struct IPhoto
{
    IPhoto();
    std::string device_name;
    std::string device_uuid;
    std::string message_uuid;
    std::string name;
    int width;
    int height;
    int depth;

    int gain;
    int iso;
    int offset;
    int binning;

    double duration;

    bool is_color;

    std::string center_ra;
    std::string center_dec;

    std::string author;
    std::string time;
    std::string software = "Lithium-Server";

    const std::string toJson() const;
    const std::string toXml() const;
};