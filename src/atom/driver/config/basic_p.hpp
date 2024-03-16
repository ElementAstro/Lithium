/*
 * basic_p.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-3-16

Description: Basic property class

*************************************************/

#ifndef ATOM_DRIVER_PROPERTY_BASIC_P_HPP
#define ATOM_DRIVER_PROPERTY_BASIC_P_HPP

#include "property_p.hpp"
#include "view.hpp"

#include <functional>
#include <vector>

namespace Atom::Driver {

template <typename T>
struct PropertyContainer {
    PropertyView<T> &typedProperty;
};
template <typename T>
class PropertyBasicPrivateTemplate : public PropertyContainer<T>,
                                     public PropertyPrivate {
public:
    using RawPropertyType = typename WidgetTraits<T>::PropertyType;
    using BasicPropertyType = PropertyBasicPrivateTemplate<T>;

public:
    PropertyBasicPrivateTemplate(size_t count);
    PropertyBasicPrivateTemplate(RawPropertyType *rawProperty);
    virtual ~PropertyBasicPrivateTemplate();

public:
    bool raw;
    std::vector<WidgetView<T>> widgets;
};

}  // namespace Atom::Driver
