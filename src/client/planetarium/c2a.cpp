#include "c2a.hpp"

#include <asio.hpp>
#include <iostream>
#include <sstream>

using asio::ip::tcp;

// Coordinates
Coordinates::Coordinates(double rightAscension, double declination)
    : rightAscension(rightAscension), declination(declination) {}

// DeepSkyObject
DeepSkyObject::DeepSkyObject(const std::string& name,
                             const Coordinates& coordinates)
    : name(name), coordinates(coordinates) {}

// Location
Location::Location(double latitude, double longitude, double elevation)
    : latitude(latitude), longitude(longitude), elevation(elevation) {}

// PlanetariumException
PlanetariumException::PlanetariumException(const std::string& msg)
    : message_(msg) {}

auto PlanetariumException::what() const noexcept -> const char* {
    return message_.c_str();
}

// C2A Implementation
class C2A::Impl {
public:
    Impl(const std::string& addr, int port) : address_(addr), port_(port) {}

    auto getTarget() -> DeepSkyObject {
        try {
            tcp::socket socket(context_);
            tcp::resolver resolver(context_);
            auto endpoints = resolver.resolve(address_, std::to_string(port_));
            asio::connect(socket, endpoints);

            const std::string command = "GetRa;GetDe;\r\n";
            asio::write(socket, asio::buffer(command));

            const auto response = readResponse(socket);
            if (!response.empty()) {
                const auto coords = parseCoordinates(response);
                return {"Target", coords};
            } else {
                throw PlanetariumException(
                    "Failed to get coordinates from C2A.");
            }
        } catch (const std::exception& ex) {
            std::cerr << "Error: " << ex.what() << std::endl;
            throw;
        }
    }

    auto getSite() -> Location {
        try {
            tcp::socket socket(context_);
            tcp::resolver resolver(context_);
            auto endpoints = resolver.resolve(address_, std::to_string(port_));
            asio::connect(socket, endpoints);

            const std::string command = "GetLatitude;GetLongitude;\r\n";
            asio::write(socket, asio::buffer(command));

            const auto response = readResponse(socket);
            if (!response.empty()) {
                return parseLocation(response);
            } else {
                throw PlanetariumException(
                    "Failed to get site location from C2A.");
            }
        } catch (const std::exception& ex) {
            std::cerr << "Error: " << ex.what() << std::endl;
            throw;
        }
    }

private:
    std::string address_;
    int port_;
    asio::io_context context_;

    static auto readResponse(tcp::socket& socket) -> std::string {
        asio::streambuf buf;
        asio::read_until(socket, buf, "\r\n");
        std::istream responseStream(&buf);
        std::string response;
        std::getline(responseStream, response);
        return response;
    }

    auto parseCoordinates(const std::string& response) -> Coordinates {
        const auto TOKENS = splitResponse(response, ';');
        if (TOKENS.size() < 2) {
            throw PlanetariumException("Invalid response format.");
        }

        double rightAscension = std::stod(TOKENS[0]);
        double declination = std::stod(TOKENS[1]);

        return {rightAscension, declination};
    }

    auto parseLocation(const std::string& response) -> Location {
        const auto TOKENS = splitResponse(response, ';');
        if (TOKENS.size() < 2) {
            throw PlanetariumException("Invalid response format.");
        }

        double latitude = std::stod(TOKENS[0]);
        double longitude = std::stod(TOKENS[1]);

        return {latitude, longitude, 0.0};
    }

    static auto splitResponse(const std::string& response,
                              char delimiter) -> std::vector<std::string> {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(response);

        while (std::getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
        }

        return tokens;
    }
};

// C2A public interface
C2A::C2A(const std::string& addr, int port)
    : pimpl_(std::make_unique<Impl>(addr, port)) {}

C2A::~C2A() = default;

auto C2A::getTarget() -> DeepSkyObject { return pimpl_->getTarget(); }

auto C2A::getSite() -> Location { return pimpl_->getSite(); }