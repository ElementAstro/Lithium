#include "app.hpp"

#include "command.hpp"
#include "token.hpp"

#include "atom/async/message_bus.hpp"
#include "atom/error/exception.hpp"
#include "atom/function/global_ptr.hpp"
#include "atom/io/io.hpp"

#include "atom/type/json.hpp"
using json = nlohmann::json;

namespace lithium {
class LithiumAppImpl {
public:
    std::weak_ptr<atom::async::MessageBus> bus_;

    // Max:: This config is for the core component.
    json config_;
};

LithiumApp::LithiumApp() {
    if (!atom::io::isFileExists("config/base.json")) {
        LOG_F(WARNING, "Failed to find config/base.json");
    }
    GetWeakPtr<atom::async::MessageBus>("");
}
}  // namespace lithium
