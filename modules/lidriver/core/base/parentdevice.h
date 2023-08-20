#pragma once

#include "basedevice.h"

/**
 * @class LITHIUM::ParentDevice
 * @brief The class is used to create device instances.
 * Class copying is not allowed. When an object is destroyed,
 * the property list (LITHIUM::Property) is cleared to prevent a circular reference along with the properties.
 * The base LITHIUM::BaseDevice class and its LITHIUM::Properties exist as long as they are used by other objects.
 */

namespace LITHIUM
{

    class ParentDevicePrivate;
    class ParentDevice : public BaseDevice
    {
        DECLARE_PRIVATE(ParentDevice)

        ParentDevice(const ParentDevice &) = delete;
        ParentDevice &operator=(const ParentDevice &) = delete;

    public:
        enum Type
        {
            Valid,
            Invalid
        };

    public:
        explicit ParentDevice(Type type);
        ~ParentDevice();

    public:
        ParentDevice(ParentDevice &&other) = default;
        ParentDevice &operator=(ParentDevice &&other) = default;

    protected:
        ParentDevice(const std::shared_ptr<ParentDevicePrivate> &dd);
    };

}
