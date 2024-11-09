#ifndef ATOM_SYSTEM_GPIO_HPP
#define ATOM_SYSTEM_GPIO_HPP

#include <functional>
#include <memory>
#include <string>

namespace atom::system {
class GPIO {
public:
    GPIO(const std::string& pin);
    ~GPIO();

    void setValue(bool value);
    bool getValue();
    void setDirection(const std::string& direction);
    static void notifyOnChange(const std::string& pin,
                               std::function<void(bool)> callback);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};
}  // namespace atom::system

#endif  // ATOM_SYSTEM_GPIO_HPP