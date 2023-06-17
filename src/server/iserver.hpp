#pragma once

#include <map>
#include <string>
#include <vector>

class IndiDriver
{
public:
    IndiDriver(std::string const &binary, std::string const &skeleton,
               std::string const &label);

    std::string const &binary() const;
    std::string const &skeleton() const;
    std::string const &label() const;

private:
    std::string const binary_;
    std::string const skeleton_;
    std::string const label_;
};

class INDIServer
{
public:
    INDIServer(std::string const &fifo = "/tmp/indiFIFO",
               std::string const &conf_dir = "/home/" + std::getenv("USER") + "/.indi");

    void start(int port = 7624, std::vector<IndiDriver> const &drivers = {});
    void stop();
    bool is_running() const;
    void set_prop(std::string const &dev, std::string const &prop,
                  std::string const &element, std::string const &value) const;
    std::string get_prop(std::string const &dev, std::string const &prop,
                         std::string const &element) const;
    std::string get_state(std::string const &dev, std::string const &prop) const;
    void auto_connect() const;
    std::map<std::string, IndiDriver> get_running_drivers() const;

private:
    void clear_fifo();
    void run(int port);
    void start_driver(IndiDriver const &driver);
    std::vector<pid_t> get_indi_pids() const;

    std::string const fifo_;
    std::string const conf_dir_;
    std::map<std::string, IndiDriver> running_drivers_;
};

/*
#include "indiserver.h"

#include <iostream>

int main() {
    IndiDriver driver("/path/to/driver/binary", "/path/to/driver/skeleton", "Driver label");
    INDIServer server;
    server.start(7624, {driver});
    server.set_prop("DEVICE_NAME", "PROPERTY_NAME", "ELEMENT_NAME", "VALUE");
    std::cout << server.get_prop("DEVICE_NAME", "PROPERTY_NAME", "ELEMENT_NAME") << std::endl;
    std::cout << server.get_state("DEVICE_NAME", "PROPERTY_NAME") << std::endl;
    server.auto_connect();
    server.stop();
    return 0;
}
*/