#include "cdc.hpp"

#include <fmt/core.h>
#include <asio.hpp>
#include <iostream>
#include <regex>

using tcp = asio::ip::tcp;

class CartesDuCiel::Impl {
public:
    Impl(std::string addr, int prt)
        : address_(std::move(addr)), port_(prt), socket_(ioContext_) {}

    auto getTarget()
        -> std::optional<std::pair<std::string, std::pair<double, double>>> {
        try {
            std::string response = sendQuery("GETSELECTEDOBJECT\r\n");
            if (response.starts_with("OK!")) {
                auto coordinates = extractCoordinates(response);
                if (coordinates) {
                    return std::make_pair("DeepSkyObject", *coordinates);
                }
            }
            return getView();
        } catch (const std::exception& e) {
            fmt::print("Error: {}\n", e.what());
            return std::nullopt;
        }
    }

    auto getSite() -> std::optional<std::pair<double, double>> {
        try {
            std::string response = sendQuery("GETOBS\r\n");
            if (response.starts_with("OK!")) {
                auto latLong = extractLatLong(response);
                if (latLong) {
                    return latLong;
                }
            }
            return std::nullopt;
        } catch (const std::exception& e) {
            fmt::print("Error: {}\n", e.what());
            return std::nullopt;
        }
    }

private:
    std::string address_;
    int port_;
    asio::io_context ioContext_;
    tcp::socket socket_;

    auto sendQuery(const std::string& command) -> std::string {
        try {
            tcp::resolver resolver(ioContext_);
            auto endpoints = resolver.resolve(address_, std::to_string(port_));
            asio::connect(socket_, endpoints);

            asio::write(socket_, asio::buffer(command));

            asio::streambuf responseBuf;
            asio::read_until(socket_, responseBuf, "\r\n");
            std::istream responseStream(&responseBuf);
            std::string response;
            std::getline(responseStream, response);
            return response;
        } catch (const std::exception& e) {
            fmt::print("Error: {}\n", e.what());
            return "";
        }
    }

    static auto extractCoordinates(const std::string& response)
        -> std::optional<std::pair<double, double>> {
        std::regex raPattern(
            R"(([0-9]{1,2})(h|:)([0-9]{1,2})(m|:)?([0-9]{1,2}(\.[0-9]+)?)?(s|:))");
        std::regex decPattern(
            R"([\+|-]([0-9]{1,2})(d|:)([0-9]{1,2})(m|:)?([0-9]{1,2}(\.[0-9]+)?)?(s|:))");

        std::smatch raMatch;
        std::smatch decMatch;
        if (std::regex_search(response, raMatch, raPattern) &&
            std::regex_search(response, decMatch, decPattern)) {
            double rightAscension = std::stod(raMatch.str());
            double declination = std::stod(decMatch.str());
            return std::make_pair(rightAscension, declination);
        }
        return std::nullopt;
    }

    static auto extractLatLong(const std::string& response)
        -> std::optional<std::pair<double, double>> {
        std::regex latPattern(R"((?<=LAT:)[\+|-]([0-9]{1,2}):([0-9]{1,2})?)");
        std::regex lonPattern(R"((?<=LON:)[\+|-]([0-9]{1,3}):([0-9]{1,2})?)");

        std::smatch latMatch;
        std::smatch lonMatch;
        if (std::regex_search(response, latMatch, latPattern) &&
            std::regex_search(response, lonMatch, lonPattern)) {
            double latitude = std::stod(latMatch.str());
            double longitude = std::stod(lonMatch.str());
            return std::make_pair(latitude, longitude);
        }
        return std::nullopt;
    }

    auto getView()
        -> std::optional<std::pair<std::string, std::pair<double, double>>> {
        try {
            std::string raResponse = sendQuery("GETRA F\r\n");
            std::string decResponse = sendQuery("GETDEC F\r\n");

            if (raResponse.starts_with("OK!") &&
                decResponse.starts_with("OK!")) {
                double rightAscension = std::stod(raResponse.substr(3));
                double declination = std::stod(decResponse.substr(3));
                return std::make_pair(
                    "DeepSkyObject",
                    std::make_pair(rightAscension, declination));
            }
            return std::nullopt;
        } catch (const std::exception& e) {
            fmt::print("Error: {}\n", e.what());
            return std::nullopt;
        }
    }
};

CartesDuCiel::CartesDuCiel(const std::string& addr, int prt)
    : pimpl_(std::make_unique<Impl>(addr, prt)) {}

CartesDuCiel::~CartesDuCiel() = default;

auto CartesDuCiel::getTarget()
    -> std::optional<std::pair<std::string, std::pair<double, double>>> {
    return pimpl_->getTarget();
}

auto CartesDuCiel::getSite() -> std::optional<std::pair<double, double>> {
    return pimpl_->getSite();
}