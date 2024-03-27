/*
 * blob.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-3-16

Description: Blob property class

*************************************************/

#pragma once

#include "indipropertybasic.h"

namespace INDI {

class PropertyBlobPrivate;
class PropertyBlob : public INDI::PropertyBasic<IBLOB> {
    DECLARE_PRIVATE(PropertyBlob)
public:
    PropertyBlob(size_t count);
    PropertyBlob(INDI::Property property);
    ~PropertyBlob();

public:
    /**
     * @brief Set the Blob Deleter function
     * You can define a function to release the memory of the elements.
     * The function will be executed when the PropertyBlob is destroyed
     *
     * @param deleter function to release the memory of a given item
     */
    void setBlobDeleter(const std::function<void(void *&)> &deleter);

public:
    bool update(const int sizes[], const int blobsizes[],
                const char *const blobs[], const char *const formats[],
                const char *const names[], int n);

    void fill(const char *device, const char *name, const char *label,
              const char *group, IPerm permission, double timeout,
              IPState state);
};

}  // namespace INDI
