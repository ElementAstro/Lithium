/*******************************************************************************
  Copyright(c) 2021 Chrysikos Efstathios. All rights reserved.

  Pegasus NYX

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.

  The full GNU General Public License is included in this distribution in the
  file called LICENSE.
*******************************************************************************/
#pragma once

#include "lx200generic.h"

class LX200NYX101 : public LX200Generic
{
public:
    LX200NYX101();
    virtual bool updateProperties() override;
    virtual bool initProperties() override;
    virtual bool ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n) override;
    virtual bool ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n) override;

protected:
    virtual bool ReadScopeStatus() override;
    virtual const char *getDefaultName() override;
    virtual bool Park() override;
    virtual bool UnPark() override;
    virtual bool updateLocation(double latitude, double longitude, double elevation) override;
    virtual bool setUTCOffset(double offset) override;
    virtual bool setLocalDate(uint8_t days, uint8_t months, uint16_t years) override;
    virtual bool SetTrackEnabled(bool enabled) override;
    virtual bool SetSlewRate(int index) override;

private:
    static constexpr const uint8_t SLEW_MODES{10};
    static constexpr const uint8_t DRIVER_LEN{64};
    static const char DRIVER_STOP_CHAR{0x23};
    static constexpr const uint8_t DRIVER_TIMEOUT{3};

    LITHIUM::PropertySwitch MountTypeSP{2};
    enum MountType
    {
        AltAz,
        Equatorial
    };
    LITHIUM::PropertySwitch GuideRateSP{3};
    LITHIUM::PropertySwitch HomeSP{1};
    LITHIUM::PropertySwitch ResetHomeSP{1};
    LITHIUM::PropertyText Report{1};
    LITHIUM::PropertySwitch VerboseReportSP{2};
    LITHIUM::PropertyText IsTracking{1};
    LITHIUM::PropertyText IsSlewCompleted{1};
    LITHIUM::PropertyText IsParked{1};
    LITHIUM::PropertyText IsParkginInProgress{1};
    LITHIUM::PropertyText IsAtHomePosition{1};
    LITHIUM::PropertyText TrackSidereal{1};
    LITHIUM::PropertyText TrackLunar{1};
    LITHIUM::PropertyText TrackSolar{1};
    LITHIUM::PropertyText MountAltAz{1};
    LITHIUM::PropertyText MountEquatorial{1};
    LITHIUM::PropertyText PierNone{1};
    LITHIUM::PropertyText PierEast{1};
    LITHIUM::PropertyText PierWest{1};
    LITHIUM::PropertyText DoesRefractionComp{1};
    LITHIUM::PropertyText WaitingAtHome{1};
    LITHIUM::PropertyText IsHomePaused{1};
    LITHIUM::PropertyText ParkFailed{1};
    LITHIUM::PropertyText SlewingHome{1};

    bool sendCommand(const char *cmd, char *res = nullptr, int cmd_len = -1, int res_len = -1);
    void hexDump(char *buf, const char *data, int size);
    std::vector<std::string> split(const std::string &input, const std::string &regex);
    bool goToPark();
    bool goToUnPark();
    bool setMountType(int type);
    bool setGuideRate(int rate);
    bool verboseReport = false;
    void SetPropertyText(LITHIUM::PropertyText propertyTxt, IPState state);
};
