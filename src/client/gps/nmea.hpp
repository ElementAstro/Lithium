#pragma once

#include <asio.hpp>
#include <future>
#include <memory>
#include <tuple>

class NMEAGps {
public:
    NMEAGps();
    ~NMEAGps();

    void initialize();
    std::future<std::tuple<double, double, double>> getLocation();
    void disconnect();

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};
