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

#include "lithiumproperty.h"
#include "lithiumproperty_p.h"

#include "basedevice.h"

#include "lithiumpropertytext.h"
#include "lithiumpropertyswitch.h"
#include "lithiumpropertynumber.h"
#include "lithiumpropertylight.h"
#include "lithiumpropertyblob.h"

#include "lithiumpropertytext_p.h"
#include "lithiumpropertyswitch_p.h"
#include "lithiumpropertynumber_p.h"
#include "lithiumpropertylight_p.h"
#include "lithiumpropertyblob_p.h"

#include <cstdlib>
#include <cstring>

namespace LITHIUM
{

    PropertyPrivate::PropertyPrivate(void *property, LITHIUM_PROPERTY_TYPE type)
        : property(property), type(property ? type : LITHIUM_UNKNOWN), registered(property != nullptr)
    {
    }

    PropertyPrivate::PropertyPrivate(PropertyViewText *property)
        : property(property), type(property ? LITHIUM_TEXT : LITHIUM_UNKNOWN), registered(property != nullptr)
    {
    }

    PropertyPrivate::PropertyPrivate(PropertyViewNumber *property)
        : property(property), type(property ? LITHIUM_NUMBER : LITHIUM_UNKNOWN), registered(property != nullptr)
    {
    }

    PropertyPrivate::PropertyPrivate(PropertyViewSwitch *property)
        : property(property), type(property ? LITHIUM_SWITCH : LITHIUM_UNKNOWN), registered(property != nullptr)
    {
    }

    PropertyPrivate::PropertyPrivate(PropertyViewLight *property)
        : property(property), type(property ? LITHIUM_LIGHT : LITHIUM_UNKNOWN), registered(property != nullptr)
    {
    }

    PropertyPrivate::PropertyPrivate(PropertyViewBlob *property)
        : property(property), type(property ? LITHIUM_BLOB : LITHIUM_UNKNOWN), registered(property != nullptr)
    {
    }

#ifdef LITHIUM_PROPERTY_BACKWARD_COMPATIBILE
    LITHIUM::Property *Property::operator->()
    {
        return this;
    }

    const LITHIUM::Property *Property::operator->() const
    {
        return this;
    }

    Property::operator LITHIUM::Property *()
    {
        D_PTR(Property);
        return isValid() ? &d->self : nullptr;
    }

    Property::operator const LITHIUM::Property *() const
    {
        D_PTR(const Property);
        return isValid() ? &d->self : nullptr;
    }
#endif

    LITHIUM::Property *Property::self()
    {
        D_PTR(Property);
        return isValid() ? &d->self : nullptr;
    }

#define PROPERTY_CASE(CODE)                                             \
    switch (d->property != nullptr ? d->type : LITHIUM_UNKNOWN)         \
    {                                                                   \
    case LITHIUM_NUMBER:                                                \
    {                                                                   \
        auto property = static_cast<PropertyViewNumber *>(d->property); \
        CODE                                                            \
    }                                                                   \
    break;                                                              \
    case LITHIUM_TEXT:                                                  \
    {                                                                   \
        auto property = static_cast<PropertyViewText *>(d->property);   \
        CODE                                                            \
    }                                                                   \
    break;                                                              \
    case LITHIUM_SWITCH:                                                \
    {                                                                   \
        auto property = static_cast<PropertyViewSwitch *>(d->property); \
        CODE                                                            \
    }                                                                   \
    break;                                                              \
    case LITHIUM_LIGHT:                                                 \
    {                                                                   \
        auto property = static_cast<PropertyViewLight *>(d->property);  \
        CODE                                                            \
    }                                                                   \
    break;                                                              \
    case LITHIUM_BLOB:                                                  \
    {                                                                   \
        auto property = static_cast<PropertyViewBlob *>(d->property);   \
        CODE                                                            \
    }                                                                   \
    break;                                                              \
    default:;                                                           \
    }

    PropertyPrivate::~PropertyPrivate()
    {
        // Only delete properties if they were created dynamically via the buildSkeleton
        // function. Other drivers are responsible for their own memory allocation.
        if (property == nullptr || !dynamic)
            return;

        auto d = this;
        PROPERTY_CASE(delete property;)
    }

    Property::Property()
        : d_ptr(new PropertyPrivate(nullptr, LITHIUM_UNKNOWN))
    {
    }

    Property::Property(LITHIUM::PropertyNumber property)
        : d_ptr(property.d_ptr)
    {
    }

    Property::Property(LITHIUM::PropertyText property)
        : d_ptr(property.d_ptr)
    {
    }

    Property::Property(LITHIUM::PropertySwitch property)
        : d_ptr(property.d_ptr)
    {
    }

    Property::Property(LITHIUM::PropertyLight property)
        : d_ptr(property.d_ptr)
    {
    }

    Property::Property(LITHIUM::PropertyBlob property)
        : d_ptr(property.d_ptr)
    {
    }

#ifdef LITHIUM_PROPERTY_BACKWARD_COMPATIBILE

