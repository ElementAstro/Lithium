#include "skyx.hpp"

#include <asio.hpp>
#include <future>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <stdexcept>

using asio::ip::tcp;
using json = nlohmann::json;

class TheSkyX::Impl {
public:
    Impl(const std::string& addr, int prt, bool useObj)
        : address_(addr), port_(prt), useSelectedObject_(useObj) {}

    std::string name() const { return "TheSkyX"; }

    bool canGetRotationAngle() const { return true; }

    std::future<json> getTarget() {
        return std::async(std::launch::async, [this]() {
            return useSelectedObject_ ? getSelectedObject()
                                      : getSkyChartCenter();
        });
    }

    std::future<json> getSite() {
        return std::async(std::launch::async,
                          [this]() { return queryLocation(); });
    }

    std::future<double> getRotationAngle() {
        return std::async(std::launch::async, [this]() -> double {
            if (useSelectedObject_)
                return std::numeric_limits<double>::quiet_NaN();
            return queryRotationAngle();
        });
    }

private:
    std::string address_;
    int port_;
    bool useSelectedObject_;

    json sendQuery(const std::string& script) {
        asio::io_context ioContext;
        tcp::resolver resolver(ioContext);
        tcp::socket socket(ioContext);

        auto endpoints = resolver.resolve(address_, std::to_string(port_));
        asio::connect(socket, endpoints);

        asio::write(socket, asio::buffer(script));

        asio::streambuf response;
        asio::read_until(socket, response, "|");

        std::istream responseStream(&response);
        std::string reply;
        std::getline(responseStream, reply);

        return json::parse(reply);
    }

    json getSelectedObject() {
        std::string script = R"(
            var Out = "";
            var Target56 = 0;
            var Target57 = 0;
            var Name0 = "";
            sky6ObjectInformation.Property(56);
            Target56 = sky6ObjectInformation.ObjInfoPropOut;
            sky6ObjectInformation.Property(57);
            Target57 = sky6ObjectInformation.ObjInfoPropOut;
            sky6ObjectInformation.Property(0);
            Name0 = sky6ObjectInformation.ObjInfoPropOut;
            Out = String(Target56) + "," + String(Target57) + "," + String(Name0);
        )";

        return sendQuery(script);
    }

    json getSkyChartCenter() {
        std::string script = R"(
            var Out = "";
            var chartRA = 0;
            var chartDec = 0;
            chartRA = sky6StarChart.RightAscension;
            chartDec = sky6StarChart.Declination;
            Out = String(chartRA) + "," + String(chartDec);
        )";

        return sendQuery(script);
    }

    json queryLocation() {
        std::string script = R"(
            var Out = "";
            var Lat = 0;
            var Long = 0;
            var Elevation = 0;
            sky6StarChart.DocumentProperty(0);
            Lat = sky6StarChart.DocPropOut;
            sky6StarChart.DocumentProperty(1);
            Long = sky6StarChart.DocPropOut;
            sky6StarChart.DocumentProperty(3);
            Elevation = sky6StarChart.DocPropOut;
            Out = String(Lat) + "," + String(Long) + "," + String(Elevation);
        )";

        return sendQuery(script);
    }

    double queryRotationAngle() {
        std::string script = R"(
            var angle = NaN;
            var fov = sky6MyFOVs;
            for (var i = 0; i < fov.Count; i++) {
                fov.Name(i);
                var name = fov.OutString;
                fov.Property(name, 0, 0);
                var isVisible = fov.OutVar;
                fov.Property(name, 0, 2);
                var refFrame = fov.OutVar;
                if (isVisible == 1 && refFrame == 0) {
                    fov.Property(name, 0, 1);
                    angle = fov.OutVar;
                    break;
                }
            }
            Out = String(angle);
        )";

        auto response = sendQuery(script);
        return response.is_number() ? response.get<double>()
                                    : std::numeric_limits<double>::quiet_NaN();
    }
};

// TheSkyX public interface implementation

TheSkyX::TheSkyX(const std::string& addr, int prt, bool useObj)
    : pimpl_(std::make_unique<Impl>(addr, prt, useObj)) {}

TheSkyX::~TheSkyX() = default;

std::string TheSkyX::name() const { return pimpl_->name(); }

bool TheSkyX::canGetRotationAngle() const {
    return pimpl_->canGetRotationAngle();
}

std::future<json> TheSkyX::getTarget() { return pimpl_->getTarget(); }

std::future<json> TheSkyX::getSite() { return pimpl_->getSite(); }

std::future<double> TheSkyX::getRotationAngle() {
    return pimpl_->getRotationAngle();
}
