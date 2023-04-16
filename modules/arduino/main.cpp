#include <iostream>
#include <string>

#include "wrapper.hpp"

#ifdef _WIN32
#include <windows.h>
#include <SetupAPI.h>
#include <devguid.h>

std::string getArduinoPort() {
    HDEVINFO deviceInfoSet = SetupDiGetClassDevs(&GUID_DEVCLASS_PORTS, NULL, NULL, DIGCF_PRESENT);
    if (deviceInfoSet == INVALID_HANDLE_VALUE) {
        return "";
    }

    SP_DEVINFO_DATA deviceInfoData;
    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    for (DWORD i = 0; SetupDiEnumDeviceInfo(deviceInfoSet, i, &deviceInfoData); i++) {
        char desc[256];
        DWORD descLen = 0;
        if (SetupDiGetDeviceRegistryProperty(deviceInfoSet, &deviceInfoData, SPDRP_FRIENDLYNAME, NULL, (BYTE*)desc, sizeof(desc), &descLen)) {
            std::string portName = "";
            if (strstr(desc, "Arduino") != NULL) {
                HKEY hkey = SetupDiOpenDevRegKey(deviceInfoSet, &deviceInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
                if (hkey != INVALID_HANDLE_VALUE) {
                    char port[256];
                    DWORD portLen = sizeof(port);
                    if (RegQueryValueEx(hkey, "PortName", NULL, NULL, (LPBYTE)port, &portLen) == ERROR_SUCCESS) {
                        portName = port;
                    }
                    RegCloseKey(hkey);
                }
            }
            if (!portName.empty()) {
                SetupDiDestroyDeviceInfoList(deviceInfoSet);
                return portName;
            }
        }
    }

    SetupDiDestroyDeviceInfoList(deviceInfoSet);
    return "";
}

#else

#include <vector>
#include <dirent.h>

std::string getArduinoPort() {
    std::vector<std::string> ports;
    DIR *dir;
    struct dirent *entry;

    dir = opendir("/dev/");
    if (dir == NULL) {
        return "";
    }

    while ((entry = readdir(dir)) != NULL) {
        std::string name = entry->d_name;
        if (name.find("ttyACM") != std::string::npos || name.find("ttyUSB") != std::string::npos) {
            ports.push_back("/dev/" + name);
        }
    }

    closedir(dir);

    for (const auto& port : ports) {
        Serial serial(port, 9600);
        if (serial.IsConnected()) {
            return port;
        }
    }

    return "";
}

#endif

const std::string DEVICE_NAME = "COM3"; // 串行端口名称或设备文件路径

int main() {
    // 连接到 Arduino
    ArduinoWrapper arduino;
    if (!arduino.connect(DEVICE_NAME)) {
        std::cout << "Failed to connect to device." << std::endl;
        return 1;
    }

    // 写入数字数据到 Arduino
    int value = 42;
    if (!arduino.writeData(value)) {
        std::cout << "Failed to write data to device." << std::endl;
        arduino.disconnect();
        return 1;
    }

    // 读取数据从 Arduino
    int result;
    if (!arduino.readData(result)) {
        std::cout << "Failed to read data from device." << std::endl;
        arduino.disconnect();
        return 1;
    }
    std::cout << "Received response: " << result << std::endl;

    // 断开连接
    arduino.disconnect();

    return 0;
}
