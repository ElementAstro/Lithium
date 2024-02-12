/*
 * kmobject.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-10

Description: KMObject

*************************************************/

#ifndef ATOM_EVENT_OBJECT_HPP
#define ATOM_EVENT_OBJECT_HPP

#include <string>
#include <atomic>
#include <sstream>

namespace Atom::Event
{
    class KMObject
    {
    public:
        KMObject()
        {
            static std::atomic<long> s_objIdSeed{0};
            objId_ = ++s_objIdSeed;
        }

        const std::string &getObjKey() const
        {
            return objKey_;
        }

        long getObjId() const { return objId_; }

    protected:
        std::string objKey_;
        long objId_ = 0;
    };

#define KM_SetObjKey(x)           \
    do                            \
    {                             \
        std::stringstream ss;     \
        ss << x << "_" << objId_; \
        objKey_ = ss.str();       \
    } while (0)

} // namespace Atom::Event

#endif /* ATOM_EVENT_OBJECT_HPP */
