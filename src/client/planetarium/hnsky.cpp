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
    Impl(const std::string& address, int port)
        : address_(address), port_(port), socket_(ioContext_) {}

    std::string getName() const { return "HNSKY"; }

    bool canGetRotationAngle() const { return false; }

    std::future<DeepSkyObject> getTarget() {
        return std::async([this]() {
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

    std::future<Location> getSite() {
        return std::async([this]() {
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

    std::future<double> getRotationAngle() {
        return std::async(
            []() { return std::numeric_limits<double>::quiet_NaN(); });
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

    std::string sendCommand(const std::string& command) {
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

    std::vector<std::string> splitString(const std::string& s, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(s);
        while (std::getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

    double radianToHour(double radian) const { return radian * 12.0 / M_PI; }

    double radianToDegree(double radian) const { return radian * 180.0 / M_PI; }
};

// HNSKY public interface implementation

HNSKY::HNSKY(const std::string& address, int port)
    : pimpl_(std::make_unique<Impl>(address, port)) {}

HNSKY::~HNSKY() = default;

std::string HNSKY::getName() const { return pimpl_->getName(); }

bool HNSKY::canGetRotationAngle() const {
    return pimpl_->canGetRotationAngle();
}

std::future<HNSKY::DeepSkyObject> HNSKY::getTarget() {
    return pimpl_->getTarget();
}

std::future<HNSKY::Location> HNSKY::getSite() { return pimpl_->getSite(); }

std::future<double> HNSKY::getRotationAngle() {
    return pimpl_->getRotationAngle();
}
