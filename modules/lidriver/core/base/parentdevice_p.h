#pragma once

#include "basedevice_p.h"
#include <atomic>

namespace LITHIUM
{

    class ParentDevicePrivate : public BaseDevicePrivate
    {
    public:
        std::atomic_int ref{0};
    };

}
