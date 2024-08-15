#include <iostream>
#include <string>
#include <memory>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

// Alias for nlohmann::json
using json = nlohmann::json;

// Enum for weather safety status
enum WeatherSafetyStatus
{
    SAFE = 1,
    UNSAFE = 0
};

class WeatherSafetyProxy
{
public:
    WeatherSafetyProxy();
    ~WeatherSafetyProxy();

    bool connect();
    bool disconnect();
    void updateWeather();

private:
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* userp);
    WeatherSafetyStatus executeCurl();
    WeatherSafetyStatus parseSafetyJSON(const std::string& buffer);

    WeatherSafetyStatus safetyStatus;
    int softErrorCount;
    int softErrorMax;
    bool softErrorRecoveryMode;
    std::string weatherSafetyURL;
};

WeatherSafetyProxy::WeatherSafetyProxy() : safetyStatus(UNSAFE), softErrorCount(0), softErrorMax(30), softErrorRecoveryMode(false)
{
    // Initialize with default URL
    weatherSafetyURL = "http://0.0.0.0:5000/weather/safety";
}

WeatherSafetyProxy::~WeatherSafetyProxy() {}

bool WeatherSafetyProxy::connect()
{
    // Perform any connection setup here
    std::cout << "Connected to Weather Safety Proxy.\n";
    return true;
}

bool WeatherSafetyProxy::disconnect()
{
    // Perform any disconnection cleanup here
    std::cout << "Disconnected from Weather Safety Proxy.\n";
    return true;
}

void WeatherSafetyProxy::updateWeather()
{
    WeatherSafetyStatus newStatus = executeCurl();

    if (newStatus != safetyStatus)
    {
        if (newStatus == UNSAFE)
        {
            std::cout << "Weather is UNSAFE.\n";
        }
        else if (newStatus == SAFE)
        {
            if (softErrorRecoveryMode)
            {
                softErrorCount++;
                if (softErrorCount > softErrorMax)
                {
                    std::cout << "Soft error recovery count exceeded. Weather is SAFE.\n";
                    softErrorRecoveryMode = false;
                }
                else
                {
                    std::cout << "Weather is SAFE but recovering from errors.\n";
                    newStatus = UNSAFE;
                }
            }
            else
            {
                std::cout << "Weather is SAFE.\n";
            }
        }
        safetyStatus = newStatus;
    }
}

size_t WeatherSafetyProxy::writeCallback(void* contents, size_t size, size_t nmemb, std::string* userp)
{
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

WeatherSafetyStatus WeatherSafetyProxy::executeCurl()
{
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, weatherSafetyURL.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << "\n";
            curl_easy_cleanup(curl);
            return UNSAFE;
        }

        curl_easy_cleanup(curl);
        return parseSafetyJSON(readBuffer);
    }
    else
    {
        std::cerr << "curl_easy_init() failed.\n";
        return UNSAFE;
    }
}

WeatherSafetyStatus WeatherSafetyProxy::parseSafetyJSON(const std::string& buffer)
{
    try
    {
        json report = json::parse(buffer);
        int newSafety;
        report.at("roof_status").at("open_ok").get_to(newSafety);

        return static_cast<WeatherSafetyStatus>(newSafety);
    }
    catch (json::exception& e)
    {
        std::cerr << "JSON parsing error: " << e.what() << "\n";
        return UNSAFE;
    }
}

int main()
{
    auto weatherSafetyProxy = std::make_unique<WeatherSafetyProxy>();

    if (weatherSafetyProxy->connect())
    {
        weatherSafetyProxy->updateWeather();
        weatherSafetyProxy->disconnect();
    }

    return 0;
}
