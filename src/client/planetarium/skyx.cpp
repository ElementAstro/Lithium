#include <iostream>
#include <string>
#include <stdexcept>
#include <system_error>
#include <sstream>
#include <vector>
#include <asio.hpp>
#include <future>
#include <nlohmann/json.hpp>

using asio::ip::tcp;
using json = nlohmann::json;

class TheSkyX {
private:
    std::string address;
    int port;
    bool useSelectedObject;

public:
    TheSkyX(const std::string& addr, int prt, bool useObj)
        : address(addr), port(prt), useSelectedObject(useObj) {}

    std::string Name() const {
        return "TheSkyX";
    }

    bool CanGetRotationAngle() const {
        return true;
    }

    std::future<json> GetTarget() {
        return std::async(std::launch::async, [this]() {
            if (useSelectedObject) {
                return GetSelectedObject();
            } else {
                return GetSkyChartCenter();
            }
        });
    }

    std::future<json> GetSite() {
        return std::async(std::launch::async, [this]() {
            return QueryLocation();
        });
    }

    std::future<double> GetRotationAngle() {
        return std::async(std::launch::async, [this]() -> double {
            if (useSelectedObject) return std::numeric_limits<double>::quiet_NaN();
            return QueryRotationAngle();
        });
    }

private:
    json SendQuery(const std::string& script) {
        asio::io_context io_context;
        tcp::resolver resolver(io_context);
        tcp::socket socket(io_context);

        auto endpoints = resolver.resolve(address, std::to_string(port));
        asio::connect(socket, endpoints);

        asio::write(socket, asio::buffer(script));

        asio::streambuf response;
        asio::read_until(socket, response, "|");

        std::istream response_stream(&response);
        std::string reply;
        std::getline(response_stream, reply);

        return json::parse(reply);
    }

    json GetSelectedObject() {
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

        return SendQuery(script);
    }

    json GetSkyChartCenter() {
        std::string script = R"(
            var Out = "";
            var chartRA = 0;
            var chartDec = 0;
            chartRA = sky6StarChart.RightAscension;
            chartDec = sky6StarChart.Declination;
            Out = String(chartRA) + "," + String(chartDec);
        )";

        return SendQuery(script);
    }

    json QueryLocation() {
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

        return SendQuery(script);
    }

    double QueryRotationAngle() {
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

        auto response = SendQuery(script);
        return response.is_number() ? response.get<double>() : std::numeric_limits<double>::quiet_NaN();
    }
};
