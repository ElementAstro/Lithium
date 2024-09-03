#include "telescope.hpp"
#include <chrono>
#include <stdexcept>
#include <thread>

AlpacaTelescope::AlpacaTelescope(std::string_view address, int device_number,
                                 std::string_view protocol)
    : AlpacaDevice(std::string(address), "telescope", device_number,
                   std::string(protocol)) {}

AlpacaTelescope::AlignmentModes AlpacaTelescope::GetAlignmentMode() {
    return static_cast<AlignmentModes>(GetProperty<int>("alignmentmode"));
}

double AlpacaTelescope::GetAltitude() {
    return GetProperty<double>("altitude");
}

double AlpacaTelescope::GetApertureArea() {
    return GetProperty<double>("aperturearea");
}

double AlpacaTelescope::GetApertureDiameter() {
    return GetProperty<double>("aperturediameter");
}

bool AlpacaTelescope::GetAtHome() { return GetProperty<bool>("athome"); }

bool AlpacaTelescope::GetAtPark() { return GetProperty<bool>("atpark"); }

double AlpacaTelescope::GetAzimuth() { return GetProperty<double>("azimuth"); }

bool AlpacaTelescope::GetCanFindHome() {
    return GetProperty<bool>("canfindhome");
}

bool AlpacaTelescope::GetCanPark() { return GetProperty<bool>("canpark"); }

bool AlpacaTelescope::GetCanPulseGuide() {
    return GetProperty<bool>("canpulseguide");
}

bool AlpacaTelescope::GetCanSetDeclinationRate() {
    return GetProperty<bool>("cansetdeclinationrate");
}

bool AlpacaTelescope::GetCanSetGuideRates() {
    return GetProperty<bool>("cansetguiderates");
}

bool AlpacaTelescope::GetCanSetPark() {
    return GetProperty<bool>("cansetpark");
}

bool AlpacaTelescope::GetCanSetPierSide() {
    return GetProperty<bool>("cansetpierside");
}

bool AlpacaTelescope::GetCanSetRightAscensionRate() {
    return GetProperty<bool>("cansetrightascensionrate");
}

bool AlpacaTelescope::GetCanSetTracking() {
    return GetProperty<bool>("cansettracking");
}

bool AlpacaTelescope::GetCanSlew() { return GetProperty<bool>("canslew"); }

bool AlpacaTelescope::GetCanSlewAsync() {
    return GetProperty<bool>("canslewasync");
}

bool AlpacaTelescope::GetCanSlewAltAz() {
    return GetProperty<bool>("canslewaltaz");
}

bool AlpacaTelescope::GetCanSlewAltAzAsync() {
    return GetProperty<bool>("canslewaltazasync");
}

bool AlpacaTelescope::GetCanSync() { return GetProperty<bool>("cansync"); }

bool AlpacaTelescope::GetCanSyncAltAz() {
    return GetProperty<bool>("cansyncaltaz");
}

bool AlpacaTelescope::GetCanUnpark() { return GetProperty<bool>("canunpark"); }

double AlpacaTelescope::GetDeclination() {
    return GetProperty<double>("declination");
}

double AlpacaTelescope::GetDeclinationRate() {
    return GetProperty<double>("declinationrate");
}

void AlpacaTelescope::SetDeclinationRate(double DeclinationRate) {
    Put("declinationrate",
        {{"DeclinationRate", std::to_string(DeclinationRate)}});
}

bool AlpacaTelescope::GetDoesRefraction() {
    return GetProperty<bool>("doesrefraction");
}

void AlpacaTelescope::SetDoesRefraction(bool DoesRefraction) {
    Put("doesrefraction",
        {{"DoesRefraction", DoesRefraction ? "true" : "false"}});
}

AlpacaTelescope::EquatorialCoordinateType
AlpacaTelescope::GetEquatorialSystem() {
    return static_cast<EquatorialCoordinateType>(
        GetProperty<int>("equatorialsystem"));
}

double AlpacaTelescope::GetFocalLength() {
    return GetProperty<double>("focallength");
}

// ... (其他属性的getter和setter方法)

