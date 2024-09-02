#pragma once

#include <future>
#include <memory>
#include <string>
#include <vector>

class HNSKY {
public:
    HNSKY(const std::string& address, int port);
    ~HNSKY();

    std::string getName() const;
    bool canGetRotationAngle() const;

    struct Coordinates {
        double ra;
        double dec;
    };

    struct DeepSkyObject {
        std::string name;
        Coordinates coordinates;
    };

    struct Location {
        double latitude;
        double longitude;
        double elevation;
    };

    std::future<DeepSkyObject> getTarget();
    std::future<Location> getSite();
    std::future<double> getRotationAngle();

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};
