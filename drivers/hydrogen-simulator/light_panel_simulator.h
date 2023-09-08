/*******************************************************************************
  Light Panel Simulator

  SPDX-FileCopyrightText: 2021 Jasem Mutlaq
  SPDX-License-Identifier: LGPL-2.0-or-later
*******************************************************************************/

#pragma once

#include "defaultdevice.h"
#include "hydrogenlightboxinterface.h"

class LightPanelSimulator : public HYDROGEN::DefaultDevice, public HYDROGEN::LightBoxInterface
{
    public:
        LightPanelSimulator();
        virtual ~LightPanelSimulator() override = default;

        void ISGetProperties(const char *dev) override;

        bool ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n) override;
        bool ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n) override;
        bool ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n) override;

    protected:

        bool initProperties() override;
        bool updateProperties() override;

        bool Connect() override
        {
            return true;
        }
        bool Disconnect() override
        {
            return true;
        }
        const char *getDefaultName() override
        {
            return "Light Panel Simulator";
        }

        // From Light Box
        virtual bool SetLightBoxBrightness(uint16_t value) override;
        virtual bool EnableLightBox(bool enable) override;
};
