#include "gpio.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <utility>

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"

#define GPIO_EXPORT "/sys/class/gpio/export"
#define GPIO_PATH "/sys/class/gpio"

namespace atom::system {
class GPIO::Impl {
public:
    explicit Impl(std::string pin) : pin_(std::move(pin)) {
        exportGPIO();
        setGPIODirection("out");
    }

    ~Impl() {
        try {
            setGPIODirection("in");
        } catch (...) {
            // Suppress all exceptions
        }
    }

    void setValue(bool value) { setGPIOValue(value ? "1" : "0"); }

    bool getValue() { return readGPIOValue(); }

    void setDirection(const std::string& direction) {
        setGPIODirection(direction);
    }

    static void notifyOnChange(const std::string& pin,
                               const std::function<void(bool)>& callback) {
        std::thread([pin, callback]() {
            std::string path =
                std::string(GPIO_PATH) + "/gpio" + pin + "/value";
            int fd = open(path.c_str(), O_RDONLY);
            if (fd < 0) {
                LOG_F(ERROR, "Failed to open gpio value for reading");
                return;
            }

            char lastValue = '0';
            while (true) {
                char value[3] = {0};
                if (read(fd, value, sizeof(value) - 1) > 0) {
                    if (value[0] != lastValue) {
                        lastValue = value[0];
                        callback(value[0] == '1');
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                lseek(fd, 0, SEEK_SET);
            }
            close(fd);
        }).detach();
    }

private:
    std::string pin_;

    void exportGPIO() { executeGPIOCommand(GPIO_EXPORT, pin_); }

    void setGPIODirection(const std::string& direction) {
        std::string path =
            std::string(GPIO_PATH) + "/gpio" + pin_ + "/direction";
        executeGPIOCommand(path, direction);
    }

    void setGPIOValue(const std::string& value) {
        std::string path = std::string(GPIO_PATH) + "/gpio" + pin_ + "/value";
        executeGPIOCommand(path, value);
    }

    auto readGPIOValue() -> bool {
        std::string path = std::string(GPIO_PATH) + "/gpio" + pin_ + "/value";
        char value[3] = {0};
        int fd = open(path.c_str(), O_RDONLY);
        if (fd < 0) {
            THROW_RUNTIME_ERROR("Failed to open gpio value for reading");
        }
        ssize_t bytes = read(fd, value, sizeof(value) - 1);
        close(fd);
        if (bytes < 0) {
            THROW_RUNTIME_ERROR("Failed to read gpio value");
        }
        return value[0] == '1';
    }

    static void executeGPIOCommand(const std::string& path,
                            const std::string& command) {
        int fd = open(path.c_str(), O_WRONLY);
        if (fd < 0) {
            THROW_RUNTIME_ERROR("Failed to open gpio path: " + path);
        }
        ssize_t bytes = write(fd, command.c_str(), command.length());
        close(fd);
        if (bytes != static_cast<ssize_t>(command.length())) {
            THROW_RUNTIME_ERROR("Failed to write to gpio path: " + path);
        }
    }
};

GPIO::GPIO(const std::string& pin) : impl_(std::make_unique<Impl>(pin)) {}

GPIO::~GPIO() = default;

void GPIO::setValue(bool value) { impl_->setValue(value); }

bool GPIO::getValue() { return impl_->getValue(); }

void GPIO::setDirection(const std::string& direction) {
    impl_->setDirection(direction);
}

void GPIO::notifyOnChange(const std::string& pin,
                          std::function<void(bool)> callback) {
    Impl::notifyOnChange(pin, std::move(callback));
}
}  // namespace atom::system
