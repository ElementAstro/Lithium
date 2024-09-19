#include "skysafari.hpp"

#include <cmath>
#include <format>
#include <optional>

class SkySafariController::Impl {
public:
    auto initialize(const std::string& /*host*/, int /*port*/) -> bool {
        // Actual connection code would go here
        connected_ = true;
        return connected_;
    }

    auto processCommand(std::string_view command) -> std::string {
        if (command.starts_with("GR")) {
            return getRightAscension();
        } else if (command.starts_with("GD")) {
            return getDeclination();
        } else if (command.starts_with("Sr")) {
            return setRightAscension(command.substr(2));
        } else if (command.starts_with("Sd")) {
            return setDeclination(command.substr(2));
        } else if (command == "MS") {
            return goTo();
        } else if (command == "CM") {
            return sync();
        } else if (command == "Q") {
            return abort();
        } else if (command == "GG") {
            return getUTCOffset();
        } else if (command.starts_with("SG")) {
            return setUTCOffset(command.substr(2));
        } else if (command.starts_with("St")) {
            return setLatitude(command.substr(2));
        } else if (command.starts_with("Sg")) {
            return setLongitude(command.substr(2));
        } else if (command == "MP") {
            return park() ? "1" : "0";
        } else if (command == "MU") {
            return unpark() ? "1" : "0";
        }
        // Add other command handlers here

        return "1";  // Default response for unrecognized commands
    }

    void setTargetCoordinates(const Coordinates& coords) {
        targetCoords_ = coords;
    }
    void setGeographicCoordinates(const GeographicCoordinates& coords) {
        geoCoords_ = coords;
    }
    void setDateTime(const DateTime& dateTime) { dateTime_ = dateTime; }
    void setSlewRate(SlewRate rate) { slewRate_ = rate; }

    auto startSlew(Direction direction) -> bool {
        slewingDirection_ = direction;
        // Actual slew start code would go here
        return true;
    }

    auto stopSlew(Direction direction) -> bool {
        if (slewingDirection_ == direction) {
            slewingDirection_ = std::nullopt;
            // Actual slew stop code would go here
            return true;
        }
        return false;
    }

    auto park() -> bool {
        // Actual park code would go here
        parked_ = true;
        return true;
    }

    auto unpark() -> bool {
        // Actual unpark code would go here
        parked_ = false;
        return true;
    }

    [[nodiscard]] auto getCurrentCoordinates() const -> Coordinates {
        return currentCoords_.value_or(Coordinates{0, 0});
    }
    [[nodiscard]] auto getGeographicCoordinates() const
        -> GeographicCoordinates {
        return geoCoords_.value_or(GeographicCoordinates{0, 0});
    }
    [[nodiscard]] auto getDateTime() const -> DateTime {
        return dateTime_.value_or(DateTime{});
    }
    [[nodiscard]] auto getSlewRate() const -> SlewRate { return slewRate_; }
    [[nodiscard]] auto isConnected() const -> bool { return connected_; }
    [[nodiscard]] auto isParked() const -> bool { return parked_; }

private:
    bool connected_ = false;
    bool parked_ = false;
    std::optional<Coordinates> currentCoords_;
    std::optional<Coordinates> targetCoords_;
    std::optional<GeographicCoordinates> geoCoords_;
    std::optional<DateTime> dateTime_;
    SlewRate slewRate_ = SlewRate::CENTERING;
    std::optional<Direction> slewingDirection_;

    static auto hoursToSexagesimal(double hours) -> std::string {
        constexpr int SECONDS_IN_MINUTE = 60;
        int hoursInt = static_cast<int>(hours);
        int minutes = static_cast<int>((hours - hoursInt) * SECONDS_IN_MINUTE);
        int seconds = static_cast<int>(
            ((hours - hoursInt) * SECONDS_IN_MINUTE - minutes) *
            SECONDS_IN_MINUTE);
        return std::format("{:02d}:{:02d}:{:02d}", hoursInt, minutes, seconds);
    }

    static auto degreesToSexagesimal(double degrees) -> std::string {
        constexpr int SECONDS_IN_MINUTE = 60;
        char sign = degrees >= 0 ? '+' : '-';
        degrees = std::abs(degrees);
        int degreesInt = static_cast<int>(degrees);
        int minutes =
            static_cast<int>((degrees - degreesInt) * SECONDS_IN_MINUTE);
        int seconds = static_cast<int>(
            ((degrees - degreesInt) * SECONDS_IN_MINUTE - minutes) *
            SECONDS_IN_MINUTE);
        return std::format("{}{:02d}:{:02d}:{:02d}", sign, degreesInt, minutes,
                           seconds);
    }

