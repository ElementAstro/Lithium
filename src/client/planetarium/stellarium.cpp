#include "stellarium.hpp"

#include <asio.hpp>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <stdexcept>

using asio::ip::tcp;
using json = nlohmann::json;

class Stellarium::Impl {
public:
    Impl(const std::string& host, const std::string& port)
        : baseUrl_("http://" + host + ":" + port), context_() {}

    std::future<json> getSite() {
        return std::async(std::launch::async, [this]() {
            return fetchJson("/api/main/status")["location"];
        });
    }

    std::future<json> getTarget() {
        return std::async(std::launch::async, [this]() {
            return fetchJson("/api/objects/info?format=json");
        });
    }

    std::future<double> getRotationAngle() {
        return std::async(std::launch::async, [this]() {
            json response = fetchJson("/api/stelproperty/list?format=json");

            bool isOcularsCcdEnabled =
                response["Oculars.enableCCD"]["value"].get<bool>();
            if (!isOcularsCcdEnabled) {
                return std::numeric_limits<double>::quiet_NaN();
            }

            double angle = response["Oculars.selectedCCDRotationAngle"]["value"]
                               .get<double>();
            return 360.0 - angle;  // Reverse angle
        });
    }

private:
    std::string baseUrl_;
    asio::io_context context_;

    json fetchJson(const std::string& route) {
        std::string response = get(route);
        return json::parse(response);
    }

    std::string get(const std::string& route) {
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

        if (!responseStream || httpVersion.substr(0, 5) != "HTTP/") {
            throw std::runtime_error("Invalid response");
        }

        if (statusCode != 200) {
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

std::future<json> Stellarium::getSite() { return pimpl_->getSite(); }

std::future<json> Stellarium::getTarget() { return pimpl_->getTarget(); }

std::future<double> Stellarium::getRotationAngle() {
    return pimpl_->getRotationAngle();
}