    Property::Property(LITHIUM::PropertyViewNumber *property)
        : d_ptr(new PropertyNumberPrivate(property))
    {
    }

    Property::Property(LITHIUM::PropertyViewText *property)
        : d_ptr(new PropertyTextPrivate(property))
    {
    }

    Property::Property(LITHIUM::PropertyViewSwitch *property)
        : d_ptr(new PropertySwitchPrivate(property))
    {
    }

    Property::Property(LITHIUM::PropertyViewLight *property)
        : d_ptr(new PropertyLightPrivate(property))
    {
    }

    Property::Property(LITHIUM::PropertyViewBlob *property)
        : d_ptr(new PropertyBlobPrivate(property))
    {
    }

    Property::Property(INumberVectorProperty *property)
        : d_ptr(new PropertyNumberPrivate(property))
    {
    }

    Property::Property(ITextVectorProperty *property)
        : d_ptr(new PropertyTextPrivate(property))
    {
    }

    Property::Property(ISwitchVectorProperty *property)
        : d_ptr(new PropertySwitchPrivate(property))
    {
    }

    Property::Property(ILightVectorProperty *property)
        : d_ptr(new PropertyLightPrivate(property))
    {
    }

    Property::Property(IBLOBVectorProperty *property)
        : d_ptr(new PropertyBlobPrivate(property))
    {
    }
#endif

    Property::~Property()
    {
    }

    Property::Property(PropertyPrivate &dd)
        : d_ptr(&dd)
    {
    }

    Property::Property(const std::shared_ptr<PropertyPrivate> &dd)
        : d_ptr(dd)
    {
    }

    void Property::setProperty(void *p)
    {
        D_PTR(Property);
        d->type = p ? d->type : LITHIUM_UNKNOWN;
        d->registered = p != nullptr;
        d->property = p;
    }

    void Property::setType(LITHIUM_PROPERTY_TYPE t)
    {
        D_PTR(Property);
        d->type = t;
    }

    void Property::setRegistered(bool r)
    {
        D_PTR(Property);
        d->registered = r;
    }

    void Property::setDynamic(bool dyn)
    {
        D_PTR(Property);
        d->dynamic = dyn;
    }

    void Property::setBaseDevice(BaseDevice *idp)
    {
        D_PTR(Property);
        d->baseDevice = (idp == nullptr ? BaseDevice() : *idp);
    }

    void Property::setBaseDevice(BaseDevice baseDevice)
    {
        D_PTR(Property);
        d->baseDevice = baseDevice;
    }

    void *Property::getProperty() const
    {
        D_PTR(const Property);
        return d->property;
    }

    LITHIUM_PROPERTY_TYPE Property::getType() const
    {
        D_PTR(const Property);
        return d->property != nullptr ? d->type : LITHIUM_UNKNOWN;
    }

    const char *Property::getTypeAsString() const
    {
        switch (getType())
        {
        case LITHIUM_NUMBER:
            return "LITHIUM_NUMBER";
        case LITHIUM_SWITCH:
            return "LITHIUM_SWITCH";
        case LITHIUM_TEXT:
            return "LITHIUM_TEXT";
        case LITHIUM_LIGHT:
            return "LITHIUM_LIGHT";
        case LITHIUM_BLOB:
            return "LITHIUM_BLOB";
        case LITHIUM_UNKNOWN:
            return "LITHIUM_UNKNOWN";
        }
        return "LITHIUM_UNKNOWN";
    }

    bool Property::getRegistered() const
    {
        D_PTR(const Property);
        return d->registered;
    }

    bool Property::isDynamic() const
    {
        D_PTR(const Property);
        return d->dynamic;
    }

    BaseDevice Property::getBaseDevice() const
    {
        D_PTR(const Property);
        return d->baseDevice;
    }

    void Property::setName(const char *name)
    {
        D_PTR(Property);
        PROPERTY_CASE(property->setName(name);)
    }

    void Property::setLabel(const char *label)
    {
        D_PTR(Property);
        PROPERTY_CASE(property->setLabel(label);)
    }

    void Property::setGroupName(const char *group)
    {
        D_PTR(Property);
        PROPERTY_CASE(property->setGroupName(group);)
    }

    void Property::setDeviceName(const char *device)
    {
        D_PTR(Property);
        PROPERTY_CASE(property->setDeviceName(device);)
    }

    void Property::setTimestamp(const char *timestamp)
    {
        D_PTR(Property);
        PROPERTY_CASE(property->setTimestamp(timestamp);)
    }

    void Property::setState(IPState state)
    {
        D_PTR(Property);
        PROPERTY_CASE(property->setState(state);)
    }

    void Property::setPermission(IPerm permission)
    {
        D_PTR(Property);
        PROPERTY_CASE(property->setPermission(permission);)
    }

    void Property::setTimeout(double timeout)
    {
        D_PTR(Property);
        PROPERTY_CASE(property->setTimeout(timeout);)
    }

