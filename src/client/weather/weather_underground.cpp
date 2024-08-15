#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

class WeatherUnderground {
public:
    WeatherUnderground(const std::string& apiKey, const std::string& stationId)
        : apiKey(apiKey), stationId(stationId), connected(false) {}

    bool connect() {
        if (apiKey.empty() || stationId.empty()) {
            std::cerr << "API key or Station ID is not configured." << std::endl;
            return false;
        }
        connected = true;
        updateWorker = std::thread(&WeatherUnderground::updateTask, this);
        return true;
    }

    void disconnect() {
        connected = false;
        if (updateWorker.joinable()) {
            updateWorker.join();
        }
    }

    // Accessor methods
    double getTemperature() const { return temperature; }
    double getPressure() const { return pressure; }
    double getHumidity() const { return humidity; }
    double getWindDirection() const { return windDirection; }
    double getWindSpeed() const { return windSpeed; }

private:
    std::string apiKey;
    std::string stationId;
    bool connected;

    double temperature = 0.0;
    double pressure = 0.0;
    double humidity = 0.0;
    double windDirection = 0.0;
    double windSpeed = 0.0;

    std::thread updateWorker;
    static constexpr int updateInterval = 600; // 10 minutes

    void updateTask() {
        while (connected) {
            std::string url = "https://api.weather.com/v2/pws/observations/current?stationId=" + stationId + "&format=json&units=m&apiKey=" + apiKey;
            std::string response = httpRequest(url);
            if (!response.empty()) {
                parseResponse(response);
            } else {
                std::cerr << "Weather Underground API did not respond." << std::endl;
                disconnect();
            }
            std::this_thread::sleep_for(std::chrono::seconds(updateInterval));
        }
    }

    std::string httpRequest(const std::string& url) {
        CURL* curl;
        CURLcode res;
        std::string response;

        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            }
            curl_easy_cleanup(curl);
        }
        curl_global_cleanup();
        return response;
    }

    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    void parseResponse(const std::string& response) {
        auto json = nlohmann::json::parse(response);
        auto observations = json["observations"][0]["metric"];

        temperature = observations["temp"];
        pressure = observations["pressure"];
        humidity = json["observations"][0]["humidity"];
        windDirection = json["observations"][0]["winddir"];
        windSpeed = observations["windSpeed"].get<double>() * 0.2778; // Convert from kph to m/s
    }
};

int main() {
    std::string apiKey = "your_api_key";
    std::string stationId = "your_station_id";

    WeatherUnderground weather(apiKey, stationId);
    if (weather.connect()) {
        std::cout << "Connected to Weather Underground." << std::endl;

        std::this_thread::sleep_for(std::chrono::minutes(11)); // Simulate some time passing

        std::cout << "Temperature: " << weather.getTemperature() << " °C" << std::endl;
        std::cout << "Pressure: " << weather.getPressure() << " hPa" << std::endl;
        std::cout << "Humidity: " << weather.getHumidity() << " %" << std::endl;
        std::cout << "Wind Speed: " << weather.getWindSpeed() << " m/s" << std::endl;
        std::cout << "Wind Direction: " << weather.getWindDirection() << " °" << std::endl;

        weather.disconnect();
    }
    return 0;
}