    auto getRightAscension() -> std::string {
        if (!currentCoords_) {
            return "Error";
        }
        return hoursToSexagesimal(currentCoords_->ra) + '#';
    }

    auto getDeclination() -> std::string {
        if (!currentCoords_) {
            return "Error";
        }
        return degreesToSexagesimal(currentCoords_->dec) + '#';
    }

    auto setRightAscension(std::string_view ra) -> std::string {
        if (!targetCoords_) {
            targetCoords_ = Coordinates{};
        }
        targetCoords_->ra = std::stod(std::string(ra));
        return "1";
    }

    auto setDeclination(std::string_view dec) -> std::string {
        if (!targetCoords_) {
            targetCoords_ = Coordinates{};
        }
        targetCoords_->dec = std::stod(std::string(dec));
        return "1";
    }

    auto goTo() -> std::string {
        if (!targetCoords_) {
            return "2<Not Ready>#";
        }

        // Actual GOTO implementation would go here
        currentCoords_ = targetCoords_;
        return "0";
    }

    auto sync() -> std::string {
        if (!targetCoords_) {
            return "Error";
        }

        currentCoords_ = targetCoords_;
        return " M31 EX GAL MAG 3.5 SZ178.0'#";
    }

    auto abort() -> std::string {
        slewingDirection_ = std::nullopt;
        // Actual abort implementation would go here
        return "1";
    }

    auto getUTCOffset() -> std::string {
        if (!dateTime_) {
            return "Error";
        }
        return std::format("{:.1f}", dateTime_->utcOffset);
    }

    auto setUTCOffset(std::string_view offset) -> std::string {
        if (!dateTime_) {
            dateTime_ = DateTime{};
        }
        dateTime_->utcOffset = std::stod(std::string(offset));
        return "1";
    }

    auto setLatitude(std::string_view lat) -> std::string {
        if (!geoCoords_) {
            geoCoords_ = GeographicCoordinates{};
        }
        geoCoords_->latitude = std::stod(std::string(lat));
        return "1";
    }

    auto setLongitude(std::string_view lon) -> std::string {
        if (!geoCoords_) {
            geoCoords_ = GeographicCoordinates{};
        }
        geoCoords_->longitude = std::stod(std::string(lon));
        return "1";
    }
};

// SkySafariController implementation

SkySafariController::SkySafariController() : pImpl(std::make_unique<Impl>()) {}
SkySafariController::~SkySafariController() = default;

SkySafariController::SkySafariController(SkySafariController&&) noexcept =
    default;
auto SkySafariController::operator=(SkySafariController&&) noexcept
    -> SkySafariController& = default;

auto SkySafariController::initialize(const std::string& host,
                                     int port) -> bool {
    return pImpl->initialize(host, port);
}

auto SkySafariController::processCommand(std::string_view command)
    -> std::string {
    return pImpl->processCommand(command);
}

void SkySafariController::setTargetCoordinates(const Coordinates& coords) {
    pImpl->setTargetCoordinates(coords);
}

void SkySafariController::setGeographicCoordinates(
    const GeographicCoordinates& coords) {
    pImpl->setGeographicCoordinates(coords);
}

void SkySafariController::setDateTime(const DateTime& dateTime) {
    pImpl->setDateTime(dateTime);
}

void SkySafariController::setSlewRate(SlewRate rate) {
    pImpl->setSlewRate(rate);
}

auto SkySafariController::startSlew(Direction direction) -> bool {
    return pImpl->startSlew(direction);
}

auto SkySafariController::stopSlew(Direction direction) -> bool {
    return pImpl->stopSlew(direction);
}

auto SkySafariController::park() -> bool { return pImpl->park(); }

auto SkySafariController::unpark() -> bool { return pImpl->unpark(); }

auto SkySafariController::getCurrentCoordinates() const -> Coordinates {
    return pImpl->getCurrentCoordinates();
}

auto SkySafariController::getGeographicCoordinates() const
    -> GeographicCoordinates {
    return pImpl->getGeographicCoordinates();
}

auto SkySafariController::getDateTime() const -> DateTime {
    return pImpl->getDateTime();
}

auto SkySafariController::getSlewRate() const -> SlewRate {
    return pImpl->getSlewRate();
}

auto SkySafariController::isConnected() const -> bool {
    return pImpl->isConnected();
}

auto SkySafariController::isParked() const -> bool { return pImpl->isParked(); }