template <typename Func>
std::future<void> AlpacaTelescope::AsyncOperation(
    Func&& func, const std::string& operationName) {
    return std::async(
        std::launch::async,
        [this, func = std::forward<Func>(func), operationName]() {
            func();
            while (GetSlewing()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        });
}

void AlpacaTelescope::AbortSlew() { Put("abortslew"); }

std::future<void> AlpacaTelescope::FindHome() {
    return AsyncOperation([this]() { Put("findhome"); }, "findhome");
}

void AlpacaTelescope::MoveAxis(TelescopeAxes Axis, double Rate) {
    Put("moveaxis", {{"Axis", std::to_string(static_cast<int>(Axis))},
                     {"Rate", std::to_string(Rate)}});
}

std::future<void> AlpacaTelescope::Park() {
    return AsyncOperation([this]() { Put("park"); }, "park");
}

std::future<void> AlpacaTelescope::PulseGuide(GuideDirections Direction,
                                              int Duration) {
    return AsyncOperation(
        [this, Direction, Duration]() {
            Put("pulseguide",
                {{"Direction", std::to_string(static_cast<int>(Direction))},
                 {"Duration", std::to_string(Duration)}});
        },
        "pulseguide");
}

void AlpacaTelescope::SetPark() { Put("setpark"); }

double AlpacaTelescope::GetGuideRateDeclination() {
    return GetProperty<double>("guideratedeclination");
}

void AlpacaTelescope::SetGuideRateDeclination(double GuideRateDeclination) {
    Put("guideratedeclination", {{"GuideRateDeclination", std::to_string(GuideRateDeclination)}});
}

double AlpacaTelescope::GetGuideRateRightAscension() {
    return GetProperty<double>("guideraterightascension");
}

void AlpacaTelescope::SetGuideRateRightAscension(double GuideRateRightAscension) {
    Put("guideraterightascension", {{"GuideRateRightAscension", std::to_string(GuideRateRightAscension)}});
}

bool AlpacaTelescope::GetIsPulseGuiding() {
    return GetProperty<bool>("ispulseguiding");
}

double AlpacaTelescope::GetRightAscension() {
    return GetProperty<double>("rightascension");
}

double AlpacaTelescope::GetRightAscensionRate() {
    return GetProperty<double>("rightascensionrate");
}

void AlpacaTelescope::SetRightAscensionRate(double RightAscensionRate) {
    Put("rightascensionrate", {{"RightAscensionRate", std::to_string(RightAscensionRate)}});
}

AlpacaTelescope::PierSide AlpacaTelescope::GetSideOfPier() {
    return static_cast<PierSide>(GetProperty<int>("sideofpier"));
}

void AlpacaTelescope::SetSideOfPier(PierSide SideOfPier) {
    Put("sideofpier", {{"SideOfPier", std::to_string(static_cast<int>(SideOfPier))}});
}

double AlpacaTelescope::GetSiderealTime() {
    return GetProperty<double>("siderealtime");
}

double AlpacaTelescope::GetSiteElevation() {
    return GetProperty<double>("siteelevation");
}

void AlpacaTelescope::SetSiteElevation(double SiteElevation) {
    Put("siteelevation", {{"SiteElevation", std::to_string(SiteElevation)}});
}

double AlpacaTelescope::GetSiteLatitude() {
    return GetProperty<double>("sitelatitude");
}

void AlpacaTelescope::SetSiteLatitude(double SiteLatitude) {
    Put("sitelatitude", {{"SiteLatitude", std::to_string(SiteLatitude)}});
}

double AlpacaTelescope::GetSiteLongitude() {
    return GetProperty<double>("sitelongitude");
}

void AlpacaTelescope::SetSiteLongitude(double SiteLongitude) {
    Put("sitelongitude", {{"SiteLongitude", std::to_string(SiteLongitude)}});
}

bool AlpacaTelescope::GetSlewing() {
    return GetProperty<bool>("slewing");
}

int AlpacaTelescope::GetSlewSettleTime() {
    return GetProperty<int>("slewsettletime");
}

void AlpacaTelescope::SetSlewSettleTime(int SlewSettleTime) {
    Put("slewsettletime", {{"SlewSettleTime", std::to_string(SlewSettleTime)}});
}

double AlpacaTelescope::GetTargetDeclination() {
    return GetProperty<double>("targetdeclination");
}

void AlpacaTelescope::SetTargetDeclination(double TargetDeclination) {
    Put("targetdeclination", {{"TargetDeclination", std::to_string(TargetDeclination)}});
}

double AlpacaTelescope::GetTargetRightAscension() {
    return GetProperty<double>("targetrightascension");
}

void AlpacaTelescope::SetTargetRightAscension(double TargetRightAscension) {
    Put("targetrightascension", {{"TargetRightAscension", std::to_string(TargetRightAscension)}});
}

bool AlpacaTelescope::GetTracking() {
    return GetProperty<bool>("tracking");
}

void AlpacaTelescope::SetTracking(bool Tracking) {
    Put("tracking", {{"Tracking", Tracking ? "true" : "false"}});
}

AlpacaTelescope::DriveRates AlpacaTelescope::GetTrackingRate() {
    return static_cast<DriveRates>(GetProperty<int>("trackingrate"));
}

void AlpacaTelescope::SetTrackingRate(DriveRates TrackingRate) {
    Put("trackingrate", {{"TrackingRate", std::to_string(static_cast<int>(TrackingRate))}});
}

std::vector<AlpacaTelescope::DriveRates> AlpacaTelescope::GetTrackingRates() {
    auto rates = GetArrayProperty<int>("trackingrates");
    std::vector<DriveRates> result;
    for (auto rate : rates) {
        result.push_back(static_cast<DriveRates>(rate));
    }
    return result;
}

std::chrono::system_clock::time_point AlpacaTelescope::GetUTCDate() {
    std::string dateStr = Get("utcdate");
    std::tm tm = {};
    std::istringstream ss(dateStr);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

void AlpacaTelescope::SetUTCDate(const std::chrono::system_clock::time_point& UTCDate) {
    auto time = std::chrono::system_clock::to_time_t(UTCDate);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time), "%Y-%m-%dT%H:%M:%S");
    Put("utcdate", {{"UTCDate", ss.str()}});
}

