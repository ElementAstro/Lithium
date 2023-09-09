/*******************************************************************************
  Copyright(c) 2011 Jasem Mutlaq. All rights reserved.

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

#pragma once

#include "util/macro.hpp"
#include "basedevice.h"
#include "lilxml.hpp"
#include "hydrogenbase.h"

#include <deque>
#include <string>
#include <mutex>
#include <map>
#include <functional>

#include "hydrogenpropertyblob.h"
#include "hydrogenlilxml.hpp"

namespace HYDROGEN
{

    class BaseDevice;
    class BaseDevicePrivate
    {
    public:
        BaseDevicePrivate();
        virtual ~BaseDevicePrivate();

        /** @brief Parse and store BLOB in the respective vector */
        int setBLOB(HYDROGEN::PropertyBlob propertyBlob, const HYDROGEN::LilXmlElement &root, char *errmsg);

        void emitWatchProperty(const HYDROGEN::Property &property, bool isNew)
        {
            auto it = watchPropertyMap.find(property.getName());
            if (it != watchPropertyMap.end())
            {
                if (
                    (it->second.watch == BaseDevice::WATCH_NEW_OR_UPDATE) ||
                    (it->second.watch == BaseDevice::WATCH_NEW && isNew) ||
                    (it->second.watch == BaseDevice::WATCH_UPDATE && !isNew))
                    it->second.callback(property);
            }
        }

        void addProperty(const HYDROGEN::Property &property)
        {
            {
                std::unique_lock<std::mutex> lock(m_Lock);
                pAll.push_back(property);
            }

            emitWatchProperty(property, true);
        }

    public: // mediator
        void mediateNewDevice(BaseDevice baseDevice)
        {
            if (mediator)
            {
                mediator->newDevice(baseDevice);
            }
        }

        void mediateRemoveDevice(BaseDevice baseDevice)
        {
            if (mediator)
            {
                mediator->removeDevice(baseDevice);
            }
        }

        void mediateNewProperty(Property property)
        {
            if (mediator)
            {
                mediator->newProperty(property);
            }
        }

        void mediateUpdateProperty(Property property)
        {
            emitWatchProperty(property, false);
            if (mediator)
            {
                mediator->updateProperty(property);
            }
        }

        void mediateRemoveProperty(Property property)
        {
            if (mediator)
            {
                mediator->removeProperty(property);
            }
        }

        void mediateNewMessage(BaseDevice baseDevice, int messageID)
        {
            if (mediator)
            {
                mediator->newMessage(baseDevice, messageID);
            }
        }

    public:
        static std::shared_ptr<BaseDevicePrivate> invalid()
        {
            static struct Invalid : public BaseDevicePrivate
            {
                Invalid() { valid = false; }
            } invalid;
            return make_shared_weak(&invalid);
        }

    public:
        struct WatchDetails
        {
            std::function<void(HYDROGEN::Property)> callback;
            BaseDevice::WATCH watch{BaseDevice::WATCH_NEW};
        };

    public:
        BaseDevice self{make_shared_weak(this)}; // backward compatibile (for operators as pointer)
        std::string deviceName;
        BaseDevice::Properties pAll;
        std::map<std::string, WatchDetails> watchPropertyMap;
        LilXmlParser xmlParser;

        HYDROGEN::BaseMediator *mediator{nullptr};
        std::deque<std::string> messageLog;
        mutable std::mutex m_Lock;

        bool valid{true};
    };

}
