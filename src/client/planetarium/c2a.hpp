#pragma once

#include <exception>
#include <memory>
#include <string>

struct alignas(16) Coordinates {
private:
    double rightAscension;
    double declination;

public:
    Coordinates(double rightAscension, double declination);

    auto getRightAscension() const -> double { return rightAscension; }
    auto getDeclination() const -> double { return declination; }
};

struct alignas(64) DeepSkyObject {
private:
    std::string name;
    Coordinates coordinates;

public:
    DeepSkyObject(const std::string& name, const Coordinates& coordinates);

    auto getName() const -> const std::string& { return name; }
    auto getCoordinates() const -> const Coordinates& { return coordinates; }
};

struct alignas(32) Location {
private:
    double latitude;
    double longitude;
    double elevation;

public:
    Location(double latitude, double longitude, double elevation);

    auto getLatitude() const -> double { return latitude; }
    auto getLongitude() const -> double { return longitude; }
    auto getElevation() const -> double { return elevation; }
};

class PlanetariumException : public std::exception {
public:
    explicit PlanetariumException(const std::string& msg);

    [[nodiscard]] auto what() const noexcept -> const char* override;

private:
    std::string message_;
};

class C2A {
public:
    C2A(const std::string& addr, int port);
    ~C2A();

    auto getTarget() -> DeepSkyObject;
    auto getSite() -> Location;

    // Disable copy and move constructors and assignment operators
    C2A(const C2A&) = delete;
    C2A& operator=(const C2A&) = delete;
    C2A(C2A&&) = delete;
    C2A& operator=(C2A&&) = delete;

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};
