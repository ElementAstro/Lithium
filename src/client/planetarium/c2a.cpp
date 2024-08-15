#include <boost/asio.hpp>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>

using namespace boost::asio;
using ip::tcp;
using json = nlohmann::json;

// Structure to represent celestial coordinates
struct Coordinates {
    double ra;   // Right Ascension
    double dec;  // Declination

    Coordinates(double ra, double dec) : ra(ra), dec(dec) {}
};

// Structure to represent a deep sky object
struct DeepSkyObject {
    std::string name;
    Coordinates coords;

    DeepSkyObject(const std::string& name, const Coordinates& coords)
        : name(name), coords(coords) {}
};

// Structure to represent a location
struct Location {
    double latitude;
    double longitude;
    double elevation;

    Location(double lat, double lon, double elev)
        : latitude(lat), longitude(lon), elevation(elev) {}
};

// Exception class for planetarium errors
class PlanetariumException : public std::exception {
    std::string message;

public:
    PlanetariumException(const std::string& msg) : message(msg) {}

    const char* what() const noexcept override { return message.c_str(); }
};

// The main C2A class equivalent in C++
class C2A {
private:
    std::string address;
    int port;
    io_context context;

public:
    C2A(const std::string& addr, int port) : address(addr), port(port) {}

    // Method to get the target from C2A
    DeepSkyObject GetTarget() {
        try {
            tcp::socket socket(context);
            tcp::resolver resolver(context);
            auto endpoints = resolver.resolve(address, std::to_string(port));
            connect(socket, endpoints);

            std::string command = "GetRa;GetDe;\r\n";
            write(socket, buffer(command));

            std::string response = ReadResponse(socket);
            if (!response.empty()) {
                auto coords = ParseCoordinates(response);
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

    // Method to get the site location from C2A
    Location GetSite() {
        try {
            tcp::socket socket(context);
            tcp::resolver resolver(context);
            auto endpoints = resolver.resolve(address, std::to_string(port));
            connect(socket, endpoints);

            std::string command = "GetLatitude;GetLongitude;\r\n";
            write(socket, buffer(command));

            std::string response = ReadResponse(socket);
            if (!response.empty()) {
                return ParseLocation(response);
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
    // Helper method to read response from the server
    std::string ReadResponse(tcp::socket& socket) {
        boost::asio::streambuf buf;
        boost::asio::read_until(socket, buf, "\r\n");
        std::istream response_stream(&buf);
        std::string response;
        std::getline(response_stream, response);
        return response;
    }

    // Helper method to parse coordinates from response
    Coordinates ParseCoordinates(const std::string& response) {
        auto tokens = SplitResponse(response, ';');
        if (tokens.size() < 2) {
            throw PlanetariumException("Invalid response format.");
        }

        double ra = std::stod(tokens[0]);
        double dec = std::stod(tokens[1]);

        return Coordinates(ra, dec);
    }

    // Helper method to parse location from response
    Location ParseLocation(const std::string& response) {
        auto tokens = SplitResponse(response, ';');
        if (tokens.size() < 2) {
            throw PlanetariumException("Invalid response format.");
        }

        double latitude = std::stod(tokens[0]);
        double longitude = std::stod(tokens[1]);

        return Location(latitude, longitude, 0.0);
    }

    // Helper method to split response string
    std::vector<std::string> SplitResponse(const std::string& response,
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

int main() {
    // Example usage of the C2A class
    try {
        C2A c2a("localhost", 8080);
        auto target = c2a.GetTarget();
        std::cout << "Target RA: " << target.coords.ra
                  << ", Dec: " << target.coords.dec << std::endl;

        auto site = c2a.GetSite();
        std::cout << "Site Latitude: " << site.latitude
                  << ", Longitude: " << site.longitude << std::endl;
    } catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
    }

    return 0;
}
