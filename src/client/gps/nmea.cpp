#include <boost/asio.hpp>
#include <fmt/core.h>
#include <regex>
#include <string>
#include <vector>
#include <iostream>
#include <future>
#include <chrono>
#include <thread>
#include <stdexcept>

class NMEAGps {
public:
    NMEAGps();
    ~NMEAGps();

    void initialize();
    std::future<std::tuple<double, double, double>> getLocation();
    void disconnect();

private:
    void autoDiscover();
    void onMessageReceived(const std::string& message);
    bool isValidGga(const std::string& message);

    boost::asio::io_context ioContext;
    std::unique_ptr<boost::asio::serial_port> serialPort;
    std::string portName;
    int baudRate;
    std::promise<std::tuple<double, double, double>> locationPromise;
    bool connected;
    static constexpr int sentenceWait = 4;
    std::regex ggaRegex{R"(^[$!](G.)GGA)"};
};

NMEAGps::NMEAGps() : connected(false), baudRate(0) {}

NMEAGps::~NMEAGps() {
    disconnect();
}


void NMEAGps::initialize() {
    if (connected) {
        disconnect();
    }
    connected = false;
    baudRate = 0;
    portName.clear();
}

void NMEAGps::autoDiscover() {
    std::vector<std::string> ports = {/* List of COM ports for your platform */};
    std::vector<int> baudRates = {4800, 2400, 9600, 19200, 38400, 57600, 115200};

    for (const auto& port : ports) {
        for (const auto& baud : baudRates) {
            try {
                serialPort = std::make_unique<boost::asio::serial_port>(ioContext, port);
                serialPort->set_option(boost::asio::serial_port_base::baud_rate(baud));
                // Perform some initial reads to check if this is a GPS device
                // If successful, set portName and baudRate
                portName = port;
                baudRate = baud;
                connected = true;
                return;
            } catch (const boost::system::system_error&) {
                // Continue to the next port or baud rate
            }
        }
    }
    throw std::runtime_error("GNSS device not found on any accessible COM port");
}

void NMEAGps::onMessageReceived(const std::string& message) {
    if (isValidGga(message)) {
        // Parse GGA sentence
        // Assuming message format is correct, extract latitude, longitude, altitude
        double latitude = 0.0; // Parse from message
        double longitude = 0.0; // Parse from message
        double altitude = 0.0; // Parse from message

        locationPromise.set_value(std::make_tuple(latitude, longitude, altitude));
        disconnect();
    }
}

bool NMEAGps::isValidGga(const std::string& message) {
    return std::regex_search(message, ggaRegex);
}

std::future<std::tuple<double, double, double>> NMEAGps::getLocation() {
    initialize();
    autoDiscover();

    locationPromise = std::promise<std::tuple<double, double, double>>();
    auto locationFuture = locationPromise.get_future();

    boost::asio::streambuf buffer;
    boost::asio::async_read_until(*serialPort, buffer, '\n',
        [this, &buffer](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                std::istream is(&buffer);
                std::string message;
                std::getline(is, message);
                onMessageReceived(message);
            } else {
                locationPromise.set_exception(std::make_exception_ptr(
                    std::runtime_error("Error receiving GPS message")));
            }
        });

    std::thread([this]() {
        ioContext.run();
    }).detach();

    if (locationFuture.wait_for(std::chrono::seconds(sentenceWait)) == std::future_status::timeout) {
        disconnect();
        throw std::runtime_error("No GPS fix obtained within the specified time");
    }

    return locationFuture;
}

void NMEAGps::disconnect() {
    if (serialPort && serialPort->is_open()) {
        serialPort->close();
    }
    connected = false;
}
