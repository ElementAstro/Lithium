#include <iostream>
#include <string>
#include <regex>
#include <optional>
#include <stdexcept>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

// Struct to store GPS location
struct Location {
    double latitude;
    double longitude;
    double elevation;
};

// Custom exception classes
class GnssNoFixException : public std::runtime_error {
public:
    explicit GnssNoFixException(const std::string& message)
        : std::runtime_error("GNSS No Fix: " + message) {}
};

class GnssFailedToConnectException : public std::runtime_error {
public:
    explicit GnssFailedToConnectException(const std::string& message)
        : std::runtime_error("GNSS Failed to Connect: " + message) {}
};

// Helper function for libcurl to write response data
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t newLength = size * nmemb;
    try {
        s->append((char*)contents, newLength);
    } catch (std::bad_alloc&) {
        return 0;  // Out of memory
    }
    return newLength;
}

// Function to cleanse GPS data
std::string CleanseValue(const std::string& value) {
    static const std::regex cleanup_regex("[^0-9.-]");
    return std::regex_replace(value, cleanup_regex, "");
}

// Function to get GPS location from the PrimaLuceLab Eagle
std::optional<Location> GetLocation(const std::string& eagleGpsUrl) {
    CURL* curl;
    CURLcode res;
    std::string response_string;

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, eagleGpsUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            curl_easy_cleanup(curl);
            throw GnssFailedToConnectException(curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
    } else {
        throw GnssFailedToConnectException("Failed to initialize libcurl.");
    }

    try {
        auto json = nlohmann::json::parse(response_string);
        if (!json.contains("result") || json["result"].get<std::string>() == "OK" || json["numsat"].get<int>() < 4 ||
            json["latitude"].get<std::string>().find("--") != std::string::npos || json["latitude"].get<std::string>().empty()) {
            throw GnssNoFixException("Invalid GPS data received.");
        }

        Location location;
        location.latitude = std::stod(CleanseValue(json["latitude"].get<std::string>()));
        location.longitude = std::stod(CleanseValue(json["longitude"].get<std::string>()));
        location.elevation = std::stod(CleanseValue(json["altitude"].get<std::string>()));

        return location;
    } catch (const std::exception& ex) {
        throw GnssFailedToConnectException(ex.what());
    }
}

int main() {
    try {
        const std::string eagleGpsUrl = "http://localhost:1380/getgps";
        auto location = GetLocation(eagleGpsUrl);

        if (location) {
            std::cout << "Latitude: " << location->latitude << "\n"
                      << "Longitude: " << location->longitude << "\n"
                      << "Elevation: " << location->elevation << "\n";
        } else {
            std::cerr << "Failed to obtain GPS location.\n";
        }
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
    }

    return 0;
}
