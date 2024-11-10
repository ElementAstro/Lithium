#include "gpio.hpp"

#include "atom/async/message_bus.hpp"
#include "atom/function/global_ptr.hpp"
#include "atom/log/loguru.hpp"
#include "atom/system/gpio.hpp"
#include "atom/utils/print.hpp"
#include "utils/constant.hpp"

#define GPIO_PIN_1 "516"
#define GPIO_PIN_2 "527"

namespace lithium::middleware {
void getGPIOsStatus() {
    LOG_F(INFO, "getGPIOsStatus: Entering function");

    std::shared_ptr<atom::async::MessageBus> messageBusPtr;
    GET_OR_CREATE_PTR(messageBusPtr, atom::async::MessageBus,
                      Constants::MESSAGE_BUS)

    const std::vector<std::pair<int, std::string>> gpioPins = {{1, GPIO_PIN_1},
                                                               {2, GPIO_PIN_2}};

    for (const auto& [id, pin] : gpioPins) {
        LOG_F(INFO, "getGPIOsStatus: Processing GPIO pin: {} with ID: %d",
              pin.c_str(), id);
        atom::system::GPIO gpio(pin);
        int value = static_cast<int>(gpio.getValue());
        LOG_F(INFO, "getGPIOsStatus: GPIO pin: {} has value: %d", pin.c_str(),
              value);
        messageBusPtr->publish("main",
                               "OutPutPowerStatus:{}:{}"_fmt(id, value));
    }

    LOG_F(INFO, "getGPIOsStatus: Exiting function");
}

void switchOutPutPower(int id) {
    LOG_F(INFO, "switchOutPutPower: Entering function with ID: %d", id);

    std::shared_ptr<atom::async::MessageBus> messageBusPtr;
    GET_OR_CREATE_PTR(messageBusPtr, atom::async::MessageBus,
                      Constants::MESSAGE_BUS)

    const std::vector<std::pair<int, std::string>> gpioPins = {{1, GPIO_PIN_1},
                                                               {2, GPIO_PIN_2}};

    auto it = std::find_if(gpioPins.begin(), gpioPins.end(),
                           [id](const auto& pair) { return pair.first == id; });

    if (it != gpioPins.end()) {
        LOG_F(INFO, "switchOutPutPower: Found GPIO pin: {} for ID: %d",
              it->second.c_str(), id);
        atom::system::GPIO gpio(it->second);
        bool newValue = !gpio.getValue();
        LOG_F(INFO, "switchOutPutPower: Setting GPIO pin: {} to new value: %d",
              it->second.c_str(), newValue);
        gpio.setValue(newValue);
        messageBusPtr->publish("main",
                               "OutPutPowerStatus:{}:{}"_fmt(id, newValue));
    } else {
        LOG_F(WARNING, "switchOutPutPower: No GPIO pin found for ID: %d", id);
    }

    LOG_F(INFO, "switchOutPutPower: Exiting function");
}
}  // namespace lithium::middleware

#undef GPIO_PIN_1
#undef GPIO_PIN_2