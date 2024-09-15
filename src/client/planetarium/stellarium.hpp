#pragma once

#include <asio.hpp>
#include <future>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>

class Stellarium {
public:
    Stellarium(const std::string& host, const std::string& port);
    ~Stellarium();

    std::future<nlohmann::json> getSite();
    std::future<nlohmann::json> getTarget();
    std::future<double> getRotationAngle();

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};