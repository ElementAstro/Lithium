#include <curl/curl.h>
#include <chrono>
#include <future>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>

using json = nlohmann::json;

size_t WriteCallback(void* contents, size_t size, size_t nmemb,
                     std::string* s) {
    s->append(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

std::string GetHttpResponse(const std::string& url) {
    CURL* curl;
    CURLcode res;
    std::string response_string;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    return response_string;
}

class OpenWeatherMap {
public:
    OpenWeatherMap(const std::string& apiKey, double latitude, double longitude)
        : apiKey(apiKey),
          latitude(latitude),
          longitude(longitude),
          connected(false) {}

    void Connect() {
        connected = true;
        updateTask = std::async(std::launch::async,
                                &OpenWeatherMap::UpdateWeatherData, this);
    }

    void Disconnect() {
        connected = false;
        if (updateTask.valid()) {
            updateTask.wait();
        }
    }

    bool IsConnected() const { return connected; }

    // Data accessors
    double GetTemperature() const { return temperature; }
    double GetPressure() const { return pressure; }
    double GetHumidity() const { return humidity; }
    double GetWindSpeed() const { return windSpeed; }
    double GetWindDirection() const { return windDirection; }
    double GetCloudCover() const { return cloudCover; }

private:
    std::string apiKey;
    double latitude;
    double longitude;
    bool connected;
    std::future<void> updateTask;

    // Weather data
    double temperature = 0.0;
    double pressure = 0.0;
    double humidity = 0.0;
    double windSpeed = 0.0;
    double windDirection = 0.0;
    double cloudCover = 0.0;

    void UpdateWeatherData() {
        while (connected) {
            std::string url =
                "https://api.openweathermap.org/data/2.5/weather?lat=" +
                std::to_string(latitude) + "&lon=" + std::to_string(longitude) +
                "&appid=" + apiKey;

            std::string response = GetHttpResponse(url);

            auto weatherData = json::parse(response);
            temperature = weatherData["main"]["temp"].get<double>() - 273.15;
            pressure = weatherData["main"]["pressure"].get<double>();
            humidity = weatherData["main"]["humidity"].get<double>();
            windSpeed = weatherData["wind"]["speed"].get<double>();
            windDirection = weatherData["wind"]["deg"].get<double>();
            cloudCover = weatherData["clouds"]["all"].get<double>();

            std::this_thread::sleep_for(std::chrono::seconds(600));
        }
    }
};

int main() {
    std::string apiKey = "your_openweathermap_api_key";
    double latitude = 52.5200;   // Example latitude
    double longitude = 13.4050;  // Example longitude

    OpenWeatherMap weather(apiKey, latitude, longitude);
    weather.Connect();

    std::this_thread::sleep_for(std::chrono::minutes(2));

    if (weather.IsConnected()) {
        std::cout << "Temperature: " << weather.GetTemperature() << " °C\n";
        std::cout << "Pressure: " << weather.GetPressure() << " hPa\n";
        std::cout << "Humidity: " << weather.GetHumidity() << " %\n";
        std::cout << "Wind Speed: " << weather.GetWindSpeed() << " m/s\n";
        std::cout << "Wind Direction: " << weather.GetWindDirection()
                  << " degrees\n";
        std::cout << "Cloud Cover: " << weather.GetCloudCover() << " %\n";
    }

    weather.Disconnect();

    return 0;
}
