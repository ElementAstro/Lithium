#pragma once

#include "iproperty.hpp"

class IPhoto : public IProperty
{
public:
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

    std::string toJson() const override;
    std::string toXml() const override;
};