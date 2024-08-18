#ifndef LITHIUM_CLIENT_PHD2_HPP
#define LITHIUM_CLIENT_PHD2_HPP

#include <mutex>
#include "device/template/guider.hpp"

class PHD2Client : public AtomGuider {
public:
    PHD2Client(std::string name);

    auto initialize() -> bool override;
    auto destroy() -> bool override;

    auto connect(const std::string& name, int timeout,
                 int maxRetry) -> bool override;

    auto disconnect(bool force, int timeout, int maxRetry) -> bool override;

    auto reconnect(int timeout, int maxRetry) -> bool override;

    auto scan() -> std::vector<std::string> override;

    auto isConnected() -> bool override;
private:
    std::unique_ptr<
};

#endif
