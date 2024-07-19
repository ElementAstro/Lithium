#ifndef LITHIUM_CLIENT_INDI_TELESCOPE_HPP
#define LITHIUM_CLIENT_INDI_TELESCOPE_HPP

#include <libindi/baseclient.h>
#include <libindi/basedevice.h>

#include <atomic>
#include <optional>
#include <string_view>

enum class ConnectionMode { SERIAL, TCP, NONE };

enum class BAUD_RATE { B9600, B19200, B38400, B57600, B115200, B230400, NONE };

enum class TrackMode { SIDEREAL, SOLAR, LUNAR, CUSTOM, NONE };

enum class PierSide { EAST, WEST, NONE };

enum class ParkOptions { CURRENT, DEFAULT, WRITE_DATA, PURGE_DATA, NONE };

enum class SlewRate { GUIDE, CENTERING, FIND, MAX, NONE };

enum class MotionEW { WEST, EAST, NONE };

enum class MotionNS { NORTH, SOUTH, NONE };

enum class DomePolicy { IGNORED, LOCKED, NONE };

class INDITelescope : public INDI::BaseClient {
public:
    explicit INDITelescope(std::string name);
    ~INDITelescope() override = default;

    auto connect(const std::string& deviceName) -> bool;
    auto disconnect() -> void;
    auto reconnect() -> bool;

    virtual auto watchAdditionalProperty() -> bool;

    void setPropertyNumber(std::string_view propertyName, double value);

    auto getTelescopeInfo()
        -> std::optional<std::tuple<double, double, double, double>>;
    auto setTelescopeInfo(double telescopeAperture, double telescopeFocal,
                          double guiderAperture, double guiderFocal) -> bool;
    auto getTelescopePierSide() -> std::optional<PierSide>;

    auto getTelescopeTrackRate() -> std::optional<TrackMode>;
    auto setTelescopeTrackRate(TrackMode rate) -> bool;

    auto getTelescopeTrackEnable() -> bool;
    auto setTelescopeTrackEnable(bool enable) -> bool;

    auto setTelescopeAbortMotion() -> bool;

    auto setTelescopeParkOption(ParkOptions option) -> bool;

    auto getTelescopeParkPosition() -> std::optional<std::pair<double, double>>;
    auto setTelescopeParkPosition(double parkRA, double parkDEC) -> bool;

    auto getTelescopePark() -> bool;
    auto setTelescopePark(bool isParked) -> bool;

    auto setTelescopeHomeInit(std::string_view command) -> bool;

    auto getTelescopeSlewRate() -> std::optional<double>;
    auto setTelescopeSlewRate(double speed) -> bool;
    auto getTelescopeTotalSlewRate() -> std::optional<double>;

    auto getTelescopeMoveWE() -> std::optional<MotionEW>;
    auto setTelescopeMoveWE(MotionEW direction) -> bool;
    auto getTelescopeMoveNS() -> std::optional<MotionNS>;
    auto setTelescopeMoveNS(MotionNS direction) -> bool;

    auto setTelescopeGuideNS(int dir, int timeGuide) -> bool;
    auto setTelescopeGuideWE(int dir, int timeGuide) -> bool;

    auto setTelescopeActionAfterPositionSet(std::string_view action) -> bool;

    auto getTelescopeRADECJ2000() -> std::optional<std::pair<double, double>>;
    auto setTelescopeRADECJ2000(double RAHours, double DECDegree) -> bool;

    auto getTelescopeRADECJNOW() -> std::optional<std::pair<double, double>>;
    auto setTelescopeRADECJNOW(double RAHours, double DECDegree) -> bool;

    auto getTelescopeTargetRADECJNOW()
        -> std::optional<std::pair<double, double>>;
    auto setTelescopeTargetRADECJNOW(double RAHours, double DECDegree) -> bool;
    auto slewTelescopeJNowNonBlock(double RAHours, double DECDegree,
                                   bool EnableTracking) -> bool;

    auto syncTelescopeJNow(double RAHours, double DECDegree) -> bool;
    auto getTelescopetAZALT() -> std::optional<std::pair<double, double>>;
    auto setTelescopetAZALT(double AZ_DEGREE, double ALT_DEGREE) -> bool;

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