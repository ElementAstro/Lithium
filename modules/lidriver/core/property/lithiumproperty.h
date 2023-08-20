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

#include "lithiumbase.h"
#include "lithiumutility.h"

#include "lithiumpropertyview.h"

#include <memory>
#include <cstdarg>
#include <functional>

#define LITHIUM_PROPERTY_BACKWARD_COMPATIBILE
namespace LITHIUM
{
    class BaseDevice;
    class PropertyNumber;
    class PropertyText;
    class PropertySwitch;
    class PropertyLight;
    class PropertyBlob;
    /**
     * \class LITHIUM::Property
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
        Property(LITHIUM::PropertyNumber property);
        Property(LITHIUM::PropertyText property);
        Property(LITHIUM::PropertySwitch property);
        Property(LITHIUM::PropertyLight property);
        Property(LITHIUM::PropertyBlob property);

#ifdef LITHIUM_PROPERTY_BACKWARD_COMPATIBILE
    public:
        Property(INumberVectorProperty *property);
        Property(ITextVectorProperty *property);
        Property(ISwitchVectorProperty *property);
        Property(ILightVectorProperty *property);
        Property(IBLOBVectorProperty *property);

    public:
        Property(LITHIUM::PropertyViewNumber *property);
        Property(LITHIUM::PropertyViewText *property);
        Property(LITHIUM::PropertyViewSwitch *property);
        Property(LITHIUM::PropertyViewLight *property);
        Property(LITHIUM::PropertyViewBlob *property);

#endif
    public:
        void setProperty(void *);
        void setType(LITHIUM_PROPERTY_TYPE t);
        void setRegistered(bool r);
        void setDynamic(bool d);

        LITHIUM_DEPRECATED("Use setBaseDevice(BaseDevice).")
        void setBaseDevice(BaseDevice *idp);

        void setBaseDevice(BaseDevice device);

    public:
        void *getProperty() const;
        LITHIUM_PROPERTY_TYPE getType() const;
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

        bool isTypeMatch(LITHIUM_PROPERTY_TYPE otherType) const;

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
#ifdef LITHIUM_PROPERTY_BACKWARD_COMPATIBILE
        LITHIUM::PropertyViewNumber *getNumber() const;
        LITHIUM::PropertyViewText *getText() const;
        LITHIUM::PropertyViewSwitch *getSwitch() const;
        LITHIUM::PropertyViewLight *getLight() const;
        LITHIUM::PropertyViewBlob *getBLOB() const;
#endif

    public:
#ifdef LITHIUM_PROPERTY_BACKWARD_COMPATIBILE
        LITHIUM_DEPRECATED("Do not use LITHIUM::Property as pointer.")
        LITHIUM::Property *operator->();

        LITHIUM_DEPRECATED("Do not use LITHIUM::Property as pointer.")
        const LITHIUM::Property *operator->() const;

        LITHIUM_DEPRECATED("Do not use LITHIUM::Property as pointer.")
        operator LITHIUM::Property *();

        LITHIUM_DEPRECATED("Do not use LITHIUM::Property as pointer.")
        operator const LITHIUM::Property *() const;

        LITHIUM_DEPRECATED("Do not use LITHIUM::Property as pointer.")
        operator LITHIUM::PropertyViewNumber *() const
        {
            return getNumber();
        }

        LITHIUM_DEPRECATED("Do not use LITHIUM::Property as pointer.")
        operator LITHIUM::PropertyViewText *() const
        {
            return getText();
        }

        LITHIUM_DEPRECATED("Do not use LITHIUM::Property as pointer.")
        operator LITHIUM::PropertyViewSwitch *() const
        {
            return getSwitch();
        }

        LITHIUM_DEPRECATED("Do not use LITHIUM::Property as pointer.")
        operator LITHIUM::PropertyViewLight *() const
        {
            return getLight();
        }

        LITHIUM_DEPRECATED("Do not use LITHIUM::Property as pointer.")
        operator LITHIUM::PropertyViewBlob *() const
        {
            return getBLOB();
        }

        LITHIUM_DEPRECATED("Use comparison to true.")
        bool operator!=(std::nullptr_t) const
        {
            return isValid();
        }

        LITHIUM_DEPRECATED("Use comparison to false.")
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
        LITHIUM::Property *self();
    };

} // namespace LITHIUM

#ifdef QT_CORE_LIB
#include <QMetaType>
Q_DECLARE_METATYPE(LITHIUM::Property)
static int indi_property_metatype_id = QMetaTypeId<LITHIUM::Property>::qt_metatype_id();
#endif
