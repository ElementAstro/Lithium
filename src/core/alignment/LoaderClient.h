
#pragma once

#include "baseclient.h"
#include "basedevice.h"

#include "alignment/AlignmentSubsystemForClients.h"

class LoaderClient : public HYDROGEN::BaseClient, HYDROGEN::AlignmentSubsystem::AlignmentSubsystemForClients
{
    public:
        LoaderClient();
        virtual ~LoaderClient();

        // Public methods

        void Initialise(int argc, char *argv[]);
        void Load();

    protected:
        // Protected methods

        virtual void newBLOB(IBLOB *bp);
        virtual void newDevice(HYDROGEN::BaseDevice *dp);
        virtual void newLight(ILightVectorProperty *lvp) {}
        virtual void newMessage(HYDROGEN::BaseDevice *dp, int messageID) {}
        virtual void newNumber(INumberVectorProperty *nvp);
        virtual void newProperty(HYDROGEN::Property *property);
        virtual void newSwitch(ISwitchVectorProperty *svp);
        virtual void newText(ITextVectorProperty *tvp) {}
        virtual void removeProperty(HYDROGEN::Property *property) {}
        virtual void serverConnected() {}
        virtual void serverDisconnected(int exit_code) {}

    private:
        HYDROGEN::BaseDevice *Device;
        std::string DeviceName;
};
