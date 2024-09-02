#pragma once

#include <future>
#include <string>
#include <nlohmann/json.hpp>

class TheSkyX {
public:
    TheSkyX(const std::string& addr, int prt, bool useObj);
    ~TheSkyX();

    std::string name() const;
    bool canGetRotationAngle() const;

    std::future<nlohmann::json> getTarget();
    std::future<nlohmann::json> getSite();
    std::future<double> getRotationAngle();

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};
