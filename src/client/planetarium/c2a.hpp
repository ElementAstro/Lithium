#pragma once

#include <memory>
#include <string>
#include <vector>

struct Coordinates {
    double ra;
    double dec;

    Coordinates(double ra, double dec);
};

struct DeepSkyObject {
    std::string name;
    Coordinates coords;

    DeepSkyObject(const std::string& name, const Coordinates& coords);
};

struct Location {
    double latitude;
    double longitude;
    double elevation;

    Location(double lat, double lon, double elev);
};

class PlanetariumException : public std::exception {
public:
    explicit PlanetariumException(const std::string& msg);

    const char* what() const noexcept override;

private:
    std::string message;
};

class C2A {
public:
    C2A(const std::string& addr, int port);
    ~C2A();

    DeepSkyObject getTarget();
    Location getSite();

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};