std::vector<AlpacaTelescope::Rate> AlpacaTelescope::AxisRates(TelescopeAxes Axis) {
    auto rates = GetArrayProperty<Rate>("axisrates", {{"Axis", std::to_string(static_cast<int>(Axis))}});
    return rates;
}

bool AlpacaTelescope::CanMoveAxis(TelescopeAxes Axis) {
    return GetProperty<bool>("canmoveaxis", {{"Axis", std::to_string(static_cast<int>(Axis))}});
}

AlpacaTelescope::PierSide AlpacaTelescope::DestinationSideOfPier(double RightAscension, double Declination) {
    return static_cast<PierSide>(GetProperty<int>("destinationsideofpier", {
        {"RightAscension", std::to_string(RightAscension)},
        {"Declination", std::to_string(Declination)}
    }));
}

std::future<void> AlpacaTelescope::SlewToAltAzAsync(double Azimuth, double Altitude) {
    return AsyncOperation([this, Azimuth, Altitude]() {
        Put("slewtoaltazasync", {{"Azimuth", std::to_string(Azimuth)}, {"Altitude", std::to_string(Altitude)}});
    }, "slewtoaltaz");
}

std::future<void> AlpacaTelescope::SlewToCoordinatesAsync(double RightAscension, double Declination) {
    return AsyncOperation([this, RightAscension, Declination]() {
        Put("slewtocoordinatesasync", {{"RightAscension", std::to_string(RightAscension)}, {"Declination", std::to_string(Declination)}});
    }, "slewtocoordinates");
}

std::future<void> AlpacaTelescope::SlewToTargetAsync() {
    return AsyncOperation([this]() {
        Put("slewtotargetasync");
    }, "slewtotarget");
}

void AlpacaTelescope::SyncToAltAz(double Azimuth, double Altitude) {
    Put("synctoaltaz", {{"Azimuth", std::to_string(Azimuth)}, {"Altitude", std::to_string(Altitude)}});
}

void AlpacaTelescope::SyncToCoordinates(double RightAscension, double Declination) {
    Put("synctocoordinates", {{"RightAscension", std::to_string(RightAscension)}, {"Declination", std::to_string(Declination)}});
}

void AlpacaTelescope::SyncToTarget() {
    Put("synctotarget");
}

void AlpacaTelescope::Unpark() {
    Put("unpark");
}