#ifndef LITHIUM_SERVER_MIDDLEWARE_GPIO_HPP
#define LITHIUM_SERVER_MIDDLEWARE_GPIO_HPP

namespace lithium::middleware {
void getGPIOsStatus();
void switchOutPutPower(int id);
}  // namespace lithium::middleware

#endif
