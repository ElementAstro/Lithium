#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <future>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

using namespace boost::asio;
using tcp = ip::tcp;
using json = nlohmann::json;

class Stellarium {
private:
    std::string baseUrl;
    io_context context;

public:
    Stellarium(const std::string& host, const std::string& port) {
        baseUrl = "http://" + host + ":" + port;
    }

    std::string Get(const std::string& route) {
        tcp::resolver resolver(context);
        tcp::resolver::results_type endpoints =
            resolver.resolve(baseUrl, "http");

        tcp::socket socket(context);
        connect(socket, endpoints);

        std::string request = "GET " + route +
                              " HTTP/1.1\r\n"
                              "Host: " +
                              baseUrl +
                              "\r\n"
                              "Connection: close\r\n\r\n";

        write(socket, buffer(request));

        boost::asio::streambuf response;
        read_until(socket, response, "\r\n");

        std::istream response_stream(&response);
        std::string http_version;
        unsigned int status_code;
        response_stream >> http_version >> status_code;

        if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
            throw std::runtime_error("Invalid response");
        }

        if (status_code != 200) {
            throw std::runtime_error("Request failed with status code " +
                                     std::to_string(status_code));
        }

        std::ostringstream response_data;
        response_data << &response;
        return response_data.str();
    }

    std::future<json> GetSite() {
        return std::async(std::launch::async, [this]() {
            try {
                std::string response = Get("/api/main/status");
                json jobj = json::parse(response);
                json location = jobj["location"];
                return location;
            } catch (const std::exception& ex) {
                std::cerr << "Error: " << ex.what() << std::endl;
                throw;
            }
        });
    }

    std::future<json> GetTarget() {
        return std::async(std::launch::async, [this]() {
            try {
                std::string response = Get("/api/objects/info?format=json");
                json jobj = json::parse(response);
                return jobj;
            } catch (const std::exception& ex) {
                std::cerr << "Error: " << ex.what() << std::endl;
                throw;
            }
        });
    }

    std::future<double> GetRotationAngle() {
        return std::async(std::launch::async, [this]() {
            try {
                std::string response =
                    Get("/api/stelproperty/list?format=json");
                json jobj = json::parse(response);

                bool isOcularsCcdEnabled =
                    jobj["Oculars.enableCCD"]["value"].get<bool>();
                if (!isOcularsCcdEnabled) {
                    return std::numeric_limits<double>::quiet_NaN();
                }

                double angle = jobj["Oculars.selectedCCDRotationAngle"]["value"]
                                   .get<double>();
                return 360.0 - angle;  // Reverse angle
            } catch (const std::exception& ex) {
                std::cerr << "Error: " << ex.what() << std::endl;
                throw;
            }
        });
    }
};

int main() {
    Stellarium stellarium("localhost", "8090");

    try {
        auto siteFuture = stellarium.GetSite();
        auto targetFuture = stellarium.GetTarget();
        auto rotationAngleFuture = stellarium.GetRotationAngle();

        auto site = siteFuture.get();
        auto target = targetFuture.get();
        auto rotationAngle = rotationAngleFuture.get();

        std::cout << "Site Location: " << site.dump() << std::endl;
        std::cout << "Target Info: " << target.dump() << std::endl;
        std::cout << "Rotation Angle: " << rotationAngle << " degrees"
                  << std::endl;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
    }

    return 0;
}
