#include "mbox.h"
#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include <memory>
#include <iostream>
#include <vector>
#include <string>
#include <regex>
#include <cstring>

#define MBOX_TIMEOUT 6
#define MBOX_BUF     64

static std::unique_ptr<MBox> mbox = std::make_unique<MBox>();

// Helper function to handle curl responses
static size_t CurlWrite_Callback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t totalSize = size * nmemb;
    s->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

// Initialize the MBox class
MBox::MBox() {
    setVersion(1, 1);
}

const char* MBox::getDefaultName() {
    return "MBox";
}

bool MBox::initProperties() {
    INDI::Weather::initProperties();

    addParameter("WEATHER_TEMPERATURE", "Temperature (C)", -10, 30, 15);
    addParameter("WEATHER_BAROMETER", "Barometer (mbar)", 20, 32.5, 15);
    addParameter("WEATHER_HUMIDITY", "Humidity %", 0, 100, 15);
    addParameter("WEATHER_DEWPOINT", "Dew Point (C)", 0, 100, 15);

    setCriticalParameter("WEATHER_TEMPERATURE");

    // Reset Calibration
    ResetSP[0].fill("RESET", "Reset", ISS_OFF);
    ResetSP.fill(getDeviceName(), "CALIBRATION_RESET", "Reset", MAIN_CONTROL_TAB, IP_RW, ISR_1OFMANY, 0, IPS_IDLE);

    // Calibration Properties
    CalibrationNP[CAL_TEMPERATURE].fill("CAL_TEMPERATURE", "Temperature", "%.f", -50, 50, 1, 0);
    CalibrationNP[CAL_PRESSURE].fill("CAL_PRESSURE", "Pressure", "%.f", -100, 100, 10, 0);
    CalibrationNP[CAL_HUMIDITY].fill("CAL_HUMIDITY", "Humidity", "%.f", -50, 50, 1, 0);
    CalibrationNP.fill(getDeviceName(), "CALIBRATION", "Calibration", MAIN_CONTROL_TAB, IP_RW, 0, IPS_IDLE);

    // Firmware Information
    FirmwareTP[0].fill("VERSION", "Version", "--");
    FirmwareTP.fill(getDeviceName(), "DEVICE_FIRMWARE", "Firmware", MAIN_CONTROL_TAB, IP_RO, 0, IPS_IDLE);

    // Add auxiliary controls, using default baud rate
    serialConnection->setDefaultBaudRate(Connection::Serial::B_38400);
    addAuxControls();

    return true;
}

bool MBox::updateProperties() {
    INDI::Weather::updateProperties();

    if (isConnected()) {
        defineProperty(CalibrationNP);
        defineProperty(ResetSP);
        defineProperty(FirmwareTP);
    } else {
        deleteProperty(CalibrationNP);
        deleteProperty(ResetSP);
        deleteProperty(FirmwareTP);
    }

    return true;
}

// Helper function to make HTTP requests using libcurl
std::string MBox::makeHttpRequest(const std::string& url) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_Callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            LOGF_ERROR("curl_easy_perform() failed: %s", curl_easy_strerror(res));
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();

    return readBuffer;
}

bool MBox::Handshake() {
    // Simulate handshake with HTTP request
    std::string response = makeHttpRequest("http://example.com/handshake");

    if (response.find("MBox") != std::string::npos) {
        getCalibration(false);
        return true;
    } else if (response.find("PXDR") != std::string::npos) {
        CalibrationNP.setState(IPS_BUSY);
        return true;
    }

    return false;
}

IPState MBox::updateWeather() {
    std::string response = makeHttpRequest("http://example.com/weather");

    if (CalibrationNP.getState() == IPS_BUSY) {
        if (getCalibration(true)) {
            CalibrationNP.setState(IPS_OK);
            CalibrationNP.apply();
        }
    }

    if (verifyCRC(response) == false) {
        LOG_ERROR("CRC check failed!");
        return IPS_ALERT;
    }

    nlohmann::json weatherData = nlohmann::json::parse(response);

    setParameterValue("WEATHER_BAROMETER", weatherData["barometer"].get<double>());
    setParameterValue("WEATHER_TEMPERATURE", weatherData["temperature"].get<double>());
    setParameterValue("WEATHER_HUMIDITY", weatherData["humidity"].get<double>());
    setParameterValue("WEATHER_DEWPOINT", weatherData["dewpoint"].get<double>());

    FirmwareTP[0].setText(weatherData["firmware"].get<std::string>().c_str());
    FirmwareTP.setState(IPS_OK);
    FirmwareTP.apply();

    return IPS_OK;
}

