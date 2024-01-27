/*
 * iproperty.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: Property type definition

**************************************************/

#include "iproperty.hpp"
#include "atom/utils/uuid.hpp"

IPropertyBase::IPropertyBase()
{
    message_uuid = Atom::Property::UUIDGenerator::generateUUIDWithFormat();
}

INumberProperty::INumberProperty() : IPropertyBase()
{
}

IStringProperty::IStringProperty() : IPropertyBase()
{
}

IBoolProperty::IBoolProperty() : IPropertyBase()
{
}

INumberVector::INumberVector() : IPropertyBase()
{
}