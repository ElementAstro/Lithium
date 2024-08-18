#ifndef LITHIUM_CLIENT_INDI_TELESCOPE_HPP
#define LITHIUM_CLIENT_INDI_TELESCOPE_HPP

#include <libindi/baseclient.h>
#include <libindi/basedevice.h>

#include <atomic>
#include <optional>
#include <string_view>

#include "device/template/telescope.hpp"

class INDITelescope : public INDI::BaseClient, public AtomTelescope {
public:
    explicit INDITelescope(std::string name);
    ~INDITelescope() override = default;

    auto connect(const std::string &deviceName, int timeout,
                 int maxRetry) -> bool override;

    auto disconnect(bool force, int timeout, int maxRetry) -> bool override;

    auto reconnect(int timeout, int maxRetry) -> bool override;

    auto scan() -> std::vector<std::string> override;

    auto isConnected() -> bool override;

    virtual auto watchAdditionalProperty() -> bool;

    void setPropertyNumber(std::string_view propertyName, double value);

    auto getTelescopeInfo()
        -> std::optional<std::tuple<double, double, double, double>> override;
    auto setTelescopeInfo(double telescopeAperture, double telescopeFocal,
                          double guiderAperture,
                          double guiderFocal) -> bool override;
    auto getTelescopePierSide() -> std::optional<PierSide> override;

    auto getTelescopeTrackRate() -> std::optional<TrackMode> override;
    auto setTelescopeTrackRate(TrackMode rate) -> bool override;

    auto getTelescopeTrackEnable() -> bool override;
    auto setTelescopeTrackEnable(bool enable) -> bool override;

    auto setTelescopeAbortMotion() -> bool override;

    auto setTelescopeParkOption(ParkOptions option) -> bool override;

    auto getTelescopeParkPosition()
        -> std::optional<std::pair<double, double>> override;
    auto setTelescopeParkPosition(double parkRA,
                                  double parkDEC) -> bool override;

    auto getTelescopePark() -> bool override;
    auto setTelescopePark(bool isParked) -> bool override;

    auto setTelescopeHomeInit(std::string_view command) -> bool override;

    auto getTelescopeSlewRate() -> std::optional<double> override;
    auto setTelescopeSlewRate(double speed) -> bool override;
    auto getTelescopeTotalSlewRate() -> std::optional<double> override;

    auto getTelescopeMoveWE() -> std::optional<MotionEW> override;
    auto setTelescopeMoveWE(MotionEW direction) -> bool override;
    auto getTelescopeMoveNS() -> std::optional<MotionNS> override;
    auto setTelescopeMoveNS(MotionNS direction) -> bool override;

    auto setTelescopeGuideNS(int dir, int timeGuide) -> bool override;
    auto setTelescopeGuideWE(int dir, int timeGuide) -> bool override;

    auto setTelescopeActionAfterPositionSet(std::string_view action)
        -> bool override;

    auto getTelescopeRADECJ2000()
        -> std::optional<std::pair<double, double>> override;
    auto setTelescopeRADECJ2000(double RAHours,
                                double DECDegree) -> bool override;

    auto getTelescopeRADECJNOW()
        -> std::optional<std::pair<double, double>> override;
    auto setTelescopeRADECJNOW(double RAHours,
                               double DECDegree) -> bool override;

    auto getTelescopeTargetRADECJNOW()
        -> std::optional<std::pair<double, double>> override;
    auto setTelescopeTargetRADECJNOW(double RAHours,
                                     double DECDegree) -> bool override;
    auto slewTelescopeJNowNonBlock(double RAHours, double DECDegree,
                                   bool EnableTracking) -> bool override;

    auto syncTelescopeJNow(double RAHours, double DECDegree) -> bool override;
    auto getTelescopetAZALT()
        -> std::optional<std::pair<double, double>> override;
    auto setTelescopetAZALT(double AZ_DEGREE,
                            double ALT_DEGREE) -> bool override;

protected:
    void newMessage(INDI::BaseDevice baseDevice, int messageID) override;

private:
    std::string name_;
    std::string deviceName_;

    std::string driverExec_;
    std::string driverVersion_;
    std::string driverInterface_;
    bool deviceAutoSearch_;
    bool devicePortScan_;

    std::atomic<double> currentPollingPeriod_;

    std::atomic_bool isDebug_;

    std::atomic_bool isConnected_;

    INDI::BaseDevice device_;
    INDI::BaseDevice gps_;
    INDI::BaseDevice dome_;
    INDI::BaseDevice joystick_;

    ConnectionMode connectionMode_;

    std::string devicePort_;
    BAUD_RATE baudRate_;

    bool isTrackingEnabled_;
    std::atomic_bool isTracking_;
    TrackMode trackMode_;
    std::atomic<double> trackRateRA_;
    std::atomic<double> trackRateDEC_;
    PierSide pierSide_;

    SlewRate slewRate_;
    int totalSlewRate_;
    double maxSlewRate_;
    double minSlewRate_;

    std::atomic<double> targetSlewRA_;
    std::atomic<double> targetSlewDEC_;

    MotionEW motionEW_;
    std::atomic_bool motionEWReserved_;
    MotionNS motionNS_;
    std::atomic_bool motionNSReserved_;

    double telescopeAperture_;
    double telescopeFocalLength_;
    double telescopeGuiderAperture_;
    double telescopeGuiderFocalLength_;

    bool isParkEnabled_;
    std::atomic_bool isParked_;
    double telescopeParkPositionRA_;
    double telescopeParkPositionDEC_;
    ParkOptions parkOption_;

    std::atomic_bool isHomed_;
    std::atomic_bool isHomeInitEnabled_;
    std::atomic_bool isHomeInitInProgress_;

    bool isJoystickEnabled_;

    DomePolicy domePolicy_;
};

#endif
