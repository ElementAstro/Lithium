/*!
 * \file ClientAPIForMathPluginManagement.h
 *
 * \author Roger James
 * \date 13th November 2013
 *
 */

#pragma once

#include "basedevice.h"
#include "baseclient.h"

#include <string>
#include <vector>
#include <pthread.h>

namespace HYDROGEN
{
namespace AlignmentSubsystem
{
/*!
 * \class ClientAPIForMathPluginManagement
 * \brief This class provides the client API for driver side math plugin management. It communicates
 * with the driver via the HYDROGEN properties interface.
 */
class ClientAPIForMathPluginManagement
{
    public:
        /// \brief Virtual destructor
        virtual ~ClientAPIForMathPluginManagement() {}

        typedef std::vector<std::string> MathPluginsList;

        // Public methods

        /*!
             * \brief Return a list of the names of the available math plugins.
             * \param[out] AvailableMathPlugins Reference to a list of the names of the available math plugins.
             * \return False on failure
             */
        bool EnumerateMathPlugins(MathPluginsList &AvailableMathPlugins);

        /// \brief Intialise the API
        /// \param[in] BaseClient Pointer to the HYDROGEN:BaseClient class
        void Initialise(HYDROGEN::BaseClient *BaseClient);

        /** \brief Process new device message from driver. This routine should be called from within
             the newDevice handler in the client. This routine is not normally called directly but is called by
             the ProcessNewDevice function in HYDROGEN::Alignment::AlignmentSubsystemForClients which filters out calls
             from unwanted devices. TODO maybe hide this function.
                \param[in] DevicePointer A pointer to the HYDROGEN::BaseDevice object.
            */
        void ProcessNewDevice(HYDROGEN::BaseDevice *DevicePointer);

        /** \brief Process new property message from driver. This routine should be called from within
             the newProperty handler in the client. This routine is not normally called directly but is called by
             the ProcessNewProperty function in HYDROGEN::Alignment::AlignmentSubsystemForClients which filters out calls
             from unwanted devices. TODO maybe hide this function.
                \param[in] PropertyPointer A pointer to the HYDROGEN::Property object.
            */
        void ProcessNewProperty(HYDROGEN::Property *PropertyPointer);

        /** \brief Process new switch message from driver. This routine should be called from within
             the newSwitch handler in the client. This routine is not normally called directly but is called by
             the ProcessNewSwitch function in HYDROGEN::Alignment::AlignmentSubsystemForClients which filters out calls
             from unwanted devices. TODO maybe hide this function.
                \param[in] SwitchVectorProperty A pointer to the HYDROGEN::ISwitchVectorProperty.
            */
        void ProcessNewSwitch(ISwitchVectorProperty *SwitchVectorProperty);

        /*!
             * \brief Selects, loads and initialises the named math plugin.
             * \param[in] MathPluginName The name of the required math plugin.
             * \return False on failure.
             */
        bool SelectMathPlugin(const std::string &MathPluginName);

        /*!
             * \brief Re-initialises the current math plugin.
             * \return False on failure.
             */
        bool ReInitialiseMathPlugin();

    private:
        // Private methods

        bool SetDriverBusy();
        bool SignalDriverCompletion();
        bool WaitForDriverCompletion();

        // Private properties

        HYDROGEN::BaseClient *BaseClient;
        pthread_cond_t DriverActionCompleteCondition;
        pthread_mutex_t DriverActionCompleteMutex;
        bool DriverActionComplete;
        HYDROGEN::BaseDevice *Device;
        HYDROGEN::Property *MathPlugins;
        HYDROGEN::Property *PluginInitialise;
};

} // namespace AlignmentSubsystem
} // namespace HYDROGEN
