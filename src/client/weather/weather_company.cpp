#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <atomic>

class WeatherCompanyClient {
public:
    WeatherCompanyClient(const std::string& apiKey, double latitude, double longitude)
        : apiKey(apiKey), latitude(latitude), longitude(longitude), connected(false) {
        spdlog::info("WeatherCompanyClient initialized.");
    }

    ~WeatherCompanyClient() {
        Disconnect();
    }

    bool Connect() {
        if (apiKey.empty()) {
            spdlog::error("No TheWeatherCompany API key configured.");
            connected = false;
            return false;
        }

        spdlog::info("Starting weather update task.");
        connected = true;
        updateWorkerThread = std::thread(&WeatherCompanyClient::UpdateWorker, this);
        return true;
    }

    void Disconnect() {
        spdlog::info("Stopping weather update task.");
        connected = false;
        if (updateWorkerThread.joinable()) {
            updateWorkerThread.join();
        }
    }

    // Getters for weather data
    double GetTemperature() const { return temperature; }
    double GetPressure() const { return pressure; }
    double GetHumidity() const { return humidity; }
    double GetWindDirection() const { return windDirection; }
    double GetWindSpeed() const { return windSpeed; }
    double GetCloudCover() const { return cloudCover; }
    double GetDewPoint() const { return ApproximateDewPoint(temperature, humidity); }

private:
    std::string apiKey;
    double latitude;
    double longitude;
    std::atomic<bool> connected;
    std::thread updateWorkerThread;

    double temperature = 0.0;
    double pressure = 0.0;
    double humidity = 0.0;
    double windDirection = 0.0;
    double windSpeed = 0.0;
    double cloudCover = 0.0;

    const std::string twcCurrentWeatherBaseURL = "https://api.weather.com/v1/";
    const double twcQueryPeriod = 600.0;  // 10 minutes

    void UpdateWorker() {
        while (connected) {
            std::string url = twcCurrentWeatherBaseURL + "geocode/" + std::to_string(latitude) + "/" + std::to_string(longitude) + "/observations.json?language=en-US&units=m&apiKey=" + apiKey;
            std::string result = PerformHttpRequest(url);
            if (!result.empty()) {
                ParseWeatherData(result);
            }

            std::this_thread::sleep_for(std::chrono::seconds(static_cast<int>(twcQueryPeriod)));
        }
    }

    std::string PerformHttpRequest(const std::string& url) {
        CURL* curl;
        CURLcode res;
        std::string response;

        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();

        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

            res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                spdlog::error("curl_easy_perform() failed: {}", curl_easy_strerror(res));
            }

            curl_easy_cleanup(curl);
        }

        curl_global_cleanup();
        return response;
    }

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    void ParseWeatherData(const std::string& jsonResponse) {
        try {
            auto json = nlohmann::json::parse(jsonResponse);

            temperature = json["observation"]["temp"].get<double>();
            pressure = json["observation"]["pressure"].get<double>();
            humidity = json["observation"]["rh"].get<double>();
            windSpeed = json["observation"]["wspd"].get<double>();
            windDirection = json["observation"]["wdir"].get<double>();
            cloudCover = ParseCloudCover(json["observation"]["clds"].get<std::string>());
        } catch (const nlohmann::json::exception& e) {
            spdlog::error("Failed to parse JSON: {}", e.what());
        }
    }

    double ParseCloudCover(const std::string& clds) {
        if (clds == "SKC") return 0;
        if (clds == "CLR") return 20;
        if (clds == "SCT") return 40;
        if (clds == "FEW") return 60;
        if (clds == "BKN") return 80;
        if (clds == "OVC") return 100;
        return 100;
    }

    double ApproximateDewPoint(double temperature, double humidity) const {
        return temperature - ((100 - humidity) / 5.0);
    }
};

int main() {
    WeatherCompanyClient client("your-api-key-here", 52.5200, 13.4050);  // Example: Berlin coordinates
    if (client.Connect()) {
        std::this_thread::sleep_for(std::chrono::seconds(5));  // Simulate some processing time

        std::cout << "Temperature: " << client.GetTemperature() << " C" << std::endl;
        std::cout << "Pressure: " << client.GetPressure() << " hPa" << std::endl;
        std::cout << "Humidity: " << client.GetHumidity() << " %" << std::endl;
        std::cout << "Dew Point: " << client.GetDewPoint() << " C" << std::endl;

        client.Disconnect();
    }

    return 0;
}
