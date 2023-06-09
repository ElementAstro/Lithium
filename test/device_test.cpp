// Camera.h

#include "device.hpp"

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

private:
    std::string _deviceName;
    int _id;
};

// Camera.cpp

Camera::Camera(const std::string &name) : Device(name)
{
    _deviceName = "Camera";
    _id = 0;
}

bool Camera::connect(std::string name)
{
    // 实现连接设备的代码，如果连接成功则返回true，否则返回false
    return true;
}

bool Camera::disconnect()
{
    // 实现断开设备连接的代码，如果断开成功则返回true，否则返回false
    return true;
}

bool Camera::reconnect()
{
    // 实现重新连接设备的代码，如果重新连接成功则返回true，否则返回false
    return true;
}

bool Camera::scanForAvailableDevices()
{
    // 实现扫描可用设备的代码，如果扫描成功则返回true，否则返回false
    return true;
}

bool Camera::getSettings()
{
    // 实现获取设备设置的代码，如果获取成功则返回true，否则返回false
    return true;
}

bool Camera::saveSettings()
{
    // 实现保存设备设置的代码，如果保存成功则返回true，否则返回false
    return true;
}

bool Camera::getParameter(const std::string &paramName)
{
    // 实现获取设备参数的代码，如果获取成功则返回true，否则返回false
    return true;
}

bool Camera::setParameter(const std::string &paramName, const std::string &paramValue)
{
    // 实现设置设备参数的代码，如果设置成功则返回true，否则返回false
    return true;
}

std::shared_ptr<OpenAPT::SimpleTask> Camera::getSimpleTask(const std::string &task_name, const nlohmann::json &params)
{
    if (task_name == "captureImage")
    {
        auto imageCaptureTask = std::make_shared<OpenAPT::SimpleTask>(
            // 函数指针，封装拍照操作
            [](const nlohmann::json &params) -> nlohmann::json
            {
                // 调用相机的拍照功能
                // ...
                std::cout << "Image captured" << std::endl;
                // 返回拍照结果
                return {"result", "success"};
            },
            // 参数
            params,
            // 停止函数
            [this]()
            {
                // 调用停止相机拍照功能
                // ...
                std::cout << "Image capture stopped" << std::endl;
            },
            // 可停止（可选）
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
    // 实现获取 ConditionalTask 的代码，返回 ConditionalTask 指针
    return nullptr;
}

std::shared_ptr<OpenAPT::LoopTask> Camera::getLoopTask(const std::string &task_name, const nlohmann::json &params)
{
    // 实现获取 LoopTask 的代码，返回 LoopTask 指针
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

// 调用方法

int main()
{
    // 创建相机实例
    std::shared_ptr<Camera> camera = std::make_shared<Camera>("myCamera");

    // 连接相机
    bool connectResult = camera->connect("myCamera");
    if (connectResult)
    {
        std::cout << "Connect to camera successfully" << std::endl;
    }
    else
    {
        std::cout << "Fail to connect to camera" << std::endl;
    }

    // 获取相机名称
    std::string cameraName = camera->getName();
    std::cout << "Camera name: " << cameraName << std::endl;

    camera->getSimpleTask("captureImage",{})->Execute();

    // 断开相机连接
    bool disconnectResult = camera->disconnect();
    if (disconnectResult)
    {
        std::cout << "Disconnect from camera successfully" << std::endl;
    }
    else
    {
        std::cout << "Fail to disconnect from camera" << std::endl;
    }

    return 0;
}
