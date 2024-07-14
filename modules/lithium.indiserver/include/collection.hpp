#ifndef LITHIUM_INDISERVER_COLLECTION_HPP
#define LITHIUM_INDISERVER_COLLECTION_HPP

#include "container.hpp"

#include <memory>
#include <unordered_map>
#include <vector>

#include "atom/type/json_fwd.hpp"
using json = nlohmann::json;

class INDIDriverCollection {
public:
    auto parseDrivers(const std::string& path) -> bool;
    auto parseCustomDrivers(const json& drivers) -> bool;
    void clearCustomDrivers();
    auto getByLabel(const std::string& label)
        -> std::shared_ptr<INDIDeviceContainer>;
    auto getByName(const std::string& name)
        -> std::shared_ptr<INDIDeviceContainer>;
    auto getByBinary(const std::string& binary)
        -> std::shared_ptr<INDIDeviceContainer>;
    auto getFamilies()
        -> std::unordered_map<std::string, std::vector<std::string>>;

private:
    std::string path_;
    std::vector<std::string> files_;
    std::vector<std::shared_ptr<INDIDeviceContainer>> drivers_;
};

#endif  // LITHIUM_INDISERVER_COLLECTION_HPP
