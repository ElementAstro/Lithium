#include "hnsky.hpp"
#include <asio.hpp>
#include <cmath>
#include <future>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using tcp = asio::ip::tcp;

class HNSKY::Impl {
public:
    Impl(std::string address, int port)
        : address_(std::move(address)), port_(port), socket_(ioContext_) {}

    [[nodiscard]] static auto getName() -> std::string { return "HNSKY"; }

    [[nodiscard]] static auto canGetRotationAngle() -> bool { return false; }

    auto getTarget() -> std::future<DeepSkyObject> {
        return std::async([this]() -> DeepSkyObject {
            std::string command = "GET_TARGET\r\n";
            auto response = sendCommand(command);
            auto info = splitString(response, ' ');

            if (info.size() < 3 || info[0] == "?") {
                throw std::runtime_error(
                    "Object not selected or invalid response.");
            }

            Coordinates coordinates{radianToHour(std::stod(info[0])),
                                    radianToDegree(std::stod(info[1]))};

            return DeepSkyObject{info[2], coordinates};
        });
    }

    auto getSite() -> std::future<Location> {
        return std::async([this]() -> Location {
            std::string command = "GET_LOCATION\r\n";
            auto response = sendCommand(command);
            auto info = splitString(response, ' ');

            if (info.size() < 2 || info[0] == "?") {
                throw std::runtime_error(
                    "Failed to get coordinates or invalid response.");
            }

            Location loc{radianToDegree(std::stod(info[1])),
                         -radianToDegree(std::stod(info[0])), 0.0};

            return loc;
        });
    }

    static auto getRotationAngle() -> std::future<double> {
        return std::async([]() -> double {
            return std::numeric_limits<double>::quiet_NaN();
        });
    }

private:
    std::string address_;
    int port_;
    asio::io_context ioContext_;
    tcp::socket socket_;

    void connect() {
        tcp::resolver resolver(ioContext_);
        asio::connect(socket_,
                      resolver.resolve(address_, std::to_string(port_)));
    }

    auto sendCommand(const std::string& command) -> std::string {
        connect();
        asio::write(socket_, asio::buffer(command));

        asio::streambuf response;
        asio::read_until(socket_, response, "\r\n");

        std::istream responseStream(&response);
        std::string responseString;
        std::getline(responseStream, responseString);
        socket_.close();

        return responseString;
    }

    static auto splitString(const std::string& str,
                            char delimiter) -> std::vector<std::string> {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(str);
        while (std::getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

    [[nodiscard]] static auto radianToHour(double radian) -> double {
        constexpr double radianToHourFactor = 12.0 / M_PI;
        return radian * radianToHourFactor;
    }

    [[nodiscard]] static auto radianToDegree(double radian) -> double {
        constexpr double radianToDegreeFactor = 180.0 / M_PI;
        return radian * radianToDegreeFactor;
    }
};

// HNSKY public interface implementation

HNSKY::HNSKY(const std::string& address, int port)
    : pimpl_(std::make_unique<Impl>(address, port)) {}

HNSKY::~HNSKY() = default;

auto HNSKY::getName() const -> std::string { return pimpl_->getName(); }

auto HNSKY::canGetRotationAngle() const -> bool {
    return pimpl_->canGetRotationAngle();
}

auto HNSKY::getTarget() -> std::future<HNSKY::DeepSkyObject> {
    return pimpl_->getTarget();
}

auto HNSKY::getSite() -> std::future<HNSKY::Location> {
    return pimpl_->getSite();
}

auto HNSKY::getRotationAngle() -> std::future<double> {
    return pimpl_->getRotationAngle();
}
