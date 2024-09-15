#include "skysafari.hpp"

#include <cmath>
#include <format>
#include <optional>

class SkySafariController::Impl {
public:
    bool initialize(const std::string& host, int port) {
        // Actual connection code would go here
        m_connected = true;
        return m_connected;
    }

    std::string processCommand(std::string_view command) {
        if (command.starts_with("GR"))
            return getRightAscension();
        else if (command.starts_with("GD"))
            return getDeclination();
        else if (command.starts_with("Sr"))
            return setRightAscension(command.substr(2));
        else if (command.starts_with("Sd"))
            return setDeclination(command.substr(2));
        else if (command == "MS")
            return goTo();
        else if (command == "CM")
            return sync();
        else if (command == "Q")
            return abort();
        else if (command == "GG")
            return getUTCOffset();
        else if (command.starts_with("SG"))
            return setUTCOffset(command.substr(2));
        else if (command.starts_with("St"))
            return setLatitude(command.substr(2));
        else if (command.starts_with("Sg"))
            return setLongitude(command.substr(2));
        else if (command == "MP")
            return park() ? "1" : "0";
        else if (command == "MU")
            return unpark() ? "1" : "0";
        // Add other command handlers here

        return "1";  // Default response for unrecognized commands
    }

    void setTargetCoordinates(const Coordinates& coords) {
        m_targetCoords = coords;
    }
    void setGeographicCoordinates(const GeographicCoordinates& coords) {
        m_geoCoords = coords;
    }
    void setDateTime(const DateTime& dt) { m_dateTime = dt; }
    void setSlewRate(SlewRate rate) { m_slewRate = rate; }

    bool startSlew(Direction direction) {
        m_slewingDirection = direction;
        // Actual slew start code would go here
        return true;
    }

    bool stopSlew(Direction direction) {
        if (m_slewingDirection == direction) {
            m_slewingDirection = std::nullopt;
            // Actual slew stop code would go here
            return true;
        }
        return false;
    }

    bool park() {
        // Actual park code would go here
        m_parked = true;
        return true;
    }

    bool unpark() {
        // Actual unpark code would go here
        m_parked = false;
        return true;
    }

    Coordinates getCurrentCoordinates() const {
        return m_currentCoords.value_or(Coordinates{0, 0});
    }
    GeographicCoordinates getGeographicCoordinates() const {
        return m_geoCoords.value_or(GeographicCoordinates{0, 0});
    }
    DateTime getDateTime() const { return m_dateTime.value_or(DateTime{}); }
    SlewRate getSlewRate() const { return m_slewRate; }
    bool isConnected() const { return m_connected; }
    bool isParked() const { return m_parked; }

private:
    bool m_connected = false;
    bool m_parked = false;
    std::optional<Coordinates> m_currentCoords;
    std::optional<Coordinates> m_targetCoords;
    std::optional<GeographicCoordinates> m_geoCoords;
    std::optional<DateTime> m_dateTime;
    SlewRate m_slewRate = SlewRate::CENTERING;
    std::optional<Direction> m_slewingDirection;

    static std::string hoursToSexagesimal(double hours) {
        int h = static_cast<int>(hours);
        int m = static_cast<int>((hours - h) * 60);
        int s = static_cast<int>(((hours - h) * 60 - m) * 60);
        return std::format("{:02d}:{:02d}:{:02d}", h, m, s);
    }

    static std::string degreesToSexagesimal(double degrees) {
        char sign = degrees >= 0 ? '+' : '-';
        degrees = std::abs(degrees);
        int d = static_cast<int>(degrees);
        int m = static_cast<int>((degrees - d) * 60);
        int s = static_cast<int>(((degrees - d) * 60 - m) * 60);
        return std::format("{}{:02d}:{:02d}:{:02d}", sign, d, m, s);
    }

    std::string getRightAscension() {
        if (!m_currentCoords)
            return "Error";
        return hoursToSexagesimal(m_currentCoords->ra) + '#';
    }

