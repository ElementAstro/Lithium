/*******************************************************************************
  Copyright(c) 2011 Jasem Mutlaq. All rights reserved.
               2022 Pawel Soja <kernel32.pl@gmail.com>

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Library General Public
 License version 2 as published by the Free Software Foundation.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Library General Public License for more details.

 You should have received a copy of the GNU Library General Public License
 along with this library; see the file COPYING.LIB.  If not, write to
 the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 Boston, MA 02110-1301, USA.
*******************************************************************************/

#pragma once

#include "hydrogenbase.h"
#include "hydrogenutility.hpp"

#include "hydrogenpropertyview.h"

#include <memory>
#include <cstdarg>
#include <functional>

#define HYDROGEN_PROPERTY_BACKWARD_COMPATIBILE
namespace HYDROGEN
{
    class BaseDevice;
    class PropertyNumber;
    class PropertyText;
    class PropertySwitch;
    class PropertyLight;
    class PropertyBlob;
    /**
     * \class HYDROGEN::Property
       \brief Provides generic container for INDI properties

    \author Jasem Mutlaq
    */
    class PropertyPrivate;
    class Property
    {
        DECLARE_PRIVATE(Property)
    public:
        Property();
        ~Property();

    public:
        Property(HYDROGEN::PropertyNumber property);
        Property(HYDROGEN::PropertyText property);
        Property(HYDROGEN::PropertySwitch property);
        Property(HYDROGEN::PropertyLight property);
        Property(HYDROGEN::PropertyBlob property);

#ifdef HYDROGEN_PROPERTY_BACKWARD_COMPATIBILE
    public:
        Property(INumberVectorProperty *property);
        Property(ITextVectorProperty *property);
        Property(ISwitchVectorProperty *property);
        Property(ILightVectorProperty *property);
        Property(IBLOBVectorProperty *property);

    public:
        Property(HYDROGEN::PropertyViewNumber *property);
        Property(HYDROGEN::PropertyViewText *property);
        Property(HYDROGEN::PropertyViewSwitch *property);
        Property(HYDROGEN::PropertyViewLight *property);
        Property(HYDROGEN::PropertyViewBlob *property);

#endif
    public:
        void setProperty(void *);
        void setType(HYDROGEN_PROPERTY_TYPE t);
        void setRegistered(bool r);
        void setDynamic(bool d);

        HYDROGEN_DEPRECATED("Use setBaseDevice(BaseDevice).")
        void setBaseDevice(BaseDevice *idp);

        void setBaseDevice(BaseDevice device);

    public:
        void *getProperty() const;
        HYDROGEN_PROPERTY_TYPE getType() const;
        const char *getTypeAsString() const;
        bool getRegistered() const;
        bool isDynamic() const;
        BaseDevice getBaseDevice() const;

    public: // Convenience Functions
        void setName(const char *name);
        void setLabel(const char *label);
        void setGroupName(const char *groupName);
        void setDeviceName(const char *deviceName);
        void setTimestamp(const char *timestamp);
        void setState(IPState state);
        void setPermission(IPerm permission);
        void setTimeout(double timeout);

    public: // Convenience Functions
        const char *getName() const;
        const char *getLabel() const;
        const char *getGroupName() const;
        const char *getDeviceName() const;
        const char *getTimestamp() const;
        IPState getState() const;
        const char *getStateAsString() const;
        IPerm getPermission() const;

    public:
        bool isEmpty() const;
        bool isValid() const;

        bool isNameMatch(const char *otherName) const;
        bool isNameMatch(const std::string &otherName) const;

        bool isLabelMatch(const char *otherLabel) const;
        bool isLabelMatch(const std::string &otherLabel) const;

        bool isDeviceNameMatch(const char *otherDeviceName) const;
        bool isDeviceNameMatch(const std::string &otherDeviceName) const;

        bool isTypeMatch(HYDROGEN_PROPERTY_TYPE otherType) const;

    public:
        void onUpdate(const std::function<void()> &callback);

    public:
        void emitUpdate();
        bool hasUpdateCallback() const;

    public:
        bool load();
        void save(FILE *fp) const;

    public:
        void apply(const char *format, ...) const ATTRIBUTE_FORMAT_PRINTF(2, 3);
        void define(const char *format, ...) const ATTRIBUTE_FORMAT_PRINTF(2, 3);

        void apply() const
        {
            apply(nullptr);
        }
        void define() const
        {
            define(nullptr);
        }

    public:
#ifdef HYDROGEN_PROPERTY_BACKWARD_COMPATIBILE
        HYDROGEN::PropertyViewNumber *getNumber() const;
        HYDROGEN::PropertyViewText *getText() const;
        HYDROGEN::PropertyViewSwitch *getSwitch() const;
        HYDROGEN::PropertyViewLight *getLight() const;
        HYDROGEN::PropertyViewBlob *getBLOB() const;
#endif

    public:
#ifdef HYDROGEN_PROPERTY_BACKWARD_COMPATIBILE
        HYDROGEN_DEPRECATED("Do not use HYDROGEN::Property as pointer.")
        HYDROGEN::Property *operator->();

        HYDROGEN_DEPRECATED("Do not use HYDROGEN::Property as pointer.")
        const HYDROGEN::Property *operator->() const;

        HYDROGEN_DEPRECATED("Do not use HYDROGEN::Property as pointer.")
        operator HYDROGEN::Property *();

        HYDROGEN_DEPRECATED("Do not use HYDROGEN::Property as pointer.")
        operator const HYDROGEN::Property *() const;

        HYDROGEN_DEPRECATED("Do not use HYDROGEN::Property as pointer.")
        operator HYDROGEN::PropertyViewNumber *() const
        {
            return getNumber();
        }

        HYDROGEN_DEPRECATED("Do not use HYDROGEN::Property as pointer.")
        operator HYDROGEN::PropertyViewText *() const
        {
            return getText();
        }

        HYDROGEN_DEPRECATED("Do not use HYDROGEN::Property as pointer.")
        operator HYDROGEN::PropertyViewSwitch *() const
        {
            return getSwitch();
        }

        HYDROGEN_DEPRECATED("Do not use HYDROGEN::Property as pointer.")
        operator HYDROGEN::PropertyViewLight *() const
        {
            return getLight();
        }

        HYDROGEN_DEPRECATED("Do not use HYDROGEN::Property as pointer.")
        operator HYDROGEN::PropertyViewBlob *() const
        {
            return getBLOB();
        }

        HYDROGEN_DEPRECATED("Use comparison to true.")
        bool operator!=(std::nullptr_t) const
        {
            return isValid();
        }

        HYDROGEN_DEPRECATED("Use comparison to false.")
        bool operator==(std::nullptr_t) const
        {
            return !isValid();
        }

        operator bool() const
        {
            return isValid();
        }
        operator bool()
        {
            return isValid();
        }
#endif

    protected:
        std::shared_ptr<PropertyPrivate> d_ptr;
        Property(const std::shared_ptr<PropertyPrivate> &dd);
        Property(PropertyPrivate &dd);
        friend class PropertyNumber;
        friend class PropertyText;
        friend class PropertySwitch;
        friend class PropertyLight;
        friend class PropertyBlob;

    protected:
        friend class BaseDevicePrivate;
        HYDROGEN::Property *self();
    };

} // namespace HYDROGEN

#ifdef QT_CORE_LIB
#include <QMetaType>
Q_DECLARE_METATYPE(HYDROGEN::Property)
static int indi_property_metatype_id = QMetaTypeId<HYDROGEN::Property>::qt_metatype_id();
#endif
