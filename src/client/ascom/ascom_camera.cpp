#include "ascom_camera.hpp"

#include "atom/log/loguru.hpp"

ASCOMCamera::ASCOMCamera(const std::string &name) :Device(name), Camera(name), ASCOMDevice(name)
{
    DLOG_F(INFO, "ASCOMCamera Simulator Loaded : %s", name.c_str());
}

ASCOMCamera::~ASCOMCamera()
{
    DLOG_F(INFO, "ASCOMCamera Simulator Destructed");
}

bool ASCOMCamera::connect(const nlohmann::json &params)
{
    return ASCOMDevice::connect(params);
}

bool ASCOMCamera::disconnect(const nlohmann::json &params)
{
    return ASCOMDevice::disconnect(params);
}

bool ASCOMCamera::reconnect(const nlohmann::json &params)
{
    return ASCOMDevice::reconnect(params);
}

bool ASCOMCamera::startExposure(const nlohmann::json &params)
{
    return true;
}

bool ASCOMCamera::abortExposure(const nlohmann::json &params)
{
    return true;
}

bool ASCOMCamera::getExposureStatus(const nlohmann::json &params)
{
    return true;
}

bool ASCOMCamera::getExposureResult(const nlohmann::json &params)
{
    return true;
}

bool ASCOMCamera::saveExposureResult(const nlohmann::json &params)
{
    return true;
}

bool ASCOMCamera::startVideo(const nlohmann::json &params)
{
    return true;
}

bool ASCOMCamera::stopVideo(const nlohmann::json &params)
{
    return true;
}

bool ASCOMCamera::getVideoStatus(const nlohmann::json &params)
{
    return true;
}

bool ASCOMCamera::getVideoResult(const nlohmann::json &params)
{
    return true;
}

bool ASCOMCamera::saveVideoResult(const nlohmann::json &params)
{
    return true;
}

bool ASCOMCamera::startCooling(const nlohmann::json &params)
{
    return true;
}

bool ASCOMCamera::stopCooling(const nlohmann::json &params)
{
    return true;
}

bool ASCOMCamera::getTemperature(const nlohmann::json &params)
{
    return true;
}

bool ASCOMCamera::getCoolingPower(const nlohmann::json &params)
{
    return true;
}

bool ASCOMCamera::setTemperature(const nlohmann::json &params)
{
    return true;
}

bool ASCOMCamera::setCoolingPower(const nlohmann::json &params)
{
    return true;
}

bool ASCOMCamera::getGain(const nlohmann::json &params)
{
    return true;
}

bool ASCOMCamera::setGain(const nlohmann::json &params)
{
    return true;
}

bool ASCOMCamera::getOffset(const nlohmann::json &params)
{
    return true;
}

bool ASCOMCamera::setOffset(const nlohmann::json &params)
{
    return true;
}

bool ASCOMCamera::getISO(const nlohmann::json &params)
{
    return true;
}

bool ASCOMCamera::setISO(const nlohmann::json &params)
{
    return true;
}

bool ASCOMCamera::getFrame(const nlohmann::json &params)
{
    return true;
}

bool ASCOMCamera::setFrame(const nlohmann::json &params)
{
    return true;
}