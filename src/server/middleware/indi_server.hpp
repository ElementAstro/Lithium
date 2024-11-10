#ifndef LITHIUM_SERVER_MIDDLEWARE_INDI_SERVER_HPP
#define LITHIUM_SERVER_MIDDLEWARE_INDI_SERVER_HPP

#include <string>

#include "device/basic.hpp"

namespace lithium::middleware {
auto indiDriverConfirm(const std::string& driverName) -> bool;
void indiDeviceConfirm(const std::string& deviceName,
                       const std::string& driverName);
void printDevGroups2(const device::DriversList& driversList, int ListNum,
                     const std::string& group);
void indiCapture(int expTime);
void indiAbortCapture();
auto setFocusSpeed(int speed) -> int;
auto focusMoveAndCalHFR(bool isInward, int steps) -> double;
void autofocus();
void showAllImageFolder();

void deviceConnect();
void getQTClientVersion();
}  // namespace lithium::middleware

#endif
