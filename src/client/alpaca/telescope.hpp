#pragma once

#include <chrono>
#include <future>
#include <optional>
#include <string>
#include <variant>
#include <vector>
#include "device.hpp"

class AlpacaTelescope : public AlpacaDevice {
public:
    enum class AlignmentModes {
        algAltAz = 0,
        algPolar = 1,
        algGermanPolar = 2
    };

    enum class DriveRates {
        driveSidereal = 0,
        driveLunar = 1,
        driveSolar = 2,
        driveKing = 3
    };

    enum class EquatorialCoordinateType {
        equOther = 0,
        equTopocentric = 1,
        equJ2000 = 2,
        equJ2050 = 3,
        equB1950 = 4
    };

    enum class GuideDirections {
        guideNorth = 0,
        guideSouth = 1,
        guideEast = 2,
        guideWest = 3
    };

    enum class PierSide { pierEast = 0, pierWest = 1, pierUnknown = -1 };

    enum class TelescopeAxes {
        axisPrimary = 0,
        axisSecondary = 1,
        axisTertiary = 2
    };

    struct Rate {
        double Maximum;
        double Minimum;
    };

    AlpacaTelescope(std::string_view address, int device_number,
                    std::string_view protocol = "http");
    virtual ~AlpacaTelescope() = default;

    // Properties
    AlignmentModes GetAlignmentMode();
    double GetAltitude();
    double GetApertureArea();
    double GetApertureDiameter();
    bool GetAtHome();
    bool GetAtPark();
    double GetAzimuth();
    bool GetCanFindHome();
    bool GetCanPark();
    bool GetCanPulseGuide();
    bool GetCanSetDeclinationRate();
    bool GetCanSetGuideRates();
    bool GetCanSetPark();
    bool GetCanSetPierSide();
    bool GetCanSetRightAscensionRate();
    bool GetCanSetTracking();
    bool GetCanSlew();
    bool GetCanSlewAsync();
    bool GetCanSlewAltAz();
    bool GetCanSlewAltAzAsync();
    bool GetCanSync();
    bool GetCanSyncAltAz();
    bool GetCanUnpark();
    double GetDeclination();
    double GetDeclinationRate();
    void SetDeclinationRate(double DeclinationRate);
    bool GetDoesRefraction();
    void SetDoesRefraction(bool DoesRefraction);
    EquatorialCoordinateType GetEquatorialSystem();
    double GetFocalLength();
    double GetGuideRateDeclination();
    void SetGuideRateDeclination(double GuideRateDeclination);
    double GetGuideRateRightAscension();
    void SetGuideRateRightAscension(double GuideRateRightAscension);
    bool GetIsPulseGuiding();
    double GetRightAscension();
    double GetRightAscensionRate();
    void SetRightAscensionRate(double RightAscensionRate);
    PierSide GetSideOfPier();
    void SetSideOfPier(PierSide SideOfPier);
    double GetSiderealTime();
    double GetSiteElevation();
    void SetSiteElevation(double SiteElevation);
    double GetSiteLatitude();
    void SetSiteLatitude(double SiteLatitude);
    double GetSiteLongitude();
    void SetSiteLongitude(double SiteLongitude);
    bool GetSlewing();
    int GetSlewSettleTime();
    void SetSlewSettleTime(int SlewSettleTime);
    double GetTargetDeclination();
    void SetTargetDeclination(double TargetDeclination);
    double GetTargetRightAscension();
    void SetTargetRightAscension(double TargetRightAscension);
    bool GetTracking();
    void SetTracking(bool Tracking);
    DriveRates GetTrackingRate();
    void SetTrackingRate(DriveRates TrackingRate);
    std::vector<DriveRates> GetTrackingRates();
    std::chrono::system_clock::time_point GetUTCDate();
    void SetUTCDate(const std::chrono::system_clock::time_point& UTCDate);

    // Methods
    std::vector<Rate> AxisRates(TelescopeAxes Axis);
    bool CanMoveAxis(TelescopeAxes Axis);
    PierSide DestinationSideOfPier(double RightAscension,
                                   double Declination);
    void AbortSlew();
    std::future<void> FindHome();
    void MoveAxis(TelescopeAxes Axis, double Rate);
    std::future<void> Park();
    std::future<void> PulseGuide(GuideDirections Direction, int Duration);
    void SetPark();
    std::future<void> SlewToAltAzAsync(double Azimuth, double Altitude);
    std::future<void> SlewToCoordinatesAsync(double RightAscension,
                                             double Declination);
    std::future<void> SlewToTargetAsync();
    void SyncToAltAz(double Azimuth, double Altitude);
    void SyncToCoordinates(double RightAscension, double Declination);
    void SyncToTarget();
    void Unpark();

private:
    template <typename T>
    T GetProperty(const std::string& property) const {
        return GetNumericProperty<T>(property);
    }

    template <typename Func>
    std::future<void> AsyncOperation(Func&& func,
                                     const std::string& operationName);
};