/*
 * blob.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-3-16

Description: Blob property class

*************************************************/

#include "blob.hpp"
#include "blob_p.hpp"

namespace Atom::Driver
{

PropertyBlobPrivate::PropertyBlobPrivate(size_t count)
    : PropertyBasicPrivateTemplate<IBLOB>(count)
{ }

PropertyBlobPrivate::~PropertyBlobPrivate()
{
    for (auto &it: widgets)
    {
        auto blob = it.getBlob();
        if (blob != nullptr && deleter != nullptr)
        {
            deleter(blob);
        }
    }
}

PropertyBlob::PropertyBlob(size_t count)
    : PropertyBasic<IBLOB>(*new PropertyBlobPrivate(count))
{ }

PropertyBlob::PropertyBlob(Atom::Driver::Property property)
    : PropertyBasic<IBLOB>(property_private_cast<PropertyBlobPrivate>(property.d_ptr))
{ }

PropertyBlob::~PropertyBlob()
{ }

void PropertyBlob::setBlobDeleter(const std::function<void(void *&)> &deleter)
{
    D_PTR(PropertyBlob);
    d->deleter = deleter;
}

bool PropertyBlob::update(
    const int sizes[], const int blobsizes[], const char * const blobs[], const char * const formats[],
    const char * const names[], int n
)
{
    D_PTR(PropertyBlob);
    return d->typedProperty.update(sizes, blobsizes, blobs, formats, names, n) && (emitUpdate(), true);
}

void PropertyBlob::fill(
    const char *device, const char *name, const char *label, const char *group,
    IPerm permission, double timeout, IPState state
)
{
    D_PTR(PropertyBlob);
    d->typedProperty.setWidgets(d->widgets.data(), d->widgets.size());
    d->typedProperty.fill(device, name, label, group, permission, timeout, state);
}

}
