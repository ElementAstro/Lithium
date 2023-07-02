#include "device.hpp"

#include <iostream>

class Camera : public Device
{
public:
    explicit Camera(const std::string &name);

    bool connect(std::string name) override;

    bool disconnect() override;

    bool reconnect() override;

    bool scanForAvailableDevices() override;

    bool getSettings() override;

    bool saveSettings() override;

    bool getParameter(const std::string &paramName) override;

    bool setParameter(const std::string &paramName, const std::string &paramValue) override;

    std::shared_ptr<OpenAPT::SimpleTask> getSimpleTask(const std::string &task_name, const nlohmann::json &params) override;

    std::shared_ptr<OpenAPT::ConditionalTask> getCondtionalTask(const std::string &task_name, const nlohmann::json &params) override;

    std::shared_ptr<OpenAPT::LoopTask> getLoopTask(const std::string &task_name, const nlohmann::json &params) override;

    std::string getName() const override;

    void setName(const std::string &name) override;

    std::string getDeviceName() override;

    void setDeviceName(const std::string &name) override;
    void setId(int id) override;

    bool startExposure(const nlohmann::json &params)
    {
        std::cout << "aaaa" << std::endl;
        return true;
    }

private:
    std::string _deviceName;
    int _id;
};

Camera::Camera(const std::string &name) : Device(name)
{
    _deviceName = "Camera";
    IAInsertMessage(IACreateMessage("name", name), "");
    IAInsertMessage(IACreateMessage("exposure", 1), "captureImage");
    IAInsertMessage(IACreateMessage("gain", 50), "");
    IAInsertMessage(IACreateMessage("offset", 20), "");
    IAInsertMessage(IACreateMessage("width", 0), "");
    IAInsertMessage(IACreateMessage("height", 0), "");
}

bool Camera::connect(std::string name)
{
    return true;
}

bool Camera::disconnect()
{
    return true;
}

bool Camera::reconnect()
{
    return true;
}

bool Camera::scanForAvailableDevices()
{
    return true;
}

bool Camera::getSettings()
{
    return true;
}

bool Camera::saveSettings()
{
    return true;
}

bool Camera::getParameter(const std::string &paramName)
{
    return true;
}

bool Camera::setParameter(const std::string &paramName, const std::string &paramValue)
{
    return true;
}

std::shared_ptr<OpenAPT::SimpleTask> Camera::getSimpleTask(const std::string &task_name, const nlohmann::json &params)
{
    if (task_name == "captureImage")
    {
        auto imageCaptureTask = std::make_shared<OpenAPT::SimpleTask>(
            [this](const nlohmann::json &params) -> nlohmann::json
            {
                std::cout << "Image captured" << std::endl;
                IAUpdateMessage("exposure", IACreateMessage("exposure", 1));
                IAUpdateMessage("test", IACreateMessage("test", 1));
                return {"result", "success"};
            },
            params,
            [this]()
            {
                std::cout << "Image capture stopped" << std::endl;
            },
            true);
        return imageCaptureTask;
    }
    else
    {
        return nullptr;
    }
}

std::shared_ptr<OpenAPT::ConditionalTask> Camera::getCondtionalTask(const std::string &task_name, const nlohmann::json &params)
{
    return nullptr;
}

std::shared_ptr<OpenAPT::LoopTask> Camera::getLoopTask(const std::string &task_name, const nlohmann::json &params)
{
    return nullptr;
}

std::string Camera::getName() const
{
    return Device::_name;
}

void Camera::setName(const std::string &name)
{
    Device::_name = name;
}

std::string Camera::getDeviceName()
{
    return _deviceName;
}

void Camera::setDeviceName(const std::string &name)
{
    _deviceName = name;
}

void Camera::setId(int id)
{
    _id = id;
}

void MyObserver(const OpenAPT::Property::IMessage &newMessage, const OpenAPT::Property::IMessage &oldMessage)
{
    std::cout << "Observer called with new message: " << newMessage.GetMessageUUID() << std::endl;
    std::cout << "Old message: " << oldMessage.GetMessageUUID() << std::endl;
    std::cout << "Old message: " << oldMessage.getValue<int>() << std::endl;
    std::cout << "Old message: " << oldMessage.GetDeviceUUID() << std::endl;
}

int main()
{
    std::shared_ptr<Camera> camera = std::make_shared<Camera>("myCamera");

    camera->observers.push_back(MyObserver);

    bool connectResult = camera->connect("myCamera");
    if (connectResult)
    {
        std::cout << "Connect to camera successfully" << std::endl;
    }
    else
    {
        std::cout << "Fail to connect to camera" << std::endl;
    }

    std::string cameraName = camera->getName();
    std::cout << "Camera name: " << cameraName << std::endl;

    camera->getSimpleTask("captureImage", {})->Execute();

    bool disconnectResult = camera->disconnect();
    if (disconnectResult)
    {
        std::cout << "Disconnect from camera successfully" << std::endl;
    }
    else
    {
        std::cout << "Fail to disconnect from camera" << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}
