#include "c2a.hpp"

#include <asio.hpp>
#include <iostream>
#include <sstream>

using asio::ip::tcp;

// Coordinates
Coordinates::Coordinates(double ra, double dec) : ra(ra), dec(dec) {}

// DeepSkyObject
DeepSkyObject::DeepSkyObject(const std::string& name, const Coordinates& coords)
    : name(name), coords(coords) {}

// Location
Location::Location(double lat, double lon, double elev)
    : latitude(lat), longitude(lon), elevation(elev) {}

// PlanetariumException
PlanetariumException::PlanetariumException(const std::string& msg)
    : message(msg) {}

const char* PlanetariumException::what() const noexcept {
    return message.c_str();
}

// C2A Implementation
class C2A::Impl {
public:
    Impl(const std::string& addr, int port) : address(addr), port(port) {}

    DeepSkyObject getTarget() {
        try {
            tcp::socket socket(context);
            tcp::resolver resolver(context);
            auto endpoints = resolver.resolve(address, std::to_string(port));
            asio::connect(socket, endpoints);

            const std::string command = "GetRa;GetDe;\r\n";
            asio::write(socket, asio::buffer(command));

            const auto response = readResponse(socket);
            if (!response.empty()) {
                const auto coords = parseCoordinates(response);
                return DeepSkyObject("Target", coords);
            } else {
                throw PlanetariumException(
                    "Failed to get coordinates from C2A.");
            }
        } catch (const std::exception& ex) {
            std::cerr << "Error: " << ex.what() << std::endl;
            throw;
        }
    }

    Location getSite() {
        try {
            tcp::socket socket(context);
            tcp::resolver resolver(context);
            auto endpoints = resolver.resolve(address, std::to_string(port));
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
    std::string address;
    int port;
    asio::io_context context;

    std::string readResponse(tcp::socket& socket) {
        asio::streambuf buf;
        asio::read_until(socket, buf, "\r\n");
        std::istream responseStream(&buf);
        std::string response;
        std::getline(responseStream, response);
        return response;
    }

    Coordinates parseCoordinates(const std::string& response) {
        const auto tokens = splitResponse(response, ';');
        if (tokens.size() < 2) {
            throw PlanetariumException("Invalid response format.");
        }

        double ra = std::stod(tokens[0]);
        double dec = std::stod(tokens[1]);

        return Coordinates(ra, dec);
    }

    Location parseLocation(const std::string& response) {
        const auto tokens = splitResponse(response, ';');
        if (tokens.size() < 2) {
            throw PlanetariumException("Invalid response format.");
        }

        double latitude = std::stod(tokens[0]);
        double longitude = std::stod(tokens[1]);

        return Location(latitude, longitude, 0.0);
    }

    std::vector<std::string> splitResponse(const std::string& response,
                                           char delimiter) {
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
    : pimpl(std::make_unique<Impl>(addr, port)) {}

C2A::~C2A() = default;

DeepSkyObject C2A::getTarget() { return pimpl->getTarget(); }

Location C2A::getSite() { return pimpl->getSite(); }
