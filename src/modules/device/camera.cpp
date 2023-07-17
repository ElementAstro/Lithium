#include "camera.hpp"

#include "loguru/loguru.hpp"

Camera::Camera(const std::string &name) : Device(name)
{
    LOG_F(INFO, "Camera Simulator Loaded : %s", name.c_str());
}

Camera::~Camera()
{
    LOG_F(INFO, "Camera Simulator Destructed");
}

bool Camera::connect(const std::string &name)
{
    LOG_F(INFO, "%s is connected", name.c_str());
    return true;
}

bool Camera::disconnect()
{
    LOG_F(INFO, "$s is disconnected", device_name.c_str());
    return true;
}

bool Camera::reconnect()
{
    return true;
}