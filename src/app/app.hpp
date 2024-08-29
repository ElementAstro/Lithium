#ifndef LITHIUM_APP_APP_HPP
#define LITHIUM_APP_APP_HPP

#include <memory>

namespace lithium {

class LithiumAppImpl;
class LithiumApp {
public:
    LithiumApp();

private:
    std::unique_ptr<LithiumAppImpl> impl_;
};

}  // namespace lithium

#endif