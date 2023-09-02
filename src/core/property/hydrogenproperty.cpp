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

#include "hydrogenproperty.h"
#include "hydrogenproperty_p.h"

#include "basedevice.h"

#include "hydrogenpropertytext.h"
#include "hydrogenpropertyswitch.h"
#include "hydrogenpropertynumber.h"
#include "hydrogenpropertylight.h"
#include "hydrogenpropertyblob.h"

#include "hydrogenpropertytext_p.h"
#include "hydrogenpropertyswitch_p.h"
#include "hydrogenpropertynumber_p.h"
#include "hydrogenpropertylight_p.h"
#include "hydrogenpropertyblob_p.h"

#include <cstdlib>
#include <cstring>

namespace HYDROGEN
{

    PropertyPrivate::PropertyPrivate(void *property, HYDROGEN_PROPERTY_TYPE type)
        : property(property), type(property ? type : HYDROGEN_UNKNOWN), registered(property != nullptr)
    {
    }

    PropertyPrivate::PropertyPrivate(PropertyViewText *property)
        : property(property), type(property ? HYDROGEN_TEXT : HYDROGEN_UNKNOWN), registered(property != nullptr)
    {
    }

    PropertyPrivate::PropertyPrivate(PropertyViewNumber *property)
        : property(property), type(property ? HYDROGEN_NUMBER : HYDROGEN_UNKNOWN), registered(property != nullptr)
    {
    }

    PropertyPrivate::PropertyPrivate(PropertyViewSwitch *property)
        : property(property), type(property ? HYDROGEN_SWITCH : HYDROGEN_UNKNOWN), registered(property != nullptr)
    {
    }

    PropertyPrivate::PropertyPrivate(PropertyViewLight *property)
        : property(property), type(property ? HYDROGEN_LIGHT : HYDROGEN_UNKNOWN), registered(property != nullptr)
    {
    }

    PropertyPrivate::PropertyPrivate(PropertyViewBlob *property)
        : property(property), type(property ? HYDROGEN_BLOB : HYDROGEN_UNKNOWN), registered(property != nullptr)
    {
    }

#ifdef HYDROGEN_PROPERTY_BACKWARD_COMPATIBILE
    HYDROGEN::Property *Property::operator->()
    {
        return this;
    }

    const HYDROGEN::Property *Property::operator->() const
    {
        return this;
    }

    Property::operator HYDROGEN::Property *()
    {
        D_PTR(Property);
        return isValid() ? &d->self : nullptr;
    }

    Property::operator const HYDROGEN::Property *() const
    {
        D_PTR(const Property);
        return isValid() ? &d->self : nullptr;
    }
#endif

    HYDROGEN::Property *Property::self()
    {
        D_PTR(Property);
        return isValid() ? &d->self : nullptr;
    }

#define PROPERTY_CASE(CODE)                                             \
    switch (d->property != nullptr ? d->type : HYDROGEN_UNKNOWN)         \
    {                                                                   \
    case HYDROGEN_NUMBER:                                                \
    {                                                                   \
        auto property = static_cast<PropertyViewNumber *>(d->property); \
        CODE                                                            \
    }                                                                   \
    break;                                                              \
    case HYDROGEN_TEXT:                                                  \
    {                                                                   \
        auto property = static_cast<PropertyViewText *>(d->property);   \
        CODE                                                            \
    }                                                                   \
    break;                                                              \
    case HYDROGEN_SWITCH:                                                \
    {                                                                   \
        auto property = static_cast<PropertyViewSwitch *>(d->property); \
        CODE                                                            \
    }                                                                   \
    break;                                                              \
    case HYDROGEN_LIGHT:                                                 \
    {                                                                   \
        auto property = static_cast<PropertyViewLight *>(d->property);  \
        CODE                                                            \
    }                                                                   \
    break;                                                              \
    case HYDROGEN_BLOB:                                                  \
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
        : d_ptr(new PropertyPrivate(nullptr, HYDROGEN_UNKNOWN))
    {
    }

    Property::Property(HYDROGEN::PropertyNumber property)
        : d_ptr(property.d_ptr)
    {
    }

    Property::Property(HYDROGEN::PropertyText property)
        : d_ptr(property.d_ptr)
    {
    }

    Property::Property(HYDROGEN::PropertySwitch property)
        : d_ptr(property.d_ptr)
    {
    }

    Property::Property(HYDROGEN::PropertyLight property)
        : d_ptr(property.d_ptr)
    {
    }

    Property::Property(HYDROGEN::PropertyBlob property)
        : d_ptr(property.d_ptr)
    {
    }

#ifdef HYDROGEN_PROPERTY_BACKWARD_COMPATIBILE

    Property::Property(HYDROGEN::PropertyViewNumber *property)
        : d_ptr(new PropertyNumberPrivate(property))
    {
    }

    Property::Property(HYDROGEN::PropertyViewText *property)
        : d_ptr(new PropertyTextPrivate(property))
    {
    }

    Property::Property(HYDROGEN::PropertyViewSwitch *property)
        : d_ptr(new PropertySwitchPrivate(property))
    {
    }

    Property::Property(HYDROGEN::PropertyViewLight *property)
        : d_ptr(new PropertyLightPrivate(property))
    {
    }

    Property::Property(HYDROGEN::PropertyViewBlob *property)
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
        d->type = p ? d->type : HYDROGEN_UNKNOWN;
        d->registered = p != nullptr;
        d->property = p;
    }

    void Property::setType(HYDROGEN_PROPERTY_TYPE t)
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

    HYDROGEN_PROPERTY_TYPE Property::getType() const
    {
        D_PTR(const Property);
        return d->property != nullptr ? d->type : HYDROGEN_UNKNOWN;
    }

    const char *Property::getTypeAsString() const
    {
        switch (getType())
        {
        case HYDROGEN_NUMBER:
            return "HYDROGEN_NUMBER";
        case HYDROGEN_SWITCH:
            return "HYDROGEN_SWITCH";
        case HYDROGEN_TEXT:
            return "HYDROGEN_TEXT";
        case HYDROGEN_LIGHT:
            return "HYDROGEN_LIGHT";
        case HYDROGEN_BLOB:
            return "HYDROGEN_BLOB";
        case HYDROGEN_UNKNOWN:
            return "HYDROGEN_UNKNOWN";
        }
        return "HYDROGEN_UNKNOWN";
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
        return d->type != HYDROGEN_UNKNOWN;
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

    bool Property::isTypeMatch(HYDROGEN_PROPERTY_TYPE otherType) const
    {
        return getType() == otherType;
    }

    PropertyViewNumber *Property::getNumber() const
    {
        D_PTR(const Property);
        if (d->type == HYDROGEN_NUMBER)
            return static_cast<PropertyViewNumber *>(d->property);

        return nullptr;
    }

    PropertyViewText *Property::getText() const
    {
        D_PTR(const Property);
        if (d->type == HYDROGEN_TEXT)
            return static_cast<PropertyViewText *>(d->property);

        return nullptr;
    }

    PropertyViewLight *Property::getLight() const
    {
        D_PTR(const Property);
        if (d->type == HYDROGEN_LIGHT)
            return static_cast<PropertyViewLight *>(d->property);

        return nullptr;
    }

    PropertyViewSwitch *Property::getSwitch() const
    {
        D_PTR(const Property);
        if (d->type == HYDROGEN_SWITCH)
            return static_cast<PropertyViewSwitch *>(d->property);

        return nullptr;
    }

    PropertyViewBlob *Property::getBLOB() const
    {
        D_PTR(const Property);
        if (d->type == HYDROGEN_BLOB)
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
