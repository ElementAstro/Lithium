#include <iostream>
#include <string>
#include <regex>
#include <boost/asio.hpp>
#include <future>
#include <fmt/core.h>
#include <nlohmann/json.hpp>

using namespace boost::asio;
using tcp = ip::tcp;

class CartesDuCiel {
private:
    std::string address;
    int port;
    io_context io_context_;
    tcp::socket socket_{io_context_};

    std::string sendQuery(const std::string& command) {
        try {
            tcp::resolver resolver(io_context_);
            tcp::resolver::results_type endpoints = resolver.resolve(address, std::to_string(port));
            connect(socket_, endpoints);
            
            write(socket_, buffer(command));
            
            boost::asio::streambuf response_buf;
            read_until(socket_, response_buf, "\r\n");
            std::istream response_stream(&response_buf);
            std::string response;
            std::getline(response_stream, response);
            return response;
        } catch (const std::exception& e) {
            fmt::print("Error: {}\n", e.what());
            return "";
        }
    }

    std::optional<std::pair<double, double>> extractCoordinates(const std::string& response) {
        std::regex ra_pattern(R"(([0-9]{1,2})(h|:)([0-9]{1,2})(m|:)?([0-9]{1,2}(\.[0-9]+)?)?(s|:))");
        std::regex dec_pattern(R"([\+|-]([0-9]{1,2})(d|:)([0-9]{1,2})(m|:)?([0-9]{1,2}(\.[0-9]+)?)?(s|:))");
        
        std::smatch ra_match, dec_match;
        if (std::regex_search(response, ra_match, ra_pattern) && std::regex_search(response, dec_match, dec_pattern)) {
            double ra = std::stod(ra_match.str());
            double dec = std::stod(dec_match.str());
            return std::make_pair(ra, dec);
        } else {
            return std::nullopt;
        }
    }

public:
    CartesDuCiel(const std::string& addr, int prt) : address(addr), port(prt), socket_(io_context_) {}

    std::optional<std::pair<std::string, std::pair<double, double>>> getTarget() {
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

    std::optional<std::pair<std::string, std::pair<double, double>>> getView() {
        try {
            std::string ra_response = sendQuery("GETRA F\r\n");
            std::string dec_response = sendQuery("GETDEC F\r\n");

            if (ra_response.starts_with("OK!") && dec_response.starts_with("OK!")) {
                double ra = std::stod(ra_response.substr(3));
                double dec = std::stod(dec_response.substr(3));
                return std::make_pair("DeepSkyObject", std::make_pair(ra, dec));
            }
            return std::nullopt;
        } catch (const std::exception& e) {
            fmt::print("Error: {}\n", e.what());
            return std::nullopt;
        }
    }

    std::optional<std::pair<double, double>> getSite() {
        try {
            std::string response = sendQuery("GETOBS\r\n");
            if (response.starts_with("OK!")) {
                // Example parsing; adjust regex for real-world format
                std::regex lat_pattern(R"((?<=LAT:)[\+|-]([0-9]{1,2}):([0-9]{1,2})?)");
                std::regex lon_pattern(R"((?<=LON:)[\+|-]([0-9]{1,3}):([0-9]{1,2})?)");

                std::smatch lat_match, lon_match;
                if (std::regex_search(response, lat_match, lat_pattern) && std::regex_search(response, lon_match, lon_pattern)) {
                    double latitude = std::stod(lat_match.str());
                    double longitude = std::stod(lon_match.str());
                    return std::make_pair(latitude, longitude);
                }
            }
            return std::nullopt;
        } catch (const std::exception& e) {
            fmt::print("Error: {}\n", e.what());
            return std::nullopt;
        }
    }
};

int main() {
    CartesDuCiel cdc("localhost", 3292);
    
    auto target = cdc.getTarget();
    if (target) {
        fmt::print("Target: {} - RA: {}, Dec: {}\n", target->first, target->second.first, target->second.second);
    } else {
        fmt::print("No target selected or error occurred.\n");
    }
    
    auto site = cdc.getSite();
    if (site) {
        fmt::print("Site - Latitude: {}, Longitude: {}\n", site->first, site->second);
    } else {
        fmt::print("Failed to get site information.\n");
    }

    return 0;
}
