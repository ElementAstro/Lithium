/*******************************************************************************
  Copyright(c) 2016 Jasem Mutlaq. All rights reserved.

 HYDROGEN Qt Client

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

#include "abstractbaseclient.h"

#include <QObject>

/**
 * @class HYDROGEN::BaseClientQt
   @brief Class to provide basic client functionality based on Qt5 toolkit and is therefore suitable for cross-platform development.

   BaseClientQt enables accelerated development of HYDROGEN Clients by providing a framework that facilitates communication, device
   handling, and event notification. By subclassing BaseClientQt, clients can quickly connect to an HYDROGEN server, and query for
   a set of HYDROGEN::BaseDevice devices, and read and write properties seamlessly. Event driven programming is possible due to
   notifications upon reception of new devices or properties.

   @attention All notifications functions defined in HYDROGEN::BaseMediator <b>must</b> be implemented in the client class even if
   they are not used because these are pure virtual functions.

   @see <a href="http://indilib.org/develop/tutorials/107-client-development-tutorial.html">HYDROGEN Client Tutorial</a> for more details.
   @author Jasem Mutlaq

 */
namespace HYDROGEN
{
class BaseClientQtPrivate;
}
class HYDROGEN::BaseClientQt : public QObject, public HYDROGEN::AbstractBaseClient
{
        Q_OBJECT
        DECLARE_PRIVATE_D(d_ptr_hydrogen, BaseClientQt)

    public:
        BaseClientQt(QObject *parent = Q_NULLPTR);
        virtual ~BaseClientQt();

    public:
        /** @brief Connect to HYDROGEN server.
         *  @returns True if the connection is successful, false otherwise.
         *  @note This function blocks until connection is either successull or unsuccessful.
         */
        bool connectServer() override;

        /** @brief Disconnect from HYDROGEN server.
         *         Any devices previously created will be deleted and memory cleared.
         *  @return True if disconnection is successful, false otherwise.
         */
        bool disconnectServer(int exit_code = 0) override;
    
    private:
        void enableDirectBlobAccess(const char * dev = nullptr, const char * prop = nullptr) = delete; // not implemented
};
