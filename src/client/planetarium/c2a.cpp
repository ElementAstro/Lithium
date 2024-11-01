#include "c2a.hpp"
#include <loguru.hpp>
#include <asio.hpp>
#include <iostream>
#include <sstream>

using asio::ip::tcp;

// Coordinates
Coordinates::Coordinates(double rightAscension, double declination)
    : rightAscension(rightAscension), declination(declination) {
    LOG_F(INFO, "Coordinates created: RA={}, Dec={}", rightAscension, declination);
}

// DeepSkyObject
DeepSkyObject::DeepSkyObject(const std::string& name, const Coordinates& coordinates)
    : name(name), coordinates(coordinates) {
    LOG_F(INFO, "DeepSkyObject created: Name={}", name);
}

// Location
Location::Location(double latitude, double longitude, double elevation)
    : latitude(latitude), longitude(longitude), elevation(elevation) {
    LOG_F(INFO, "Location created: Lat={}, Lon={}, Elev={}", latitude, longitude, elevation);
}

// PlanetariumException
PlanetariumException::PlanetariumException(const std::string& msg)
    : message_(msg) {
    LOG_F(ERROR, "PlanetariumException: {}", msg);
}

auto PlanetariumException::what() const noexcept -> const char* {
    return message_.c_str();
}

// C2A Implementation
class C2A::Impl {
public:
    Impl(const std::string& addr, int port) : address_(addr), port_(port) {
        LOG_F(INFO, "C2A::Impl created: Address={}, Port={}", addr, port);
    }

    auto getTarget() -> DeepSkyObject {
        LOG_F(INFO, "getTarget called");
        try {
            tcp::socket socket(context_);
            tcp::resolver resolver(context_);
            auto endpoints = resolver.resolve(address_, std::to_string(port_));
            asio::connect(socket, endpoints);

            const std::string COMMAND = "GetRa;GetDe;\r\n";
            asio::write(socket, asio::buffer(COMMAND));

            const auto response = readResponse(socket);
            if (!response.empty()) {
                const auto COORDS = parseCoordinates(response);
                LOG_F(INFO, "getTarget successful");
                return {"Target", COORDS};
            } else {
                throw PlanetariumException("Failed to get coordinates from C2A.");
            }
        } catch (const std::exception& ex) {
            LOG_F(ERROR, "Error in getTarget: {}", ex.what());
            throw;
        }
    }

    auto getSite() -> Location {
        LOG_F(INFO, "getSite called");
        try {
            tcp::socket socket(context_);
            tcp::resolver resolver(context_);
            auto endpoints = resolver.resolve(address_, std::to_string(port_));
            asio::connect(socket, endpoints);

            const std::string COMMAND = "GetLatitude;GetLongitude;\r\n";
            asio::write(socket, asio::buffer(COMMAND));

            const auto RESPONSE = readResponse(socket);
            if (!RESPONSE.empty()) {
                LOG_F(INFO, "getSite successful");
                return parseLocation(RESPONSE);
            } else {
                throw PlanetariumException("Failed to get site location from C2A.");
            }
        } catch (const std::exception& ex) {
            LOG_F(ERROR, "Error in getSite: {}", ex.what());
            throw;
        }
    }

private:
    std::string address_;
    int port_;
    asio::io_context context_;

    static auto readResponse(tcp::socket& socket) -> std::string {
        LOG_F(INFO, "readResponse called");
        asio::streambuf buf;
        asio::read_until(socket, buf, "\r\n");
        std::istream responseStream(&buf);
        std::string response;
        std::getline(responseStream, response);
        LOG_F(INFO, "readResponse: {}", response);
        return response;
    }

    static auto parseCoordinates(const std::string& response) -> Coordinates {
        LOG_F(INFO, "parseCoordinates called with response: {}", response);
        const auto TOKENS = splitResponse(response, ';');
        if (TOKENS.size() < 2) {
            throw PlanetariumException("Invalid response format.");
        }

        double rightAscension = std::stod(TOKENS[0]);
        double declination = std::stod(TOKENS[1]);

        LOG_F(INFO, "parseCoordinates successful: RA={}, Dec={}", rightAscension, declination);
        return {rightAscension, declination};
    }

    static auto parseLocation(const std::string& response) -> Location {
        LOG_F(INFO, "parseLocation called with response: {}", response);
        const auto TOKENS = splitResponse(response, ';');
        if (TOKENS.size() < 2) {
            throw PlanetariumException("Invalid response format.");
        }

        double latitude = std::stod(TOKENS[0]);
        double longitude = std::stod(TOKENS[1]);

        LOG_F(INFO, "parseLocation successful: Lat={}, Lon={}", latitude, longitude);
        return {latitude, longitude, 0.0};
    }

    static auto splitResponse(const std::string& response, char delimiter) -> std::vector<std::string> {
        LOG_F(INFO, "splitResponse called with response: {}", response);
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(response);

        while (std::getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
        }

        LOG_F(INFO, "splitResponse successful");
        return tokens;
    }
};

C2A::C2A(const std::string& addr, int port)
    : pimpl_(std::make_unique<Impl>(addr, port)) {
    LOG_F(INFO, "C2A created: Address={}, Port={}", addr, port);
}

C2A::~C2A() {
    LOG_F(INFO, "C2A destroyed");
}

auto C2A::getTarget() -> DeepSkyObject {
    LOG_F(INFO, "C2A::getTarget called");
    return pimpl_->getTarget();
}

auto C2A::getSite() -> Location {
    LOG_F(INFO, "C2A::getSite called");
    return pimpl_->getSite();
}
