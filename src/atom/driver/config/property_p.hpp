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

#ifndef ATOM_DRIVER_CONFIG_PROPERTY_PROPERTY_P_HPP
#define ATOM_DRIVER_CONFIG_PROPERTY_PROPERTY_P_HPP

#include <functional>
#include <memory>
#include "basedevice.h"
#include "indibase.h"
#include "indimacros.h"
#include "indiproperty.h"

namespace Atom::Driver {

template <typename T, typename U>
static inline std::shared_ptr<T> property_private_cast(
    const std::shared_ptr<U> &r) {
    static struct Invalid : public T {
        Invalid() : T(16) { this->type = ATOM_UNKNOWN; }
    } invalid;
    auto result = std::dynamic_pointer_cast<T>(r);
    return result != nullptr ? result : make_shared_weak(&invalid);
}

class BaseDevice;
class PropertyPrivate {
public:
    PropertyPrivate(void *property, ATOM_PROPERTY_TYPE type);
    PropertyPrivate(PropertyViewText *property);
    PropertyPrivate(PropertyViewNumber *property);
    PropertyPrivate(PropertyViewSwitch *property);
    PropertyPrivate(PropertyViewLight *property);
    PropertyPrivate(PropertyViewBlob *property);

    virtual ~PropertyPrivate();

    void *property = nullptr;
    BaseDevice baseDevice;
    ATOM_PROPERTY_TYPE type = ATOM_UNKNOWN;
    bool registered = false;
    bool dynamic = false;

    std::function<void()> onUpdateCallback;
};

}  // namespace Atom::Driver

#endif
