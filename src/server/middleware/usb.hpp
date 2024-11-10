#ifndef LITHIUM_SERVER_MIDDLEWARE_USB_HPP
#define LITHIUM_SERVER_MIDDLEWARE_USB_HPP

#include <string>

namespace lithium::middleware {
void moveImageToUSB(const std::string& path);
void deleteFile(const std::string& path);
void usbCheck();
}  // namespace lithium::middleware

#endif  // LITHIUM_SERVER_MIDDLEWARE_USB_HPP