bool MBox::ISNewNumber(const char* dev, const char* name, double values[], char* names[], int n) {
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0) {
        if (CalibrationNP.isNameMatch(name)) {
            double prevPressure = CalibrationNP[CAL_PRESSURE].getValue();
            double prevTemperature = CalibrationNP[CAL_TEMPERATURE].getValue();
            double prevHumidaty = CalibrationNP[CAL_HUMIDITY].getValue();
            CalibrationNP.update(values, names, n);
            double targetPressure = CalibrationNP[CAL_PRESSURE].getValue();
            double targetTemperature = CalibrationNP[CAL_TEMPERATURE].getValue();
            double targetHumidity = CalibrationNP[CAL_HUMIDITY].getValue();

            bool rc = true;
            if (targetPressure != prevPressure) {
                rc = setCalibration(CAL_PRESSURE);
                usleep(200000);
            }
            if (targetTemperature != prevTemperature) {
                rc = setCalibration(CAL_TEMPERATURE);
                usleep(200000);
            }
            if (targetHumidity != prevHumidaty) {
                rc = setCalibration(CAL_HUMIDITY);
            }

            CalibrationNP.setState(rc ? IPS_OK : IPS_ALERT);
            CalibrationNP.apply();
            return true;
        }
    }

    return INDI::Weather::ISNewNumber(dev, name, values, names, n);
}

bool MBox::ISNewSwitch(const char* dev, const char* name, ISState* states, char* names[], int n) {
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0) {
        if (ResetSP.isNameMatch(name)) {
            if (resetCalibration()) {
                ResetSP.setState(IPS_OK);
                ResetSP.apply();
                LOG_INFO("Calibration values are reset.");

                CalibrationNP[CAL_PRESSURE].setValue(0);
                CalibrationNP[CAL_TEMPERATURE].setValue(0);
                CalibrationNP[CAL_HUMIDITY].setValue(0);
                CalibrationNP.setState(IPS_IDLE);
                CalibrationNP.apply();
            } else {
                ResetSP.setState(IPS_ALERT);
                ResetSP.apply();
            }

            return true;
        }
    }

    return INDI::Weather::ISNewSwitch(dev, name, states, names, n);
}

bool MBox::getCalibration(bool sendCommand) {
    std::string response;
    if (sendCommand) {
        response = makeHttpRequest("http://example.com/calibration");
    } else {
        response = "{ \"pressure\": 20, \"temperature\": 50, \"humidity\": -10 }"; // Simulated response
    }

    if (verifyCRC(response) == false) {
        LOG_ERROR("CRC check failed!");
        return false;
    }

    nlohmann::json calibrationData = nlohmann::json::parse(response);
    CalibrationNP[CAL_PRESSURE].setValue(calibrationData["pressure"].get<double>());
    CalibrationNP[CAL_TEMPERATURE].setValue(calibrationData["temperature"].get<double>());
    CalibrationNP[CAL_HUMIDITY].setValue(calibrationData["humidity"].get<double>());

    return true;
}

bool MBox::setCalibration(CalibrationType type) {
    nlohmann::json calibrationData;
    std::string url;

    if (type == CAL_PRESSURE) {
        calibrationData["pressure"] = CalibrationNP[CAL_PRESSURE].getValue();
        url = "http://example.com/setPressure";
    } else if (type == CAL_TEMPERATURE) {
        calibrationData["temperature"] = CalibrationNP[CAL_TEMPERATURE].getValue();
        url = "http://example.com/setTemperature";
    } else if (type == CAL_HUMIDITY) {
        calibrationData["humidity"] = CalibrationNP[CAL_HUMIDITY].getValue();
        url = "http://example.com/setHumidity";
    }

    std::string response = makeHttpRequest(url);

    return verifyCRC(response);
}

bool MBox::resetCalibration() {
    std::string response = makeHttpRequest("http://example.com/reset");

    return verifyCRC(response);
}

bool MBox::HandshakeCrcCheck(const char* response) {
    return strstr(response, "MBox") != nullptr;
}

bool MBox::verifyCRC(const std::string& response) {
    // Implement your CRC verification logic here
    return true; // Assuming CRC is valid
}

bool MBox::ISNewText(const char* dev, const char* name, char* texts[], char* names[], int n) {
    return INDI::Weather::ISNewText(dev, name, texts, names, n);
}

bool MBox::Connect() {
    if (Handshake()) {
        LOG_INFO("MBox is online.");
        return true;
    } else {
        LOG_ERROR("Handshake failed.");
        return false;
    }
}

bool MBox::Disconnect() {
    LOG_INFO("MBox is offline.");
    return true;
}

const char* MBox::getSwitchState(SwitchVectorProperty* sp) {
    return sp->getState() == IPS_IDLE ? "Idle" : "Active";
}

void ISGetProperties(const char* dev) {
    mbox->ISGetProperties(dev);
}

void ISNewSwitch(const char* dev, const char* name, ISState* states, char* names[], int n) {
    mbox->ISNewSwitch(dev, name, states, names, n);
}

void ISNewText(const char* dev, const char* name, char* texts[], char* names[], int n) {
    mbox->ISNewText(dev, name, texts, names, n);
}

void ISNewNumber(const char* dev, const char* name, double values[], char* names[], int n) {
    mbox->ISNewNumber(dev, name, values, names, n);
}

void ISSnoopDevice(XMLEle* root) {
    mbox->ISSnoopDevice(root);
}
