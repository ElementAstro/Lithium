#include "skyx.hpp"

#include <asio.hpp>
#include <future>
#include <iostream>

using asio::ip::tcp;
using json = nlohmann::json;

class TheSkyX::Impl {
public:
    Impl(std::string addr, int prt, bool useObj)
        : address_(std::move(addr)), port_(prt), useSelectedObject_(useObj) {}

    [[nodiscard]] static auto name() -> std::string { return "TheSkyX"; }

    [[nodiscard]] static auto canGetRotationAngle() -> bool { return true; }

    auto getTarget() -> std::future<json> {
        return std::async(std::launch::async, [this]() -> json {
            return useSelectedObject_ ? getSelectedObject()
                                      : getSkyChartCenter();
        });
    }

    auto getSite() -> std::future<json> {
        return std::async(std::launch::async,
                          [this]() -> json { return queryLocation(); });
    }

    auto getRotationAngle() -> std::future<double> {
        return std::async(std::launch::async, [this]() -> double {
            if (useSelectedObject_) {
                return std::numeric_limits<double>::quiet_NaN();
            }
            return queryRotationAngle();
        });
    }

private:
    std::string address_;
    int port_;
    bool useSelectedObject_;

    auto sendQuery(const std::string& script) -> json {
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

    auto getSelectedObject() -> json {
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

    auto getSkyChartCenter() -> json {
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

    auto queryLocation() -> json {
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

    auto queryRotationAngle() -> double {
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

auto TheSkyX::name() const -> std::string { return pimpl_->name(); }

auto TheSkyX::canGetRotationAngle() const -> bool {
    return pimpl_->canGetRotationAngle();
}

auto TheSkyX::getTarget() -> std::future<json> { return pimpl_->getTarget(); }

auto TheSkyX::getSite() -> std::future<json> { return pimpl_->getSite(); }

auto TheSkyX::getRotationAngle() -> std::future<double> {
    return pimpl_->getRotationAngle();
}
