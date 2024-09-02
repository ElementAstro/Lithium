#include "phd2client.hpp"
#include "guider.hpp"

PHD2Client::PHD2Client(std::string name) : AtomGuider(name) {}

auto PHD2Client::initialize() -> bool { return true; }

auto PHD2Client::destroy() -> bool { return true; }

auto PHD2Client::connect(const std::string& name, int timeout,
                         int maxRetry) -> bool {
    return true;
}

auto PHD2Client::disconnect(bool force, int timeout, int maxRetry) -> bool {
    return true;
}

auto PHD2Client::reconnect(int timeout, int maxRetry) -> bool { return true; }

auto PHD2Client::isConnected() -> bool { return true; }