    const char *Property::getName() const
    {
        D_PTR(const Property);
        PROPERTY_CASE(return property->getName();)
        return nullptr;
    }

    const char *Property::getLabel() const
    {
        D_PTR(const Property);
        PROPERTY_CASE(return property->getLabel();)
        return nullptr;
    }

    const char *Property::getGroupName() const
    {
        D_PTR(const Property);
        PROPERTY_CASE(return property->getGroupName();)
        return nullptr;
    }

    const char *Property::getDeviceName() const
    {
        D_PTR(const Property);
        PROPERTY_CASE(return property->getDeviceName();)
        return nullptr;
    }

    const char *Property::getTimestamp() const
    {
        D_PTR(const Property);
        PROPERTY_CASE(return property->getTimestamp();)
        return nullptr;
    }

    IPState Property::getState() const
    {
        D_PTR(const Property);
        PROPERTY_CASE(return property->getState();)
        return IPS_ALERT;
    }

    const char *Property::getStateAsString() const
    {
        return pstateStr(getState());
    }

    IPerm Property::getPermission() const
    {
        D_PTR(const Property);
        PROPERTY_CASE(return property->getPermission();)
        return IP_RO;
    }

    bool Property::isEmpty() const
    {
        D_PTR(const Property);
        PROPERTY_CASE(return property->isEmpty();)
        return true;
    }

    bool Property::isValid() const
    {
        D_PTR(const Property);
        return d->type != LITHIUM_UNKNOWN;
    }

    bool Property::isNameMatch(const char *otherName) const
    {
        D_PTR(const Property);
        PROPERTY_CASE(return property->isNameMatch(otherName);)
        return false;
    }

    bool Property::isNameMatch(const std::string &otherName) const
    {
        D_PTR(const Property);
        PROPERTY_CASE(return property->isNameMatch(otherName);)
        return false;
    }

    bool Property::isLabelMatch(const char *otherLabel) const
    {
        D_PTR(const Property);
        PROPERTY_CASE(return property->isLabelMatch(otherLabel);)
        return false;
    }

    bool Property::isLabelMatch(const std::string &otherLabel) const
    {
        D_PTR(const Property);
        PROPERTY_CASE(return property->isLabelMatch(otherLabel);)
        return false;
    }

    bool Property::isDeviceNameMatch(const char *otherDeviceName) const
    {
        return isDeviceNameMatch(std::string(otherDeviceName));
    }

    bool Property::isDeviceNameMatch(const std::string &otherDeviceName) const
    {
        return getDeviceName() == otherDeviceName;
    }

    bool Property::isTypeMatch(LITHIUM_PROPERTY_TYPE otherType) const
    {
        return getType() == otherType;
    }

    PropertyViewNumber *Property::getNumber() const
    {
        D_PTR(const Property);
        if (d->type == LITHIUM_NUMBER)
            return static_cast<PropertyViewNumber *>(d->property);

        return nullptr;
    }

    PropertyViewText *Property::getText() const
    {
        D_PTR(const Property);
        if (d->type == LITHIUM_TEXT)
            return static_cast<PropertyViewText *>(d->property);

        return nullptr;
    }

    PropertyViewLight *Property::getLight() const
    {
        D_PTR(const Property);
        if (d->type == LITHIUM_LIGHT)
            return static_cast<PropertyViewLight *>(d->property);

        return nullptr;
    }

    PropertyViewSwitch *Property::getSwitch() const
    {
        D_PTR(const Property);
        if (d->type == LITHIUM_SWITCH)
            return static_cast<PropertyViewSwitch *>(d->property);

        return nullptr;
    }

    PropertyViewBlob *Property::getBLOB() const
    {
        D_PTR(const Property);
        if (d->type == LITHIUM_BLOB)
            return static_cast<PropertyViewBlob *>(d->property);

        return nullptr;
    }

    bool Property::load()
    {
        D_PTR(const Property);
        PROPERTY_CASE(return property->load();)
        return false;
    }

    void Property::save(FILE *fp) const
    {
        D_PTR(const Property);
        PROPERTY_CASE(property->save(fp);)
    }

    void Property::apply(const char *format, ...) const
    {
        D_PTR(const Property);
        va_list ap;
        va_start(ap, format);
        PROPERTY_CASE(property->vapply(format, ap);)
        va_end(ap);
    }

    void Property::define(const char *format, ...) const
    {
        D_PTR(const Property);
        va_list ap;
        va_start(ap, format);
        PROPERTY_CASE(property->vdefine(format, ap);)
        va_end(ap);
    }

    void Property::onUpdate(const std::function<void()> &callback)
    {
        D_PTR(Property);
        d->onUpdateCallback = callback;
    }

    void Property::emitUpdate()
    {
        D_PTR(Property);
        if (d->onUpdateCallback)
            d->onUpdateCallback();
    }

    bool Property::hasUpdateCallback() const
    {
        D_PTR(const Property);
        return d->onUpdateCallback != nullptr;
    }

}
