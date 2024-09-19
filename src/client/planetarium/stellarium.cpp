#include "stellarium.hpp"

#include <asio.hpp>
#include <iostream>
#include <sstream>
#include <stdexcept>

using asio::ip::tcp;
using json = nlohmann::json;

namespace {
constexpr double REVERSE_ANGLE_BASE = 360.0;
constexpr int HTTP_VERSION_PREFIX_LENGTH = 5;
constexpr int HTTP_STATUS_OK = 200;
}  // namespace

class Stellarium::Impl {
public:
    Impl(const std::string& host, const std::string& port)
        : baseUrl_("http://" + host + ":" + port) {}

    auto getSite() -> std::future<json> {
        return std::async(std::launch::async, [this]() {
            return fetchJson("/api/main/status")["location"];
        });
    }

    auto getTarget() -> std::future<json> {
        return std::async(std::launch::async, [this]() {
            return fetchJson("/api/objects/info?format=json");
        });
    }

    auto getRotationAngle() -> std::future<double> {
        return std::async(std::launch::async, [this]() -> double {
            json response = fetchJson("/api/stelproperty/list?format=json");

            bool isOcularsCcdEnabled =
                response["Oculars.enableCCD"]["value"].get<bool>();
            if (!isOcularsCcdEnabled) {
                return std::numeric_limits<double>::quiet_NaN();
            }

            double angle = response["Oculars.selectedCCDRotationAngle"]["value"]
                               .get<double>();
            return REVERSE_ANGLE_BASE - angle;  // Reverse angle
        });
    }

private:
    std::string baseUrl_;
    asio::io_context context_;

    auto fetchJson(const std::string& route) -> json {
        std::string response = get(route);
        return json::parse(response);
    }

    auto get(const std::string& route) -> std::string {
        tcp::resolver resolver(context_);
        tcp::resolver::results_type endpoints =
            resolver.resolve(baseUrl_, "http");

        tcp::socket socket(context_);
        asio::connect(socket, endpoints);

        std::string request = "GET " + route +
                              " HTTP/1.1\r\n"
                              "Host: " +
                              baseUrl_ +
                              "\r\n"
                              "Connection: close\r\n\r\n";

        asio::write(socket, asio::buffer(request));

        asio::streambuf response;
        asio::read_until(socket, response, "\r\n");

        std::istream responseStream(&response);
        std::string httpVersion;
        unsigned int statusCode;
        responseStream >> httpVersion >> statusCode;

        if (!responseStream ||
            httpVersion.substr(0, HTTP_VERSION_PREFIX_LENGTH) != "HTTP/") {
            throw std::runtime_error("Invalid response");
        }

        if (statusCode != HTTP_STATUS_OK) {
            throw std::runtime_error("Request failed with status code " +
                                     std::to_string(statusCode));
        }

        std::ostringstream responseData;
        responseData << &response;
        return responseData.str();
    }
};

Stellarium::Stellarium(const std::string& host, const std::string& port)
    : pimpl_(std::make_unique<Impl>(host, port)) {}

Stellarium::~Stellarium() = default;

auto Stellarium::getSite() -> std::future<json> { return pimpl_->getSite(); }

auto Stellarium::getTarget() -> std::future<json> {
    return pimpl_->getTarget();
}

auto Stellarium::getRotationAngle() -> std::future<double> {
    return pimpl_->getRotationAngle();
}