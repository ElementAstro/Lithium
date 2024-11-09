/*
 * focuser.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-1

Description: AtomTelescope Simulator and Basic Definition

*************************************************/

#pragma once

#include "device.hpp"

#include <optional>

enum class ConnectionMode { SERIAL, TCP, NONE };

enum class T_BAUD_RATE { B9600, B19200, B38400, B57600, B115200, B230400, NONE };

enum class TrackMode { SIDEREAL, SOLAR, LUNAR, CUSTOM, NONE };

enum class PierSide { EAST, WEST, NONE };

enum class ParkOptions { CURRENT, DEFAULT, WRITE_DATA, PURGE_DATA, NONE };

enum class SlewRate { GUIDE, CENTERING, FIND, MAX, NONE };

enum class MotionEW { WEST, EAST, NONE };

enum class MotionNS { NORTH, SOUTH, NONE };

enum class DomePolicy { IGNORED, LOCKED, NONE };

class AtomTelescope : public AtomDriver {
public:
    explicit AtomTelescope(std::string name) : AtomDriver(name) {}

    virtual auto getTelescopeInfo()
        -> std::optional<std::tuple<double, double, double, double>> = 0;
    virtual auto setTelescopeInfo(double telescopeAperture,
                                  double telescopeFocal, double guiderAperture,
                                  double guiderFocal) -> bool = 0;
    virtual auto getTelescopePierSide() -> std::optional<PierSide> = 0;

    virtual auto getTelescopeTrackRate() -> std::optional<TrackMode> = 0;
    virtual auto setTelescopeTrackRate(TrackMode rate) -> bool = 0;

    virtual auto getTelescopeTrackEnable() -> bool = 0;
    virtual auto setTelescopeTrackEnable(bool enable) -> bool = 0;

    virtual auto setTelescopeAbortMotion() -> bool = 0;

    virtual auto setTelescopeParkOption(ParkOptions option) -> bool = 0;

    virtual auto getTelescopeParkPosition()
        -> std::optional<std::pair<double, double>> = 0;
    virtual auto setTelescopeParkPosition(double parkRA,
                                          double parkDEC) -> bool = 0;

    virtual auto getTelescopePark() -> bool = 0;
    virtual auto setTelescopePark(bool isParked) -> bool = 0;

    virtual auto setTelescopeHomeInit(std::string_view command) -> bool = 0;

    virtual auto getTelescopeSlewRate() -> std::optional<double> = 0;
    virtual auto setTelescopeSlewRate(double speed) -> bool = 0;
    virtual auto getTelescopeTotalSlewRate() -> std::optional<double> = 0;

    virtual auto getTelescopeMoveWE() -> std::optional<MotionEW> = 0;
    virtual auto setTelescopeMoveWE(MotionEW direction) -> bool = 0;
    virtual auto getTelescopeMoveNS() -> std::optional<MotionNS> = 0;
    virtual auto setTelescopeMoveNS(MotionNS direction) -> bool = 0;

    virtual auto setTelescopeGuideNS(int dir, int timeGuide) -> bool = 0;
    virtual auto setTelescopeGuideWE(int dir, int timeGuide) -> bool = 0;

    virtual auto setTelescopeActionAfterPositionSet(std::string_view action)
        -> bool = 0;

    virtual auto getTelescopeRADECJ2000()
        -> std::optional<std::pair<double, double>> = 0;
    virtual auto setTelescopeRADECJ2000(double RAHours,
                                        double DECDegree) -> bool = 0;

    virtual auto getTelescopeRADECJNOW()
        -> std::optional<std::pair<double, double>> = 0;
    virtual auto setTelescopeRADECJNOW(double RAHours,
                                       double DECDegree) -> bool = 0;

    virtual auto getTelescopeTargetRADECJNOW()
        -> std::optional<std::pair<double, double>> = 0;
    virtual auto setTelescopeTargetRADECJNOW(double RAHours,
                                             double DECDegree) -> bool = 0;
    virtual auto slewTelescopeJNowNonBlock(double RAHours, double DECDegree,
                                           bool EnableTracking) -> bool = 0;

    virtual auto syncTelescopeJNow(double RAHours,
                                   double DECDegree) -> bool = 0;
    virtual auto getTelescopetAZALT()
        -> std::optional<std::pair<double, double>> = 0;
    virtual auto setTelescopetAZALT(double AZ_DEGREE,
                                    double ALT_DEGREE) -> bool = 0;
};
