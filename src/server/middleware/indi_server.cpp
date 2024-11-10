#include "indi_server.hpp"

#include "config.h"

#include <memory>

#include "config/configor.hpp"
#include "device/basic.hpp"

#include "atom/async/message_bus.hpp"
#include "atom/async/pool.hpp"
#include "atom/async/timer.hpp"
#include "atom/error/exception.hpp"
#include "atom/function/global_ptr.hpp"
#include "atom/io/file_permission.hpp"
#include "atom/log/loguru.hpp"
#include "atom/sysinfo/disk.hpp"
#include "atom/system/command.hpp"
#include "atom/system/env.hpp"
#include "atom/system/gpio.hpp"
#include "atom/system/process_manager.hpp"
#include "atom/type/json.hpp"
#include "atom/utils/print.hpp"
#include "atom/utils/qtimer.hpp"

#include "device/template/camera.hpp"
#include "device/template/filterwheel.hpp"
#include "device/template/focuser.hpp"
#include "device/template/guider.hpp"
#include "device/template/solver.hpp"
#include "device/template/telescope.hpp"

#include "utils/constant.hpp"



namespace lithium::middleware {
namespace internal {
auto clearCheckDeviceExists(const std::string& driverName) -> bool {
    LOG_F(INFO, "Middleware::indiDriverConfirm: Checking device exists");
    return true;
}

void printSystemDeviceList(device::SystemDeviceList s) {
    LOG_F(INFO,
          "Middleware::printSystemDeviceList: Printing system device list");
    std::string dpName;
    for (auto& systemDevice : s.systemDevices) {
        dpName = systemDevice.deviceIndiName;
        LOG_F(INFO,
              "Middleware::printSystemDeviceList: Device {} is connected: {}",
              dpName, systemDevice.isConnect);
    }
}

void saveSystemDeviceList(const device::SystemDeviceList& deviceList) {
    const std::string directory = "config";  // 配置文件夹名
    const std::string filename =
        directory + "/device_connect.dat";  // 在配置文件夹中创建文件

    std::ofstream outfile(filename, std::ios::binary);

    if (!outfile.is_open()) {
        std::cerr << "打开文件写入时发生错误: " << filename << std::endl;
        return;
    }

    for (const auto& device : deviceList.systemDevices) {
        // 转换 std::string 成员为 UTF-8 字符串
        const std::string& descriptionUtf8 = device.description;
        const std::string& deviceIndiNameUtf8 = device.deviceIndiName;
        const std::string& driverIndiNameUtf8 = device.driverIndiName;
        const std::string& driverFromUtf8 = device.driverForm;

        // 写入 std::string 大小信息和数据
        size_t descriptionSize = descriptionUtf8.size();
        outfile.write(reinterpret_cast<const char*>(&descriptionSize),
                      sizeof(size_t));
        outfile.write(descriptionUtf8.data(), descriptionSize);

        outfile.write(reinterpret_cast<const char*>(&device.deviceIndiGroup),
                      sizeof(int));

        size_t deviceIndiNameSize = deviceIndiNameUtf8.size();
        outfile.write(reinterpret_cast<const char*>(&deviceIndiNameSize),
                      sizeof(size_t));
        outfile.write(deviceIndiNameUtf8.data(), deviceIndiNameSize);

        size_t driverIndiNameSize = driverIndiNameUtf8.size();
        outfile.write(reinterpret_cast<const char*>(&driverIndiNameSize),
                      sizeof(size_t));
        outfile.write(driverIndiNameUtf8.data(), driverIndiNameSize);

        size_t driverFromSize = driverFromUtf8.size();
        outfile.write(reinterpret_cast<const char*>(&driverFromSize),
                      sizeof(size_t));
        outfile.write(driverFromUtf8.data(), driverFromSize);

        outfile.write(reinterpret_cast<const char*>(&device.isConnect),
                      sizeof(bool));
    }

    outfile.close();
}

void clearSystemDeviceListItem(device::SystemDeviceList& s, int index) {
    // clear one device
    LOG_F(INFO, "Middleware::clearSystemDeviceListItem: Clearing device");
    if (s.systemDevices.empty()) {
        LOG_F(INFO,
              "Middleware::clearSystemDeviceListItem: System device list is "
              "empty");
    } else {
        auto& currentDevice = s.systemDevices[index];
        currentDevice.deviceIndiGroup = -1;
        currentDevice.deviceIndiName = "";
        currentDevice.driverIndiName = "";
        currentDevice.driverForm = "";
        currentDevice.isConnect = false;
        currentDevice.driver = nullptr;
        currentDevice.description = "";
        LOG_F(INFO, "Middleware::clearSystemDeviceListItem: Device is cleared");
    }
}

void selectIndiDevice(int systemNumber, int grounpNumber) {
    std::shared_ptr<device::SystemDeviceList> systemDeviceListPtr;
    GET_OR_CREATE_PTR(systemDeviceListPtr, device::SystemDeviceList,
                      Constants::SYSTEM_DEVICE_LIST)
    systemDeviceListPtr->currentDeviceCode = systemNumber;
    std::shared_ptr<device::DriversList> driversListPtr;
    GET_OR_CREATE_PTR(driversListPtr, device::DriversList,
                      Constants::DRIVERS_LIST)
    driversListPtr->selectedGroup = grounpNumber;

    static const std::unordered_map<int, std::string> deviceDescriptions = {
        {0, "Mount"},
        {1, "Guider"},
        {2, "PoleCamera"},
        {3, ""},
        {4, ""},
        {5, ""},
        {20, "Main Camera #1"},
        {21, "CFW #1"},
        {22, "Focuser #1"},
        {23, "Lens Cover #1"}};

    auto it = deviceDescriptions.find(systemNumber);
    if (it != deviceDescriptions.end()) {
        systemDeviceListPtr->systemDevices[systemNumber].description =
            it->second;
    }

    LOG_F(INFO, "Middleware::SelectIndiDevice: Selecting device");
    LOG_F(INFO, "Middleware::SelectIndiDevice: System number: {}",
          systemNumber);

    for (auto& device : driversListPtr->devGroups[grounpNumber].devices) {
        LOG_F(INFO, "Middleware::SelectIndiDevice: Device: {}",
              device.driverName);

        std::shared_ptr<atom::async::MessageBus> messageBusPtr;
        GET_OR_CREATE_PTR(messageBusPtr, atom::async::MessageBus,
                          Constants::MESSAGE_BUS)
        messageBusPtr->publish("main", "AddDriver:" + device.driverName);
    }
}

void DeviceSelect(int systemNumber, int grounpNumber) {
    LOG_F(INFO, "Middleware::DeviceSelect: Selecting device");
    std::shared_ptr<device::SystemDeviceList> systemDeviceListPtr;
    GET_OR_CREATE_PTR(systemDeviceListPtr, device::SystemDeviceList,
                      Constants::SYSTEM_DEVICE_LIST)
    clearSystemDeviceListItem(*systemDeviceListPtr, systemNumber);
    selectIndiDevice(systemNumber, grounpNumber);
}

int getFocuserPosition() {
    std::shared_ptr<AtomFocuser> dpFocuser;
    GET_OR_CREATE_PTR(dpFocuser, AtomFocuser, Constants::MAIN_FOCUSER)
    if (dpFocuser) {
        return getFocuserPosition();
    }
    return -1;
}

void focusingLooping() {
    std::shared_ptr<AtomCamera> dpMainCamera;
    if (dpMainCamera) {
        return;
    }

    std::shared_ptr<bool> isFocusingLooping;
    GET_OR_CREATE_PTR(isFocusingLooping, bool, Constants::IS_FOCUSING_LOOPING)
    *isFocusingLooping = true;
    std::shared_ptr<ConfigManager> configManager;
    GET_OR_CREATE_PTR(configManager, ConfigManager, Constants::CONFIG_MANAGER)
    auto status = configManager->getValue("/lithium/device/camera/status")
                      ->get<std::string>();
    if (status == "Displaying") {
        double expTimeSec;
        auto expTime =
            configManager->getValue("/lithium/device/camera/current_exposure");
        if (expTime) {
            expTimeSec = expTime->get<double>() / 1000;
        } else {
            expTimeSec = 1;
        }

        configManager->setValue("/lithium/device/camera/status", "Exposuring");
        LOG_F(INFO, "Middleware::focusingLooping: Focusing looping");

        auto [x, y] = dpMainCamera->getFrame().value();
        std::array<int, 2> cameraResolution{x, y};
        auto boxSideLength =
            configManager->getValue("/lithium/device/camera/box_side_length")
                ->get<int>();
        auto [ROI_X, ROI_Y] =
            configManager->getValue("/lithium/device/camera/roi")
                ->get<std::array<int, 2>>();
        std::array<int, 2> ROI{boxSideLength, boxSideLength};
        auto [cameraX, cameraY] =
            configManager->getValue("/lithium/device/camera_frame")
                ->get<std::array<int, 2>>();
        cameraX = ROI_X * cameraResolution[0] / (double)x;
        cameraY = ROI_Y * cameraResolution[1] / (double)y;
        if (cameraX < x - ROI[0] && cameraY < y - ROI[1]) {
            dpMainCamera->setFrame(cameraX, cameraY, boxSideLength,
                                   boxSideLength);
        } else {
            LOG_F(INFO,
                  "Middleware::focusingLooping: Too close to the edge, please "
                  "reselect the area.");
            if (cameraX + ROI[0] > x) {
                cameraX = x - ROI[0];
            }
            if (cameraY + ROI[1] > y) {
                cameraY = y - ROI[1];
            }
            dpMainCamera->setFrame(cameraX, cameraY, boxSideLength,
                                   boxSideLength);
        }
        /*
        int cameraX =
            glROI_x * cameraResolution.width() / (double)CaptureViewWidth;
        int cameraY =
            glROI_y * cameraResolution.height() / (double)CaptureViewHeight;

        if (cameraX < glMainCCDSizeX - ROI.width() &&
            cameraY < glMainCCDSizeY - ROI.height()) {
            indi_Client->setCCDFrameInfo(
                dpMainCamera, cameraX, cameraY, BoxSideLength,
                BoxSideLength);  // add by CJQ 2023.2.15
            indi_Client->takeExposure(dpMainCamera, expTime_sec);
        } else {
            qDebug("Too close to the edge, please reselect the area.");  //
        TODO: if (cameraX + ROI.width() > glMainCCDSizeX) cameraX =
        glMainCCDSizeX - ROI.width(); if (cameraY + ROI.height() >
        glMainCCDSizeY) cameraY = glMainCCDSizeY - ROI.height();

            indi_Client->setCCDFrameInfo(dpMainCamera, cameraX, cameraY,
                                         ROI.width(),
                                         ROI.height());  // add by CJQ 2023.2.15
            indi_Client->takeExposure(dpMainCamera, expTime_sec);
        }
        */
        dpMainCamera->startExposure(expTimeSec);
    }
}

void focuserMove(bool isInward, int steps) {
    std::shared_ptr<AtomFocuser> dpFocuser;
    GET_OR_CREATE_PTR(dpFocuser, AtomFocuser, Constants::MAIN_FOCUSER)
    if (dpFocuser) {
        std::shared_ptr<atom::async::Timer> focusTimer;
        GET_OR_CREATE_PTR(focusTimer, atom::async::Timer, Constants::MAIN_TIMER)
        auto currentPosition = getFocuserPosition();
        int targetPosition;
        targetPosition = currentPosition + (isInward ? steps : -steps);
        LOG_F(INFO, "Focuser Move: {} -> {}", currentPosition, targetPosition);

        dpFocuser->setFocuserMoveDirection(isInward);
        dpFocuser->moveFocuserSteps(steps);

        focusTimer->setInterval(
            [&targetPosition]() {
                auto currentPosition = getFocuserPosition();
                if (currentPosition == targetPosition) {
                    LOG_F(INFO, "Focuser Move Complete!");
                    std::shared_ptr<atom::async::MessageBus> messageBusPtr;
                    GET_OR_CREATE_PTR(messageBusPtr, atom::async::MessageBus,
                                      Constants::MESSAGE_BUS)
                    messageBusPtr->publish("main", "FocuserMoveDone");
                } else {
                    LOG_F(INFO, "Focuser Moving: {} -> {}", currentPosition,
                          targetPosition);
                }
            },
            1000, 30, 0);
    }
}

int fitQuadraticCurve(const std::vector<std::pair<double, double>>& data,
                      double& a, double& b, double& c) {
    int n = data.size();
    if (n < 5) {
        return -1;  // 数据点数量不足
    }

    double sumX = 0, sumY = 0, sumX2 = 0, sumX3 = 0, sumX4 = 0;
    double sumXY = 0, sumX2Y = 0;

    for (const auto& point : data) {
        double x = point.first;
        double y = point.second;
        double x2 = x * x;
        double x3 = x2 * x;
        double x4 = x3 * x;

        sumX += x;
        sumY += y;
        sumX2 += x2;
        sumX3 += x3;
        sumX4 += x4;
        sumXY += x * y;
        sumX2Y += x2 * y;
    }

    double denom = n * (sumX2 * sumX4 - sumX3 * sumX3) -
                   sumX * (sumX * sumX4 - sumX2 * sumX3) +
                   sumX2 * (sumX * sumX3 - sumX2 * sumX2);
    if (denom == 0) {
        return -1;  // 无法拟合
    }

    a = (n * (sumX2 * sumX2Y - sumX3 * sumXY) -
         sumX * (sumX * sumX2Y - sumX2 * sumXY) +
         sumX2 * (sumX * sumXY - sumX2 * sumY)) /
        denom;
    b = (n * (sumX4 * sumXY - sumX3 * sumX2Y) -
         sumX2 * (sumX2 * sumX2Y - sumX3 * sumXY) +
         sumX3 * (sumX2 * sumY - sumX * sumXY)) /
        denom;
    c = (sumY * (sumX2 * sumX4 - sumX3 * sumX3) -
         sumX * (sumX2 * sumX2Y - sumX3 * sumXY) +
         sumX2 * (sumX2 * sumXY - sumX3 * sumY)) /
        denom;

    return 0;  // 拟合成功
}

device::SystemDeviceList readSystemDeviceList() {
    device::SystemDeviceList deviceList;
    const std::string directory = "config";
    const std::string filename =
        directory + "/device_connect.dat";  // 在配置文件夹中创建文件
    std::ifstream infile(filename, std::ios::binary);

    if (!infile.is_open()) {
        LOG_F(INFO, "Middleware::readSystemDeviceList: File not found: {}",
              filename);
        return deviceList;
    }

    while (true) {
        device::SystemDevice device;

        // 读取 description
        size_t descriptionSize;
        infile.read(reinterpret_cast<char*>(&descriptionSize), sizeof(size_t));
        if (infile.eof())
            break;
        device.description.resize(descriptionSize);
        infile.read(&device.description[0], descriptionSize);

        // 读取 deviceIndiGroup
        infile.read(reinterpret_cast<char*>(&device.deviceIndiGroup),
                    sizeof(int));

        // 读取 deviceIndiName
        size_t deviceIndiNameSize;
        infile.read(reinterpret_cast<char*>(&deviceIndiNameSize),
                    sizeof(size_t));
        device.deviceIndiName.resize(deviceIndiNameSize);
        infile.read(&device.deviceIndiName[0], deviceIndiNameSize);

        // 读取 driverIndiName
        size_t driverIndiNameSize;
        infile.read(reinterpret_cast<char*>(&driverIndiNameSize),
                    sizeof(size_t));
        device.driverIndiName.resize(driverIndiNameSize);
        infile.read(&device.driverIndiName[0], driverIndiNameSize);

        // 读取 driverForm
        size_t driverFormSize;
        infile.read(reinterpret_cast<char*>(&driverFormSize), sizeof(size_t));
        device.driverForm.resize(driverFormSize);
        infile.read(&device.driverForm[0], driverFormSize);

        // 读取 isConnect
        infile.read(reinterpret_cast<char*>(&device.isConnect), sizeof(bool));

        deviceList.systemDevices.push_back(device);
    }

    infile.close();
    return deviceList;
}

int getTotalDeviceFromSystemDeviceList(const device::SystemDeviceList& s) {
    return std::count_if(
        s.systemDevices.begin(), s.systemDevices.end(),
        [](const auto& dev) { return !dev.deviceIndiName.empty(); });
}

void cleanSystemDeviceListConnect(device::SystemDeviceList& s) {
    for (auto& device : s.systemDevices) {
        device.isConnect = false;
        device.driver = nullptr;
    }
}

void startIndiDriver(const std::string& driverName) {
    std::string s;
    s = "echo ";
    s.append("\"start ");
    s.append(driverName);
    s.append("\"");
    s.append("> /tmp/myFIFO");
    system(s.c_str());
    // qDebug() << "startIndiDriver" << driver_name;
    LOG_F(INFO, "Start INDI Driver | DriverName: {}", driverName);
}

void stopIndiDriver(const std::string& driverName) {
    std::string s;
    s = "echo ";
    s.append("\"stop ");
    s.append(driverName);
    s.append("\"");
    s.append("> /tmp/myFIFO");
    system(s.c_str());
    LOG_F(INFO, "Stop INDI Driver | DriverName: {}", driverName);
}

void stopIndiDriverAll(const device::DriversList& driver_list) {
    // before each connection. need to stop all of the indi driver
    // need to make sure disconnect all the driver for first. If the driver is
    // under operation, stop it may cause crash
    std::shared_ptr<ConfigManager> configManager;
    GET_OR_CREATE_PTR(configManager, ConfigManager, Constants::CONFIG_MANAGER)
    bool status = configManager->getValue("/lithium/server/indi/status")
                      ->get<bool>();  // get the indi server status
    if (!status) {
        LOG_F(ERROR, "stopIndiDriverAll | ERROR | INDI DRIVER NOT running");
        return;
    }

    for (const auto& group : driver_list.devGroups) {
        for (const auto& device : group.devices) {
            stopIndiDriver(device.driverName);
        }
    }
}

std::string printDevices() {
    LOG_F(INFO, "Middleware::printDevices: Printing devices");
    std::string dev;
    std::shared_ptr<device::SystemDeviceList> systemDeviceListPtr;
    GET_OR_CREATE_PTR(systemDeviceListPtr, device::SystemDeviceList,
                      Constants::SYSTEM_DEVICE_LIST)
    const auto& deviceList = systemDeviceListPtr->systemDevices;
    if (deviceList.empty()) {
        LOG_F(INFO, "Middleware::printDevices: No device exist");
    } else {
        for (size_t i = 0; i < deviceList.size(); ++i) {
            LOG_F(INFO, "Middleware::printDevices: Device: {}",
                  deviceList[i].deviceIndiName);
            if (i > 0) {
                dev.append("|");  // 添加分隔符
            }
            dev.append(deviceList[i].deviceIndiName);  // 添加设备名称
            dev.append(":");
            dev.append(std::to_string(i));  // 添加序号
        }
    }

    LOG_F(INFO, "Middleware::printDevices: Devices printed");
    return dev;
}

bool getIndexFromSystemDeviceList(const device::SystemDeviceList& s,
                                  const std::string& devname, int& index) {
    auto it = std::find_if(
        s.systemDevices.begin(), s.systemDevices.end(),
        [&devname](const auto& dev) { return dev.deviceIndiName == devname; });

    if (it != s.systemDevices.end()) {
        index = std::distance(s.systemDevices.begin(), it);
        LOG_F(INFO,
              "getIndexFromSystemDeviceList | found device in system list. "
              "device name: {} index: {}",
              devname, index);
        return true;
    } else {
        index = 0;
        LOG_F(INFO,
              "getIndexFromSystemDeviceList | not found device in system list, "
              "devname: {}",
              devname);
        return false;
    }
}

std::string getDeviceNameFromList(int index) {
    std::shared_ptr<device::SystemDeviceList> systemDeviceListPtr;
    GET_OR_CREATE_PTR(systemDeviceListPtr, device::SystemDeviceList,
                      Constants::SYSTEM_DEVICE_LIST)
    const auto& deviceNames = systemDeviceListPtr->systemDevices;
    if (index < 0 || index >= static_cast<int>(deviceNames.size())) {
        return "";
    }
    return deviceNames[index].deviceIndiName;
}

uint8_t MSB(uint16_t i) { return static_cast<uint8_t>((i >> 8) & 0xFF); }

uint8_t LSB(uint16_t i) { return static_cast<uint8_t>(i & 0xFF); }

auto callPHDWhichCamera(const std::string& Camera) -> bool {
    unsigned int vendcommand;
    unsigned int baseAddress;

    /*
    bzero(sharedmemory_phd, 1024);  // 共享内存清空

    baseAddress = 0x03;
    vendcommand = 0x0d;

    sharedmemory_phd[1] = MSB(vendcommand);
    sharedmemory_phd[2] = LSB(vendcommand);

    sharedmemory_phd[0] = 0x01;  // enable command

    int length = Camera.length() + 1;

    unsigned char addr = 0;
    // memcpy(sharedmemory_phd + baseAddress + addr, &index, sizeof(int));
    // addr = addr + sizeof(int);
    memcpy(sharedmemory_phd + baseAddress + addr, &length, sizeof(int));
    addr = addr + sizeof(int);
    memcpy(sharedmemory_phd + baseAddress + addr, Camera.c_str(), length);
    addr = addr + length;

    // wait stellarium finished this task
    QElapsedTimer t;
    t.start();

    while (sharedmemory_phd[0] == 0x01 && t.elapsed() < 500) {
        // QCoreApplication::processEvents();
    }  // wait stellarium run end

    if (t.elapsed() >= 500)
        return QHYCCD_ERROR;  // timeout
    else
        return QHYCCD_SUCCESS;
    */
    return true;
}




auto getAllFile() -> std::string {
    std::shared_ptr<ConfigManager> configManager;
    GET_OR_CREATE_PTR(configManager, ConfigManager, Constants::CONFIG_MANAGER)
    std::string imageSaveBasePath =
        configManager->getValue("/lithium/server/image_save_path")
            ->get<std::string>();
    std::string capturePath = imageSaveBasePath + "/CaptureImage/";
    std::string planPath = imageSaveBasePath + "/ScheduleImage/";

    auto getFiles = [](const std::string& path) {
        std::ostringstream oss;
        for (const auto& entry : fs::directory_iterator(path)) {
            oss << entry.path().filename().string() << ";";
        }
        return oss.str();
    };

    std::string captureString = "CaptureImage{" + getFiles(capturePath) + "}";
    std::string planString = "ScheduleImage{" + getFiles(planPath) + "}";
    return captureString + ":" + planString;
}

}  // namespace internal

auto indiDriverConfirm(const std::string& driverName) -> bool {
    LOG_F(INFO, "Middleware::indiDriverConfirm: Checking driver: {}",
          driverName);

    auto isExist = internal::clearCheckDeviceExists(driverName);
    if (!isExist) {
        std::shared_ptr<device::SystemDeviceList> systemDeviceListPtr;
        GET_OR_CREATE_PTR(systemDeviceListPtr, device::SystemDeviceList,
                          Constants::SYSTEM_DEVICE_LIST)
        auto& currentDevice =
            systemDeviceListPtr
                ->systemDevices[systemDeviceListPtr->currentDeviceCode];
        currentDevice.deviceIndiGroup = -1;
        currentDevice.deviceIndiName = "";
        currentDevice.driverIndiName = "";
        currentDevice.driverForm = "";
        currentDevice.isConnect = false;
        currentDevice.driver = nullptr;
        currentDevice.description = "";
    }
    LOG_F(INFO, "Middleware::indiDriverConfirm: Driver {} is exist: {}",
          driverName, isExist);
    return isExist;
}

void indiDeviceConfirm(const std::string& deviceName,
                       const std::string& driverName) {
    LOG_F(INFO,
          "Middleware::indiDeviceConfirm: Checking device: {} with driver: {}",
          deviceName, driverName);

    int deviceCode;
    std::shared_ptr<device::SystemDeviceList> systemDeviceListPtr;
    GET_OR_CREATE_PTR(systemDeviceListPtr, device::SystemDeviceList,
                      Constants::SYSTEM_DEVICE_LIST)
    deviceCode = systemDeviceListPtr->currentDeviceCode;

    std::shared_ptr<device::DriversList> driversListPtr;
    GET_OR_CREATE_PTR(driversListPtr, device::DriversList,
                      Constants::DRIVERS_LIST)

    auto& currentDevice = systemDeviceListPtr->systemDevices[deviceCode];
    currentDevice.driverIndiName = driverName;
    currentDevice.deviceIndiGroup = driversListPtr->selectedGroup;
    currentDevice.deviceIndiName = deviceName;

    LOG_F(INFO,
          "Middleware::indiDeviceConfirm: Device {} with driver {} is "
          "confirmed",
          deviceName, driverName);

    internal::printSystemDeviceList(*systemDeviceListPtr);

    internal::saveSystemDeviceList(*systemDeviceListPtr);
}

void printDevGroups2(const device::DriversList& driversList, int ListNum,
                     const std::string& group) {
    LOG_F(INFO, "Middleware::printDevGroups: printDevGroups2:");

    for (int index = 0; index < driversList.devGroups.size(); ++index) {
        const auto& devGroup = driversList.devGroups[index];
        LOG_F(INFO, "Middleware::printDevGroups: Group: {}",
              devGroup.groupName);

        if (devGroup.groupName == group) {
            LOG_F(INFO, "Middleware::printDevGroups: Group: {}",
                  devGroup.groupName);
            /*
            for (const auto& device : devGroup.devices) {
                LOG_F(INFO, "Middleware::printDevGroups: Device: {}",
            device.driverName); std::shared_ptr<atom::async::MessageBus>
            messageBusPtr; GET_OR_CREATE_PTR(messageBusPtr,
            atom::async::MessageBus, Constants::MESSAGE_BUS)
                messageBusPtr->publish("main", "AddDriver:" +
            device.driverName);
            }
            */
            internal::selectIndiDevice(ListNum, index);
        }
    }
}

void indiCapture(int expTime) {
    auto glIsFocusingLooping =
        GetPtr<bool>(Constants::IS_FOCUSING_LOOPING).value();
    *glIsFocusingLooping = false;
    double expTimeSec = static_cast<double>(expTime) / 1000;
    LOG_F(INFO, "INDI_Capture | exptime: {}", expTimeSec);

    auto dpMainCameraOpt = GetPtr<AtomCamera>(Constants::MAIN_CAMERA);
    if (!dpMainCameraOpt.has_value()) {
        LOG_F(ERROR, "INDI_Capture | dpMainCamera is NULL");
        return;
    }

    auto dpMainCamera = dpMainCameraOpt.value();
    auto configManagerPtr =
        GetPtr<ConfigManager>(Constants::CONFIG_MANAGER).value();
    configManagerPtr->setValue("/lithium/device/camera/status", "Exposuring");
    LOG_F(INFO, "INDI_Capture | Camera status: Exposuring");

    dpMainCamera->getGain();
    dpMainCamera->getOffset();

    auto messageBusPtr =
        GetPtr<atom::async::MessageBus>(Constants::MESSAGE_BUS).value();
    auto [x, y] = dpMainCamera->getFrame().value();
    messageBusPtr->publish("main", "MainCameraSize:{}:{}"_fmt(x, y));

    dpMainCamera->startExposure(expTimeSec);
    LOG_F(INFO, "INDI_Capture | Camera status: Exposuring");
}

void indiAbortCapture() {
    auto dpMainCameraOpt = GetPtr<AtomCamera>(Constants::MAIN_CAMERA);
    if (!dpMainCameraOpt.has_value()) {
        LOG_F(ERROR, "INDI_AbortCapture | dpMainCamera is NULL");
        return;
    }

    auto dpMainCamera = dpMainCameraOpt.value();
    dpMainCamera->abortExposure();
    LOG_F(INFO, "INDI_AbortCapture | Camera status: Aborted");
}

auto setFocusSpeed(int speed) -> int {
    std::shared_ptr<AtomFocuser> dpFocuser;
    if (dpFocuser) {
        dpFocuser->setFocuserSpeed(speed);
        auto [value, min, max] = dpFocuser->getFocuserSpeed().value();
        LOG_F(INFO, "INDI_FocusSpeed | Focuser Speed: {}, {}, {}", value, min,
              max);
        return value;
    }
    LOG_F(ERROR, "INDI_FocusSpeed | dpFocuser is NULL");
    return -1;
}

auto focusMoveAndCalHFR(bool isInward, int steps) -> double {
    double FWHM = 0;

    std::shared_ptr<ConfigManager> configManager;
    GET_OR_CREATE_PTR(configManager, ConfigManager, Constants::CONFIG_MANAGER)
    configManager->setValue("/lithium/device/focuser/calc_fwhm", false);

    internal::focuserMove(isInward, steps);

    std::shared_ptr<atom::async::Timer> focusTimer;
    GET_OR_CREATE_PTR(focusTimer, atom::async::Timer, Constants::MAIN_TIMER)

    focusTimer->setInterval(
        [&FWHM, configManager]() {
            if (configManager->getValue("/lithium/device/focuser/calc_fwhm")
                    ->get<bool>()) {
                FWHM = configManager->getValue("/lithium/device/focuser/fwhm")
                           ->get<double>();  // 假设 this->FWHM 保存了计算结果
                LOG_F(INFO, "FWHM Calculation Complete!");
            }
        },
        1000, 30, 0);

    focusTimer->wait();
    return FWHM;
}

void autofocus() {
    std::shared_ptr<ConfigManager> configManager;
    GET_OR_CREATE_PTR(configManager, ConfigManager, Constants::CONFIG_MANAGER)
    configManager->setValue("/lithium/device/focuser/auto_focus", false);

    int stepIncrement =
        configManager
            ->getValue("/lithium/device/focuser/auto_focus_step_increase")
            .value_or(100);
    LOG_F(INFO, "AutoFocus | Step Increase: {}", stepIncrement);

    bool isInward = true;
    focusMoveAndCalHFR(!isInward, stepIncrement * 5);

    int initialPosition = internal::getFocuserPosition();
    int currentPosition = initialPosition;
    int onePassSteps = 8;
    int lostStarNum = 0;
    std::vector<std::pair<double, double>> focusMeasures;

    std::shared_ptr<atom::async::MessageBus> messageBusPtr;
    GET_OR_CREATE_PTR(messageBusPtr, atom::async::MessageBus,
                      Constants::MESSAGE_BUS)

    auto stopAutoFocus = [&]() {
        LOG_F(INFO, "AutoFocus | Stop Auto Focus");
        messageBusPtr->publish("main", "AutoFocusOver:true");
    };

    for (int i = 1; i < onePassSteps; i++) {
        if (configManager->getValue("/lithium/device/focuser/auto_focus")
                .value_or(false)) {
            stopAutoFocus();
            return;
        }
        double hfr = focusMoveAndCalHFR(isInward, stepIncrement);
        LOG_F(INFO, "AutoFocus | Pass1: HFR-{}({}) Calculation Complete!", i,
              hfr);
        if (hfr == -1) {
            lostStarNum++;
            if (lostStarNum >= 3) {
                LOG_F(INFO, "AutoFocus | Too many number of lost star points.");
                // TODO: Implement FocusGotoAndCalFWHM(initialPosition -
                // stepIncrement * 5);
                LOG_F(INFO, "AutoFocus | Returned to the starting point.");
                stopAutoFocus();
                return;
            }
        }
        currentPosition = internal::getFocuserPosition();
        focusMeasures.emplace_back(currentPosition, hfr);
    }

    auto fitAndCheck = [&](double& a, double& b, double& c) -> bool {
        int result = internal::fitQuadraticCurve(focusMeasures, a, b, c);
        if (result != 0 || a >= 0) {
            LOG_F(INFO, "AutoFocus | Fit failed or parabola opens upward");
            return false;
        }
        return true;
    };

    double a;
    double b;
    double c;
    if (!fitAndCheck(a, b, c)) {
        stopAutoFocus();
        return;
    }

    int minPointX =
        configManager->getValue("/lithium/device/focuser/auto_focus_min_point")
            .value_or(0);
    int countLessThan = std::count_if(
        focusMeasures.begin(), focusMeasures.end(),
        [&](const auto& point) { return point.first < minPointX; });
    int countGreaterThan = focusMeasures.size() - countLessThan;

    if (countLessThan > countGreaterThan) {
        LOG_F(INFO, "AutoFocus | More points are less than minPointX.");
        if (a > 0) {
            focusMoveAndCalHFR(!isInward,
                               stepIncrement * (onePassSteps - 1) * 2);
        }
    } else if (countGreaterThan > countLessThan) {
        LOG_F(INFO, "AutoFocus | More points are greater than minPointX.");
        if (a < 0) {
            focusMoveAndCalHFR(!isInward,
                               stepIncrement * (onePassSteps - 1) * 2);
        }
    }

    for (int i = 1; i < onePassSteps; i++) {
        if (configManager->getValue("/lithium/device/focuser/auto_focus")
                .value_or(false)) {
            stopAutoFocus();
            return;
        }
        double hfr = focusMoveAndCalHFR(isInward, stepIncrement);
        LOG_F(INFO, "AutoFocus | Pass2: HFR-{}({}) Calculation Complete!", i,
              hfr);
        currentPosition = internal::getFocuserPosition();
        focusMeasures.emplace_back(currentPosition, hfr);
    }

    if (!fitAndCheck(a, b, c)) {
        stopAutoFocus();
        return;
    }

    int pass3Steps = std::abs(countLessThan - countGreaterThan);
    LOG_F(INFO, "AutoFocus | Pass3Steps: {}", pass3Steps);

    for (int i = 1; i <= pass3Steps; i++) {
        if (configManager->getValue("/lithium/device/focuser/auto_focus")
                .value_or(false)) {
            stopAutoFocus();
            return;
        }
        double HFR = focusMoveAndCalHFR(isInward, stepIncrement);
        LOG_F(INFO, "AutoFocus | Pass3: HFR-{}({}) Calculation Complete!", i,
              HFR);
        currentPosition = internal::getFocuserPosition();
        focusMeasures.emplace_back(currentPosition, HFR);
    }

    // TODO: Implement FocusGotoAndCalFWHM(minPointX);
    LOG_F(INFO, "Auto focus complete. Best step: {}", minPointX);
    messageBusPtr->publish("main", "AutoFocusOver:true");
}

void deviceConnect() {
    std::shared_ptr<ConfigManager> configManager;
    GET_OR_CREATE_PTR(configManager, ConfigManager, Constants::CONFIG_MANAGER)
    bool oneTouchConnect =
        configManager->getValue("/lithium/device/oneTouchConnect")
            .value_or(false);
    bool oneTouchConnectFirst =
        configManager->getValue("/lithium/device/oneTouchConnectFirst")
            .value_or(true);

    std::shared_ptr<atom::async::MessageBus> messageBusPtr;
    GET_OR_CREATE_PTR(messageBusPtr, atom::async::MessageBus,
                      Constants::MESSAGE_BUS)

    std::shared_ptr<device::SystemDeviceList> systemDeviceListPtr;
    GET_OR_CREATE_PTR(systemDeviceListPtr, device::SystemDeviceList,
                      Constants::SYSTEM_DEVICE_LIST)
    if (oneTouchConnect && oneTouchConnectFirst) {
        *systemDeviceListPtr = internal::readSystemDeviceList();
        for (int i = 0; i < 32; i++) {
            if (!systemDeviceListPtr->systemDevices[i].deviceIndiName.empty()) {
                LOG_F(INFO, "DeviceConnect | {}: {}",
                      systemDeviceListPtr->systemDevices[i].deviceIndiName,
                      systemDeviceListPtr->systemDevices[i].description);

                messageBusPtr->publish(
                    "main",
                    "updateDevices_:{}:{}"_fmt(
                        i,
                        systemDeviceListPtr->systemDevices[i].deviceIndiName));
            }
        }
        oneTouchConnectFirst = false;
        return;
    }

    if (internal::getTotalDeviceFromSystemDeviceList(*systemDeviceListPtr) ==
        0) {
        LOG_F(ERROR, "DeviceConnect | No device found");
        messageBusPtr->publish(
            "main", "ConnectFailed:no device in system device list.");
        return;
    }
    // systemDeviceListPtr->systemDevicescleanSystemDeviceListConnect(*systemDeviceListPtr);
    internal::printSystemDeviceList(*systemDeviceListPtr);

    // qApp->processEvents();
    // connect all camera on the list
    std::string driverName;

    std::vector<std::string> nameCheck;
    // disconnectIndiServer(indi_Client);

    std::shared_ptr<device::DriversList> driversListPtr;
    GET_OR_CREATE_PTR(driversListPtr, device::DriversList,
                      Constants::DRIVERS_LIST)

    internal::stopIndiDriverAll(*driversListPtr);
    int k = 3;
    while (k--) {
        LOG_F(INFO, "DeviceConnect | Wait stopIndiDriverAll...");
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    for (const auto& device : systemDeviceListPtr->systemDevices) {
        driverName = device.driverIndiName;
        if (!driverName.empty()) {
            if (std::find(nameCheck.begin(), nameCheck.end(), driverName) !=
                nameCheck.end()) {
                LOG_F(INFO,
                      "DeviceConnect | found one duplicate driver, do not "
                      "start it again: {}",
                      driverName);

            } else {
                internal::startIndiDriver(driverName);
                for (int k = 0; k < 3; ++k) {
                    LOG_F(INFO, "DeviceConnect | Wait startIndiDriver...");
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
                nameCheck.push_back(driverName);
            }
        }
    }

    // Max: Our logic is not same as QHYCCD, in our logic, one device will
    // handle an INDI CLient
    // connectIndiServer(indi_Client);

    // if (indi_Client->isServerConnected() == false) {
    //     qDebug() << "System Connect | ERROR:can not find server";
    //     return;
    // }

    std::this_thread::sleep_for(std::chrono::seconds(
        3));  // connect server will generate the callback of newDevice and
              // then put the device into list. this need take some time and it
              // is non-block

    // wait the client device list's device number match the system device
    // list's device number
    int totalDevice =
        internal::getTotalDeviceFromSystemDeviceList(*systemDeviceListPtr);
    atom::utils::ElapsedTimer timer;
    int timeoutMs = 10000;
    timer.start();
    while (timer.elapsed() < timeoutMs) {
        int connectedDevice = std::count_if(
            systemDeviceListPtr->systemDevices.begin(),
            systemDeviceListPtr->systemDevices.end(),
            [](const auto& device) { return device.driver != nullptr; });
        if (connectedDevice >= totalDevice)
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        LOG_F(INFO, "DeviceConnect | Wait for device connection...");
    }
    if (timer.elapsed() > timeoutMs) {
        LOG_F(ERROR, "DeviceConnect | Device connection timeout");
        messageBusPtr->publish(
            "main",
            "ConnectFailed:Device connected less than system device list.");
    } else {
        LOG_F(INFO, "DeviceConnect | Device connection success");
    }

    internal::printDevices();

    if (systemDeviceListPtr->systemDevices.empty()) {
        LOG_F(ERROR, "DeviceConnect | No device found");
        messageBusPtr->publish("main", "ConnectFailed:No device found.");
        return;
    }
    LOG_F(INFO, "DeviceConnect | Device connection complete");
    int index;
    int total_errors = 0;

    int connectedDevice = std::count_if(
        systemDeviceListPtr->systemDevices.begin(),
        systemDeviceListPtr->systemDevices.end(),
        [](const auto& device) { return device.driver != nullptr; });
    for (int i = 0; i < connectedDevice; i++) {
        LOG_F(INFO, "DeviceConnect | Device: {}",
              systemDeviceListPtr->systemDevices[i].deviceIndiName);

        // take one device from indi_Client detected devices and get the index
        // number in pre-selected systemDeviceListPtr->systemDevices
        auto ret = internal::getIndexFromSystemDeviceList(
            *systemDeviceListPtr, internal::getDeviceNameFromList(index),
            index);
        if (ret) {
            LOG_F(INFO, "DeviceConnect | Device: {} is connected",
                  systemDeviceListPtr->systemDevices[index].deviceIndiName);
            systemDeviceListPtr->systemDevices[index].isConnect = true;
            systemDeviceListPtr->systemDevices[index].driver->connect(
                systemDeviceListPtr->systemDevices[index].deviceIndiName, 60,
                5);

            systemDeviceListPtr->systemDevices[index].isConnect = false;
            if (index == 1) {
                internal::callPHDWhichCamera(
                    systemDeviceListPtr->systemDevices[i]
                        .driver->getName());  // PHD2 Guider Connect
            } else {
                systemDeviceListPtr->systemDevices[index].driver->connect(
                    systemDeviceListPtr->systemDevices[index].deviceIndiName,
                    60, 5);
            }
            // guider will be control by PHD2, so that the watch device should
            // exclude the guider
            // indi_Client->StartWatch(systemDeviceListPtr->systemDevices[index].dp);
        } else {
            total_errors++;
        }
    }
    if (total_errors > 0) {
        LOG_F(ERROR,
              "DeviceConnect | Error: There is some detected list is not in "
              "the pre-select system list, total mismatch device: {}",
              total_errors);
        // return;
    }

    // connecting.....
    // QElapsedTimer t;
    timer.start();
    timeoutMs = 20000 * connectedDevice;
    while (timer.elapsed() < timeoutMs) {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        int totalConnected = 0;
        for (int i = 0; i < connectedDevice; i++) {
            int index;
            auto ret = internal::getIndexFromSystemDeviceList(
                *systemDeviceListPtr, internal::getDeviceNameFromList(index),
                index);
            if (ret) {
                if (systemDeviceListPtr->systemDevices[index].driver &&
                    systemDeviceListPtr->systemDevices[index]
                        .driver->isConnected()) {
                    systemDeviceListPtr->systemDevices[index].isConnect = true;
                    totalConnected++;
                }
            } else {
                LOG_F(ERROR,
                      "DeviceConnect |Warn: {} is found in the client list but "
                      "not in pre-select system list",
                      internal::getDeviceNameFromList(index));
            }
        }

        if (totalConnected >= connectedDevice)
            break;
        // qApp->processEvents();
    }

    if (timer.elapsed() > timeoutMs) {
        LOG_F(ERROR, "DeviceConnect | ERROR: Connect time exceed (ms): {}",
              timeoutMs);
        messageBusPtr->publish("main",
                               "ConnectFailed:Device connected timeout.");
    } else {
        LOG_F(INFO, "DeviceConnect | Device connected success");
    }
    if (systemDeviceListPtr->systemDevices[0].isConnect) {
        AddPtr<AtomTelescope>(
            Constants::MAIN_TELESCOPE,
            std::static_pointer_cast<AtomTelescope>(
                systemDeviceListPtr->systemDevices[0].driver));
    }
    if (systemDeviceListPtr->systemDevices[1].isConnect) {
        AddPtr<AtomGuider>(Constants::MAIN_GUIDER,
                           std::static_pointer_cast<AtomGuider>(
                               systemDeviceListPtr->systemDevices[1].driver));
    }
    if (systemDeviceListPtr->systemDevices[2].isConnect) {
        AddPtr<AtomFilterWheel>(
            Constants::MAIN_FILTERWHEEL,
            std::static_pointer_cast<AtomFilterWheel>(
                systemDeviceListPtr->systemDevices[2].driver));
    }
    if (systemDeviceListPtr->systemDevices[20].isConnect) {
        AddPtr<AtomCamera>(Constants::MAIN_CAMERA,
                           std::static_pointer_cast<AtomCamera>(
                               systemDeviceListPtr->systemDevices[20].driver));
    }
    if (systemDeviceListPtr->systemDevices[22].isConnect) {
        AddPtr<AtomFocuser>(Constants::MAIN_FOCUSER,
                            std::static_pointer_cast<AtomFocuser>(
                                systemDeviceListPtr->systemDevices[22].driver));
    }
    // printSystemDeviceList(systemDeviceListPtr->systemDevicesiceConnect();
}

void initINDIServer() {
    atom::system::executeCommandSimple("pkill indiserver");
    atom::system::executeCommandSimple("rm -f /tmp/myFIFO");
    atom::system::executeCommandSimple("mkfifo /tmp/myFIFO");
    std::shared_ptr<atom::system::ProcessManager> processManager;
    GET_OR_CREATE_PTR(processManager, atom::system::ProcessManager,
                      Constants::PROCESS_MANAGER)
    processManager->createProcess("indiserver -v -p 7624 -f /tmp/myFIFO",
                                  "indiserver");
}



void showAllImageFolder() {
    auto files = internal::getAllFile();
    std::shared_ptr<atom::async::MessageBus> messageBusPtr;
    GET_OR_CREATE_PTR(messageBusPtr, atom::async::MessageBus,
                      Constants::MESSAGE_BUS)
    messageBusPtr->publish("main", "ShowAllImageFolder:" + files);
}


void getQTClientVersion() {
    std::shared_ptr<atom::async::MessageBus> messageBusPtr;
    GET_OR_CREATE_PTR(messageBusPtr, atom::async::MessageBus,
                      Constants::MESSAGE_BUS)

    messageBusPtr->publish(
        "main", "QTClientVersion:" + std::string(LITHIUM_VERSION_STRING));
}
}  // namespace lithium::middleware
