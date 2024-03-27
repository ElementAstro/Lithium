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

#include "indibase.h"
#include "indiutility.h"

#include "indipropertyview.h"

#include <memory>
#include <cstdarg>
#include <functional>

namespace Atom::Driver
{
class BaseDevice;
class PropertyNumber;
class PropertyText;
class PropertySwitch;
class PropertyLight;
class PropertyBlob;
/**
 * \class Atom::Driver::Property
   \brief Provides generic container for Atom::Driver properties

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
        Property(Atom::Driver::PropertyNumber property);
        Property(Atom::Driver::PropertyText   property);
        Property(Atom::Driver::PropertySwitch property);
        Property(Atom::Driver::PropertyLight  property);
        Property(Atom::Driver::PropertyBlob   property);

    public:
        void setProperty(void *);
        void setType(ATOM_PROPERTY_TYPE t);
        void setRegistered(bool r);
        void setDynamic(bool d);

        void setBaseDevice(BaseDevice device);

    public:
        void *getProperty() const;
        ATOM_PROPERTY_TYPE getType() const;
        std::string getTypeAsString() const;
        bool getRegistered() const;
        bool isDynamic() const;
        BaseDevice getBaseDevice() const;

    public: // Convenience Functions
        void setName(const std::string &name);
        void setLabel(const std::string &label);
        void setGroupName(const std::string &groupName);
        void setDeviceName(const std::string &deviceName);
        void setTimestamp(const std::string &timestamp);
        void setState(IPState state);
        void setPermission(IPerm permission);
        void setTimeout(double timeout);

    public: // Convenience Functions
        std::string getName() const;
        std::string getLabel() const;
        std::string getGroupName() const;
        std::string getDeviceName() const;
        std::string getTimestamp() const;
        IPState getState() const;
        std::string getStateAsString() const;
        IPerm getPermission() const;

    public:
        bool isEmpty() const;
        bool isValid() const;

        bool isNameMatch(const std::string &otherName) const;

        bool isLabelMatch(const std::string &otherLabel) const;

        bool isDeviceNameMatch(const std::string &otherDeviceName) const;

        bool isTypeMatch(ATOM_PROPERTY_TYPE otherType) const;

    public:
        void onUpdate(const std::function<void()> &callback);

    public:
        void emitUpdate();
        bool hasUpdateCallback() const;

    public:
        bool load();
        void save(FILE *fp) const;

    public:
        void apply(const std::string &format, ...) const ATTRIBUTE_FORMAT_PRINTF(2, 3);
        void define(const std::string &format, ...) const ATTRIBUTE_FORMAT_PRINTF(2, 3);

        void apply() const
        {
            apply(nullptr);
        }
        void define() const
        {
            define(nullptr);
        }

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
        Atom::Driver::Property *self();
};

} // namespace Atom::Driver