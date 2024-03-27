/*
 * blob_p.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-3-16

Description: Blob property class

*************************************************/

#ifndef ATOM_DRIVER_PROPERTY_BLOB_P_HPP
#define ATOM_DRIVER_PROPERTY_BLOB_P_HPP

#include "propertybasic_p.hpp"

#include "basic_p.hpp"
#include "view.hpp"

namespace Atom::Driver {

class PropertyBlobPrivate : public PropertyBasicPrivateTemplate<IBLOB> {
public:
    PropertyBlobPrivate(size_t count);
    PropertyBlobPrivate(RawPropertyType *p) : BasicPropertyType(p) {}
    virtual ~PropertyBlobPrivate();

public:
    std::function<void(void *&)> deleter;
};

}  // namespace Atom::Driver
