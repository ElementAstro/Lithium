#ifndef URANUSMETEO_H
#define URANUSMETEO_H

#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <nlohmann/json.hpp>
#include <curl/curl.h>

class UranusMeteo
{
public:
    UranusMeteo();
    bool initProperties();
    bool updateProperties();
    const char *getDefaultName();
    bool Handshake();
    bool readSensors();
    bool readSkyQuality();
    bool readClouds();
    bool sendCommand(const std::string &cmd, std::string &res);

private:
    void measureSkyQuality();
    std::vector<std::string> split(const std::string &input, const std::string &regex);
    void TimerHit();
    void updateWeather();
    bool updateGPS();
    bool saveConfigItems(FILE *fp);
    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
    
    int PortFD = -1;
    bool m_SetupComplete = false;
    std::thread m_SkyQualityUpdateThread;
    std::vector<std::string> m_Sensors, m_SkyQuality, m_Clouds;
    std::string m_GPSTime;

    nlohmann::json SensorData;
};

#endif // URANUSMETEO_H

#include "uranusmeteo.h"
#include <iostream>
#include <regex>
#include <ctime>
#include <chrono>
#include <thread>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <curl/curl.h>

UranusMeteo::UranusMeteo()
{
    // Initialization code
}

bool UranusMeteo::initProperties()
{
    // Initialize properties using JSON structure
    SensorData["Sensors"] = nlohmann::json::array();
    SensorData["Clouds"] = nlohmann::json::array();
    SensorData["SkyQuality"] = nlohmann::json::array();
    SensorData["GPS"] = nlohmann::json::array();
    
    // Start the Sky Quality Update Thread
    m_SkyQualityUpdateThread = std::thread(&UranusMeteo::measureSkyQuality, this);
    return true;
}

bool UranusMeteo::updateProperties()
{
    if (isConnected())
    {
        // Define properties based on JSON data
        readSensors();
        readClouds();
        measureSkyQuality();
        m_SetupComplete = true;
    }
    else
    {
        m_SetupComplete = false;
    }

    return true;
}

const char *UranusMeteo::getDefaultName()
{
    return "Uranus Meteo Sensor";
}

bool UranusMeteo::Handshake()
{
    std::string response;
    if (sendCommand("M#", response))
    {
        return (response == "MS_OK");
    }
    return false;
}

bool UranusMeteo::readSensors()
{
    std::string response;
    if (sendCommand("MA", response))
    {
        try
        {
            m_Sensors = split(response.substr(6), ":");
            // Process and store sensor data in JSON
            SensorData["Sensors"] = m_Sensors;
            return true;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Failed to process sensor response: " << e.what() << std::endl;
            return false;
        }
    }
    return false;
}

bool UranusMeteo::readSkyQuality()
{
    std::string response;
    if (sendCommand("SQ", response))
    {
        try
        {
            m_SkyQuality = split(response.substr(3), ":");
            // Process and store sky quality data in JSON
            SensorData["SkyQuality"] = m_SkyQuality;
            return true;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Failed to process sky quality response: " << e.what() << std::endl;
            return false;
        }
    }
    return false;
}

bool UranusMeteo::readClouds()
{
    std::string response;
    if (sendCommand("CI", response))
    {
        try
        {
            m_Clouds = split(response.substr(3), ":");
            // Process and store cloud data in JSON
            SensorData["Clouds"] = m_Clouds;
            return true;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Failed to process cloud response: " << e.what() << std::endl;
            return false;
        }
    }
    return false;
}

bool UranusMeteo::sendCommand(const std::string &cmd, std::string &res)
{
    CURL *curl;
    CURLcode result;
    curl = curl_easy_init();
    if(curl)
    {
        std::string readBuffer;
        curl_easy_setopt(curl, CURLOPT_URL, cmd.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        result = curl_easy_perform(curl);
        if(result == CURLE_OK)
        {
            res = readBuffer;
            curl_easy_cleanup(curl);
            return true;
        }
        curl_easy_cleanup(curl);
    }
    return false;
}

size_t UranusMeteo::WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

std::vector<std::string> UranusMeteo::split(const std::string &input, const std::string &regex)
{
    std::regex re(regex);
    std::sregex_token_iterator
    first{input.begin(), input.end(), re, -1},
          last;
    return {first, last};
}

void UranusMeteo::measureSkyQuality()
{
    while (true)
    {
        if (!isConnected() || !m_SetupComplete)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
        readSkyQuality();
        std::this_thread::sleep_for(std::chrono::seconds(60));
    }
}
