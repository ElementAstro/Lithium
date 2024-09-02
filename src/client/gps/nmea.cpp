#include "nmea.hpp"

#include <asio.hpp>
#include <iostream>
#include <regex>
#include <stdexcept>
#include <thread>

class NMEAGps::Impl {
public:
    Impl() : connected_(false), baudRate_(0) {}

    ~Impl() { disconnect(); }

    void initialize() {
        if (connected_) {
            disconnect();
        }
        connected_ = false;
        baudRate_ = 0;
        portName_.clear();
    }

    std::future<std::tuple<double, double, double>> getLocation() {
        initialize();
        autoDiscover();

        locationPromise_ = std::promise<std::tuple<double, double, double>>();
        auto locationFuture = locationPromise_.get_future();

        asio::streambuf buffer;
        asio::async_read_until(
            *serialPort_, buffer, '\n',
            [this, &buffer](std::error_code ec, std::size_t length) {
                if (!ec) {
                    std::istream is(&buffer);
                    std::string message;
                    std::getline(is, message);
                    onMessageReceived(message);
                } else {
                    locationPromise_.set_exception(std::make_exception_ptr(
                        std::runtime_error("Error receiving GPS message")));
                }
            });

        std::thread([this]() { ioContext_.run(); }).detach();

        if (locationFuture.wait_for(std::chrono::seconds(sentenceWait_)) ==
            std::future_status::timeout) {
            disconnect();
            throw std::runtime_error(
                "No GPS fix obtained within the specified time");
        }

        return locationFuture;
    }

    void disconnect() {
        if (serialPort_ && serialPort_->is_open()) {
            serialPort_->close();
        }
        connected_ = false;
    }

private:
    void autoDiscover() {
        std::vector<std::string> ports = {
            /* List of COM ports for your platform */};
        std::vector<int> baudRates = {4800,  2400,  9600,  19200,
                                      38400, 57600, 115200};

        for (const auto& port : ports) {
            for (const auto& baud : baudRates) {
                try {
                    serialPort_ =
                        std::make_unique<asio::serial_port>(ioContext_, port);
                    serialPort_->set_option(
                        asio::serial_port_base::baud_rate(baud));
                    portName_ = port;
                    baudRate_ = baud;
                    connected_ = true;
                    return;
                } catch (const std::system_error&) {
                    // Continue to the next port or baud rate
                }
            }
        }
        throw std::runtime_error(
            "GNSS device not found on any accessible COM port");
    }

    void onMessageReceived(const std::string& message) {
        if (isValidGga(message)) {
            double latitude = 0.0;   // Parse from message
            double longitude = 0.0;  // Parse from message
            double altitude = 0.0;   // Parse from message

            locationPromise_.set_value(
                std::make_tuple(latitude, longitude, altitude));
            disconnect();
        }
    }

    bool isValidGga(const std::string& message) {
        return std::regex_search(message, ggaRegex_);
    }

    asio::io_context ioContext_;
    std::unique_ptr<asio::serial_port> serialPort_;
    std::string portName_;
    int baudRate_;
    std::promise<std::tuple<double, double, double>> locationPromise_;
    bool connected_;
    static constexpr int sentenceWait_ = 4;
    std::regex ggaRegex_{R"(^[$!](G.)GGA)"};
};

NMEAGps::NMEAGps() : pimpl_(std::make_unique<Impl>()) {}

NMEAGps::~NMEAGps() = default;

void NMEAGps::initialize() { pimpl_->initialize(); }

std::future<std::tuple<double, double, double>> NMEAGps::getLocation() {
    return pimpl_->getLocation();
}

void NMEAGps::disconnect() { pimpl_->disconnect(); }
