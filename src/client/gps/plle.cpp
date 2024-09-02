#include "plle.hpp"

#include <asio.hpp>
#include <iostream>
#include <regex>

class GpsClient::Impl {
public:
    explicit Impl(const std::string& url)
        : eagleGpsUrl(url), resolver_(ioContext_), socket_(ioContext_) {}

    std::optional<Location> getLocation() {
        try {
            std::string response = fetchGpsData();

            auto json = nlohmann::json::parse(response);
            if (!json.contains("result") ||
                json["result"].get<std::string>() != "OK" ||
                json["numsat"].get<int>() < 4 ||
                json["latitude"].get<std::string>().find("--") !=
                    std::string::npos ||
                json["latitude"].get<std::string>().empty()) {
                throw GnssNoFixException("Invalid GPS data received.");
            }

            Location location;
            location.latitude =
                std::stod(cleanseValue(json["latitude"].get<std::string>()));
            location.longitude =
                std::stod(cleanseValue(json["longitude"].get<std::string>()));
            location.elevation =
                std::stod(cleanseValue(json["altitude"].get<std::string>()));

            return location;
        } catch (const std::exception& ex) {
            throw GnssFailedToConnectException(ex.what());
        }
    }

private:
    std::string eagleGpsUrl;
    asio::io_context ioContext_;
    asio::ip::tcp::resolver resolver_;
    asio::ip::tcp::socket socket_;

    std::string fetchGpsData() {
        auto endpoints = resolver_.resolve(eagleGpsUrl, "80");

        asio::connect(socket_, endpoints);

        std::string request =
            "GET /getgps HTTP/1.1\r\n"
            "Host: " +
            eagleGpsUrl +
            "\r\n"
            "Connection: close\r\n\r\n";

        asio::write(socket_, asio::buffer(request));

        asio::streambuf response;
        asio::read_until(socket_, response, "\r\n");

        std::istream responseStream(&response);
        std::string httpVersion;
        unsigned int statusCode;
        responseStream >> httpVersion >> statusCode;

        if (!responseStream || httpVersion.substr(0, 5) != "HTTP/") {
            throw std::runtime_error("Invalid response");
        }

        if (statusCode != 200) {
            throw std::runtime_error("Request failed with status code " +
                                     std::to_string(statusCode));
        }

        asio::read_until(socket_, response, "\r\n\r\n");

        std::ostringstream responseData;
        responseData << &response;

        return responseData.str();
    }

    std::string cleanseValue(const std::string& value) const {
        static const std::regex cleanup_regex("[^0-9.-]");
        return std::regex_replace(value, cleanup_regex, "");
    }
};

GpsClient::GpsClient(const std::string& eagleGpsUrl)
    : pimpl_(std::make_unique<Impl>(eagleGpsUrl)) {}

GpsClient::~GpsClient() = default;

std::optional<Location> GpsClient::getLocation() {
    return pimpl_->getLocation();
}
