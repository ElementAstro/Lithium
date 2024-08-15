#include <boost/asio.hpp>
#include <future>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

class HNSKY {
public:
    HNSKY(const std::string& address, int port)
        : address(address), port(port), io_context(), socket(io_context) {}

    std::string getName() const { return "HNSKY"; }

    bool canGetRotationAngle() const { return false; }

    struct Coordinates {
        double ra;
        double dec;
    };

    struct DeepSkyObject {
        std::string name;
        Coordinates coordinates;
    };

    struct Location {
        double latitude;
        double longitude;
        double elevation;
    };

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
    std::string address;
    int port;
    boost::asio::io_context io_context;
    boost::asio::ip::tcp::socket socket;

    void connect() {
        boost::asio::ip::tcp::resolver resolver(io_context);
        boost::asio::connect(socket,
                             resolver.resolve(address, std::to_string(port)));
    }

    std::string sendCommand(const std::string& command) {
        connect();
        boost::asio::write(socket, boost::asio::buffer(command));

        boost::asio::streambuf response;
        boost::asio::read_until(socket, response, "\r\n");

        std::istream response_stream(&response);
        std::string response_string;
        std::getline(response_stream, response_string);
        socket.close();

        return response_string;
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

    double radianToHour(double radian) { return radian * 12.0 / M_PI; }

    double radianToDegree(double radian) { return radian * 180.0 / M_PI; }
};

int main() {
    // Example usage
    HNSKY hnsky("127.0.0.1", 12345);

    try {
        auto target = hnsky.getTarget().get();
        std::cout << "Target Name: " << target.name << std::endl;
        std::cout << "RA: " << target.coordinates.ra
                  << ", Dec: " << target.coordinates.dec << std::endl;

        auto location = hnsky.getSite().get();
        std::cout << "Latitude: " << location.latitude
                  << ", Longitude: " << location.longitude << std::endl;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
    }

    return 0;
}
