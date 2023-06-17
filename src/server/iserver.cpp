#include "iserver.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

#include <dirent.h>

IndiDriver::IndiDriver(std::string const &binary, std::string const &skeleton,
                       std::string const &label)
    : binary_(binary), skeleton_(skeleton), label_(label) {}

std::string const &IndiDriver::binary() const { return binary_; }

std::string const &IndiDriver::skeleton() const { return skeleton_; }

std::string const &IndiDriver::label() const { return label_; }

INDIServer::INDIServer(std::string const &fifo, std::string const &conf_dir)
    : fifo_(fifo), conf_dir_(conf_dir)
{
    stop();
}

void INDIServer::start(int port, std::vector<IndiDriver> const &drivers)
{
    if (is_running())
    {
        stop();
    }

    clear_fifo();
    run(port);
    running_drivers_.clear();

    for (auto const &driver : drivers)
    {
        start_driver(driver);
    }
}

void INDIServer::stop()
{
    std::vector<std::string> cmd = {"pkill", "-9", "indiserver"};
    int ret = std::system(fmt::format("{}", fmt::join(cmd, " ").c_str()).c_str());
    if (ret == 0)
    {
        spdlog::info("indiserver terminated successfully");
    }
    else
    {
        spdlog::warn("terminating indiserver failed code {}", ret);
    }
}

bool INDIServer::is_running() const
{
    for (auto pid : get_indi_pids())
    {
        return true;
    }
    return false;
}

void INDIServer::set_prop(std::string const &dev, std::string const &prop,
                          std::string const &element, std::string const &value) const
{
    std::vector<std::string> cmd = {"indi_setprop",
                                    dev + "." + prop + "." + element + "=" + value};
    std::system(fmt::format("{}", fmt::join(cmd, " ").c_str()).c_str());
}

std::string INDIServer::get_prop(std::string const &dev, std::string const &prop,
                                 std::string const &element) const
{
    std::vector<std::string> cmd = {"indi_getprop",
                                    dev + "." + prop + "." + element};
    std::ostringstream output;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(
                                                      fmt::format("{}", fmt::join(cmd, " ").c_str()).c_str(), "r"),
                                                  pclose);
    if (!pipe)
    {
        spdlog::error("popen() failed!");
    }

    char buffer[128];
    while (fgets(buffer, 128, pipe.get()) != nullptr)
    {
        output << buffer;
    }

    return output.str().substr(output.str().find("=") + 1,
                               output.str().size() - output.str().find("="));
}

std::string INDIServer::get_state(std::string const &dev,
                                  std::string const &prop) const
{
    return get_prop(dev, prop, "_STATE");
}

void INDIServer::auto_connect() const
{
    std::vector<std::string> cmd = {"indi_getprop", "*.CONNECTION.CONNECT"};

    std::ostringstream output;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(
                                                      fmt::format("{}", fmt::join(cmd, " ").c_str()).c_str(), "r"),
                                                  pclose);
    if (!pipe)
    {
        spdlog::error("popen() failed!");
    }

    std::string line;
    while (std::getline(pipe.get(), line))
    {
        if (line == "Off")
        {
            continue;
        }
        std::vector<std::string> command = {"indi_setprop", line};
        std::system(fmt::format("{}", fmt::join(command, " ").c_str()).c_str());
    }
}

std::map<std::string, IndiDriver> INDIServer::get_running_drivers() const
{
    std::map<std::string, IndiDriver> drivers;
    for (auto const &driver : running_drivers_)
    {
        drivers.emplace(driver.first, driver.second);
    }
    return drivers;
}

void INDIServer::clear_fifo()
{
    spdlog::info("Deleting fifo {}", fifo_);
    std::vector<std::string> cmd = {"rm", "-f", fifo_};
    std::system(fmt::format("{}", fmt::join(cmd, " ").c_str()).c_str());
    cmd = {"mkfifo", fifo_};
    std::system(fmt::format("{}", fmt::join(cmd, " ").c_str()).c_str());
}

void INDIServer::run(int port)
{
    std::stringstream cmd_ss;
    cmd_ss << "indiserver -p " << port << " -m 1000 -v -f " << fifo_
           << " > /tmp/indiserver.log 2>&1 &";
    std::string const cmd = cmd_ss.str();
    std::system(fmt::format("{}", cmd.c_str()).c_str());
}

void INDIServer::start_driver(IndiDriver const &driver)
{
    std::stringstream cmd_ss;
    cmd_ss << "start " << driver.binary();

    if (!driver.skeleton().empty())
    {
        cmd_ss << " -s \"" << driver.skeleton() << "\"";
    }

    cmd_ss << " -n \"" << driver.label() << "\"";
    std::string const cmd = cmd_ss.str();
    std::vector<std::string> full_cmd = {"echo", cmd,
                                         ">", fifo_};
    std::system(fmt::format("{}", fmt::join(full_cmd, " ").c_str()).c_str());
    running_drivers_[driver.label()] = driver;
}

std::vector<pid_t> INDIServer::get_indi_pids() const
{
    std::vector<pid_t> pids;
    DIR *dir = opendir("/proc");
    if (dir == nullptr)
    {
        return pids;
    }
    dirent *entry = nullptr;
    while ((entry = readdir(dir)) != nullptr)
    {
        if (entry->d_type != DT_DIR)
        {
            continue;
        }
        char *endptr = nullptr;
        long const pid = strtol(entry->d_name, &endptr, 10);
        if (*endptr != '\0')
        {
            continue;
        }
        std::string const comm_path = "/proc/" + std::to_string(pid) + "/comm";
        std::ifstream comm_file(comm_path);
        std::string comm;
        std::getline(comm_file, comm);
        if (comm == "indiserver")
        {
            pids.push_back(pid);
        }
    }
    closedir(dir);
    return pids;
}
