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

#include "indiproperty.h"
#include "indiproperty_p.h"

#include "basedevice.h"

#include "indipropertyblob.h"
#include "indipropertylight.h"
#include "indipropertynumber.h"
#include "indipropertyswitch.h"
#include "indipropertytext.h"


#include "indipropertyblob_p.h"
#include "indipropertylight_p.h"
#include "indipropertynumber_p.h"
#include "indipropertyswitch_p.h"
#include "indipropertytext_p.h"


#include <cstdlib>
#include <cstring>

namespace Atom::Driver {

PropertyPrivate::PropertyPrivate(void *property, ATOM_PROPERTY_TYPE type)
    : property(property),
      type(property ? type : ATOM_UNKNOWN),
      registered(property != nullptr) {}

PropertyPrivate::PropertyPrivate(PropertyViewText *property)
    : property(property),
      type(property ? ATOM_TEXT : ATOM_UNKNOWN),
      registered(property != nullptr) {}

PropertyPrivate::PropertyPrivate(PropertyViewNumber *property)
    : property(property),
      type(property ? ATOM_NUMBER : ATOM_UNKNOWN),
      registered(property != nullptr) {}

PropertyPrivate::PropertyPrivate(PropertyViewSwitch *property)
    : property(property),
      type(property ? ATOM_SWITCH : ATOM_UNKNOWN),
      registered(property != nullptr) {}

PropertyPrivate::PropertyPrivate(PropertyViewLight *property)
    : property(property),
      type(property ? ATOM_LIGHT : ATOM_UNKNOWN),
      registered(property != nullptr) {}

PropertyPrivate::PropertyPrivate(PropertyViewBlob *property)
    : property(property),
      type(property ? ATOM_BLOB : ATOM_UNKNOWN),
      registered(property != nullptr) {}

Atom::Driver::Property *Property::self() {
    D_PTR(Property);
    return isValid() ? &d->self : nullptr;
}

#define PROPERTY_CASE(CODE)                                                 \
    switch (d->property != nullptr ? d->type : ATOM_UNKNOWN) {              \
        case ATOM_NUMBER: {                                                 \
            auto property = static_cast<PropertyViewNumber *>(d->property); \
            CODE                                                            \
        } break;                                                            \
        case ATOM_TEXT: {                                                   \
            auto property = static_cast<PropertyViewText *>(d->property);   \
            CODE                                                            \
        } break;                                                            \
        case ATOM_SWITCH: {                                                 \
            auto property = static_cast<PropertyViewSwitch *>(d->property); \
            CODE                                                            \
        } break;                                                            \
        case ATOM_LIGHT: {                                                  \
            auto property = static_cast<PropertyViewLight *>(d->property);  \
            CODE                                                            \
        } break;                                                            \
        case ATOM_BLOB: {                                                   \
            auto property = static_cast<PropertyViewBlob *>(d->property);   \
            CODE                                                            \
        } break;                                                            \
        default:;                                                           \
    }

PropertyPrivate::~PropertyPrivate() {
    // Only delete properties if they were created dynamically via the
    // buildSkeleton function. Other drivers are responsible for their own
    // memory allocation.
    if (property == nullptr || !dynamic)
        return;

    auto d = this;
    PROPERTY_CASE(delete property;)
}

Property::Property() : d_ptr(new PropertyPrivate(nullptr, ATOM_UNKNOWN)) {}

Property::Property(Atom::Driver::PropertyNumber property) : d_ptr(property.d_ptr) {}

Property::Property(Atom::Driver::PropertyText property) : d_ptr(property.d_ptr) {}

Property::Property(Atom::Driver::PropertySwitch property) : d_ptr(property.d_ptr) {}

Property::Property(Atom::Driver::PropertyLight property) : d_ptr(property.d_ptr) {}

Property::Property(Atom::Driver::PropertyBlob property) : d_ptr(property.d_ptr) {}

Property::~Property() {}

Property::Property(PropertyPrivate &dd) : d_ptr(&dd) {}

Property::Property(const std::shared_ptr<PropertyPrivate> &dd) : d_ptr(dd) {}

void Property::setProperty(void *p) {
    D_PTR(Property);
    d->type = p ? d->type : ATOM_UNKNOWN;
    d->registered = p != nullptr;
    d->property = p;
}

void Property::setType(ATOM_PROPERTY_TYPE t) {
    D_PTR(Property);
    d->type = t;
}

void Property::setRegistered(bool r) {
    D_PTR(Property);
    d->registered = r;
}

void Property::setDynamic(bool dyn) {
    D_PTR(Property);
    d->dynamic = dyn;
}

void Property::setBaseDevice(BaseDevice *idp) {
    D_PTR(Property);
    d->baseDevice = (idp == nullptr ? BaseDevice() : *idp);
}

void Property::setBaseDevice(BaseDevice baseDevice) {
    D_PTR(Property);
    d->baseDevice = baseDevice;
}

void *Property::getProperty() const {
    D_PTR(const Property);
    return d->property;
}

ATOM_PROPERTY_TYPE Property::getType() const {
    D_PTR(const Property);
    return d->property != nullptr ? d->type : ATOM_UNKNOWN;
}

std::string Property::getTypeAsString() const {
    switch (getType()) {
        case ATOM_NUMBER:
            return "ATOM_NUMBER";
        case ATOM_SWITCH:
            return "ATOM_SWITCH";
        case ATOM_TEXT:
            return "ATOM_TEXT";
        case ATOM_LIGHT:
            return "ATOM_LIGHT";
        case ATOM_BLOB:
            return "ATOM_BLOB";
        case ATOM_UNKNOWN:
            return "ATOM_UNKNOWN";
    }
    return "ATOM_UNKNOWN";
}

bool Property::getRegistered() const {
    D_PTR(const Property);
    return d->registered;
}

bool Property::isDynamic() const {
    D_PTR(const Property);
    return d->dynamic;
}

BaseDevice Property::getBaseDevice() const {
    D_PTR(const Property);
    return d->baseDevice;
}

void Property::setName(const std::string &name) {
    D_PTR(Property);
    PROPERTY_CASE(property->setName(name);)
}

void Property::setLabel(const std::string &label) {
    D_PTR(Property);
    PROPERTY_CASE(property->setLabel(label);)
}

void Property::setGroupName(const std::string &group) {
    D_PTR(Property);
    PROPERTY_CASE(property->setGroupName(group);)
}

void Property::setDeviceName(const std::string &device) {
    D_PTR(Property);
    PROPERTY_CASE(property->setDeviceName(device);)
}

void Property::setTimestamp(const std::string &timestamp) {
    D_PTR(Property);
    PROPERTY_CASE(property->setTimestamp(timestamp);)
}

void Property::setState(IPState state) {
    D_PTR(Property);
    PROPERTY_CASE(property->setState(state);)
}

void Property::setPermission(IPerm permission) {
    D_PTR(Property);
    PROPERTY_CASE(property->setPermission(permission);)
}

void Property::setTimeout(double timeout) {
    D_PTR(Property);
    PROPERTY_CASE(property->setTimeout(timeout);)
}

std::string Property::getName() const {
    D_PTR(const Property);
    PROPERTY_CASE(return property->getName();)
    return nullptr;
}

std::string Property::getLabel() const {
    D_PTR(const Property);
    PROPERTY_CASE(return property->getLabel();)
    return nullptr;
}

std::string Property::getGroupName() const {
    D_PTR(const Property);
    PROPERTY_CASE(return property->getGroupName();)
    return nullptr;
}

std::string Property::getDeviceName() const {
    D_PTR(const Property);
    PROPERTY_CASE(return property->getDeviceName();)
    return nullptr;
}

std::string Property::getTimestamp() const {
    D_PTR(const Property);
    PROPERTY_CASE(return property->getTimestamp();)
    return nullptr;
}

IPState Property::getState() const {
    D_PTR(const Property);
    PROPERTY_CASE(return property->getState();)
    return IPS_ALERT;
}

std::string Property::getStateAsString() const { return pstateStr(getState()); }

IPerm Property::getPermission() const {
    D_PTR(const Property);
    PROPERTY_CASE(return property->getPermission();)
    return IP_RO;
}

bool Property::isEmpty() const {
    D_PTR(const Property);
    PROPERTY_CASE(return property->isEmpty();)
    return true;
}

bool Property::isValid() const {
    D_PTR(const Property);
    return d->type != ATOM_UNKNOWN;
}

bool Property::isNameMatch(const std::string &otherName) const {
    D_PTR(const Property);
    PROPERTY_CASE(return property->isNameMatch(otherName);)
    return false;
}

bool Property::isLabelMatch(const std::string &otherLabel) const {
    D_PTR(const Property);
    PROPERTY_CASE(return property->isLabelMatch(otherLabel);)
    return false;
}

bool Property::isDeviceNameMatch(const std::string &otherDeviceName) const {
    return getDeviceName() == otherDeviceName;
}

bool Property::isTypeMatch(ATOM_PROPERTY_TYPE otherType) const {
    return getType() == otherType;
}

PropertyViewNumber *Property::getNumber() const {
    D_PTR(const Property);
    if (d->type == ATOM_NUMBER)
        return static_cast<PropertyViewNumber *>(d->property);

    return nullptr;
}

PropertyViewText *Property::getText() const {
    D_PTR(const Property);
    if (d->type == ATOM_TEXT)
        return static_cast<PropertyViewText *>(d->property);

    return nullptr;
}

PropertyViewLight *Property::getLight() const {
    D_PTR(const Property);
    if (d->type == ATOM_LIGHT)
        return static_cast<PropertyViewLight *>(d->property);

    return nullptr;
}

PropertyViewSwitch *Property::getSwitch() const {
    D_PTR(const Property);
    if (d->type == ATOM_SWITCH)
        return static_cast<PropertyViewSwitch *>(d->property);

    return nullptr;
}

PropertyViewBlob *Property::getBLOB() const {
    D_PTR(const Property);
    if (d->type == ATOM_BLOB)
        return static_cast<PropertyViewBlob *>(d->property);

    return nullptr;
}

bool Property::load() {
    D_PTR(const Property);
    PROPERTY_CASE(return property->load();)
    return false;
}

void Property::save(FILE *fp) const {
    D_PTR(const Property);
    PROPERTY_CASE(property->save(fp);)
}

void Property::apply(const std::string &format, ...) const {
    D_PTR(const Property);
    va_list ap;
    va_start(ap, format);
    PROPERTY_CASE(property->vapply(format, ap);)
    va_end(ap);
}

void Property::define(const std::string &format, ...) const {
    D_PTR(const Property);
    va_list ap;
    va_start(ap, format);
    PROPERTY_CASE(property->vdefine(format, ap);)
    va_end(ap);
}

void Property::onUpdate(const std::function<void()> &callback) {
    D_PTR(Property);
    d->onUpdateCallback = callback;
}

void Property::emitUpdate() {
    D_PTR(Property);
    if (d->onUpdateCallback)
        d->onUpdateCallback();
}

bool Property::hasUpdateCallback() const {
    D_PTR(const Property);
    return d->onUpdateCallback != nullptr;
}

}  // namespace Atom::Driver