    std::string getDeclination() {
        if (!m_currentCoords)
            return "Error";
        return degreesToSexagesimal(m_currentCoords->dec) + '#';
    }

    std::string setRightAscension(std::string_view ra) {
        if (!m_targetCoords)
            m_targetCoords = Coordinates{};
        m_targetCoords->ra = std::stod(std::string(ra));
        return "1";
    }

    std::string setDeclination(std::string_view dec) {
        if (!m_targetCoords)
            m_targetCoords = Coordinates{};
        m_targetCoords->dec = std::stod(std::string(dec));
        return "1";
    }

    std::string goTo() {
        if (!m_targetCoords)
            return "2<Not Ready>#";

        // Actual GOTO implementation would go here
        m_currentCoords = m_targetCoords;
        return "0";
    }

    std::string sync() {
        if (!m_targetCoords)
            return "Error";

        m_currentCoords = m_targetCoords;
        return " M31 EX GAL MAG 3.5 SZ178.0'#";
    }

    std::string abort() {
        m_slewingDirection = std::nullopt;
        // Actual abort implementation would go here
        return "1";
    }

    std::string getUTCOffset() {
        if (!m_dateTime)
            return "Error";
        return std::format("{:.1f}", m_dateTime->utcOffset);
    }

    std::string setUTCOffset(std::string_view offset) {
        if (!m_dateTime)
            m_dateTime = DateTime{};
        m_dateTime->utcOffset = std::stod(std::string(offset));
        return "1";
    }

    std::string setLatitude(std::string_view lat) {
        if (!m_geoCoords)
            m_geoCoords = GeographicCoordinates{};
        m_geoCoords->latitude = std::stod(std::string(lat));
        return "1";
    }

    std::string setLongitude(std::string_view lon) {
        if (!m_geoCoords)
            m_geoCoords = GeographicCoordinates{};
        m_geoCoords->longitude = std::stod(std::string(lon));
        return "1";
    }
};

// SkySafariController implementation

SkySafariController::SkySafariController() : pImpl(std::make_unique<Impl>()) {}
SkySafariController::~SkySafariController() = default;

SkySafariController::SkySafariController(SkySafariController&&) noexcept =
    default;
SkySafariController& SkySafariController::operator=(
    SkySafariController&&) noexcept = default;

bool SkySafariController::initialize(const std::string& host, int port) {
    return pImpl->initialize(host, port);
}

std::string SkySafariController::processCommand(std::string_view command) {
    return pImpl->processCommand(command);
}

void SkySafariController::setTargetCoordinates(const Coordinates& coords) {
    pImpl->setTargetCoordinates(coords);
}

void SkySafariController::setGeographicCoordinates(
    const GeographicCoordinates& coords) {
    pImpl->setGeographicCoordinates(coords);
}

void SkySafariController::setDateTime(const DateTime& dt) {
    pImpl->setDateTime(dt);
}

void SkySafariController::setSlewRate(SlewRate rate) {
    pImpl->setSlewRate(rate);
}

bool SkySafariController::startSlew(Direction direction) {
    return pImpl->startSlew(direction);
}

bool SkySafariController::stopSlew(Direction direction) {
    return pImpl->stopSlew(direction);
}

bool SkySafariController::park() { return pImpl->park(); }

bool SkySafariController::unpark() { return pImpl->unpark(); }

SkySafariController::Coordinates SkySafariController::getCurrentCoordinates()
    const {
    return pImpl->getCurrentCoordinates();
}

SkySafariController::GeographicCoordinates
SkySafariController::getGeographicCoordinates() const {
    return pImpl->getGeographicCoordinates();
}

SkySafariController::DateTime SkySafariController::getDateTime() const {
    return pImpl->getDateTime();
}

SkySafariController::SlewRate SkySafariController::getSlewRate() const {
    return pImpl->getSlewRate();
}

bool SkySafariController::isConnected() const { return pImpl->isConnected(); }

bool SkySafariController::isParked() const { return pImpl->isParked(); }
