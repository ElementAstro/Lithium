#pragma once
#include <string>

#include "hydrogendevapi.h"

/* device + property name */
class Property
{
public:
    std::string dev;
    std::string name;
    BLOBHandling blob = B_NEVER; /* when to snoop BLOBs */

    Property(const std::string &dev, const std::string &name) : dev(dev), name(name) {}
};