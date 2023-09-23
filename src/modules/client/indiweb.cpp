#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <map>
#include <fstream>
#include <pugixml/pugixml.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
namespace fs = std::filesystem;

class INDIDeviceContainer
{
public:
    std::string name;
    std::string label;
    std::string version;
    std::string binary;
    std::string family;
    std::string skeleton;
    bool custom;

    INDIDeviceContainer(const std::string &name, const std::string &label, const std::string &version,
                        const std::string &binary, const std::string &family,
                        const std::string &skeleton = "", bool custom = false)
        : name(name), label(label), version(version), binary(binary),
          family(family), skeleton(skeleton), custom(custom) {}
};

class INDIDriverCollection
{
private:
    std::string path;
    std::vector<std::string> files;
    std::vector<INDIDeviceContainer> drivers;

public:
    INDIDriverCollection(const std::string &path = "/usr/share/indi") : path(path)
    {
        parseDrivers();
    }

    void parseDrivers()
    {
        for (const auto &entry : fs::directory_iterator(path))
        {
            const std::string &fname = entry.path().filename().string();
            if (fname.ends_with(".xml") && fname.find("_sk") == std::string::npos)
            {
                files.push_back(entry.path());
            }
        }

        for (const std::string &fname : files)
        {
            pugi::xml_document doc;
            if (!doc.load_file(fname.c_str()))
            {
                std::cerr << "Error loading file " << fname << std::endl;
                continue;
            }

            pugi::xml_node root = doc.child("root");
            for (pugi::xml_node group = root.child("devGroup"); group; group = group.next_sibling("devGroup"))
            {
                const std::string &family = group.attribute("group").as_string();
                for (pugi::xml_node device = group.child("device"); device; device = device.next_sibling("device"))
                {
                    const std::string &label = device.attribute("label").as_string();
                    const std::string &skel = device.attribute("skel").as_string();
                    const std::string &name = device.child("driver").attribute("name").as_string();
                    const std::string &binary = device.child("driver").text().as_string();
                    const std::string &version = device.child("version").text().as_string("0.0");

                    INDIDeviceContainer driver(name, label, version, binary, family, skel);
                    drivers.push_back(driver);
                }
            }
        }

        // Sort drivers by label
        std::sort(drivers.begin(), drivers.end(), [](const INDIDeviceContainer &a, const INDIDeviceContainer &b)
                  { return a.label < b.label; });
    }

    void parseCustomDrivers(const json &drivers)
    {
        for (const auto &custom : drivers)
        {
            const std::string &name = custom["name"].get<std::string>();
            const std::string &label = custom["label"].get<std::string>();
            const std::string &version = custom["version"].get<std::string>();
            const std::string &binary = custom["exec"].get<std::string>();
            const std::string &family = custom["family"].get<std::string>();

            INDIDeviceContainer driver(name, label, version, binary, family, "", true);
            drivers.push_back(driver);
        }
    }

    void clearCustomDrivers()
    {
        drivers.erase(std::remove_if(drivers.begin(), drivers.end(), [](const INDIDeviceContainer &driver)
                                     { return driver.custom == true; }),
                      drivers.end());
    }

    INDIDeviceContainer *getByLabel(const std::string &label)
    {
        for (auto &driver : drivers)
        {
            if (driver.label == label)
            {
                return &driver;
            }
        }
        return nullptr;
    }

    INDIDeviceContainer *getByName(const std::string &name)
    {
        for (auto &driver : drivers)
        {
            if (driver.name == name)
            {
                return &driver;
            }
        }
        return nullptr;
    }

    INDIDeviceContainer *getByBinary(const std::string &binary)
    {
        for (auto &driver : drivers)
        {
            if (driver.binary == binary)
            {
                return &driver;
            }
        }
        return nullptr;
    }

    std::map<std::string, std::vector<std::string>> getFamilies()
    {
        std::map<std::string, std::vector<std::string>> families;
        for (const auto &driver : drivers)
        {
            families[driver.family].push_back(driver.label);
        }
        return families;
    }
};

class INDIManager
{

private:
    std::string host;
    int port;
    std::string config_path;
    std::string data_path;
    std::string fifo_path;
    std::map<std::string, INDIDeviceContainer> running_drivers;

public:
    INDIManager(std::string hst = "localhost", int prt = 7624, std::string cfg = "", std::string dta = "/usr/share/indi", std::string fif = "/tmp/indiFIFO")
    {
        host = hst;
        port = prt;
        config_path = cfg;
        data_path = dta;
        fifo_path = fif;
    }

    void start_server()
    {
        // If there is a INDI server running, just kill it
        if (is_running())
        {
            stop_server();
        }
        // Clear old fifo pipe and create new one
        std::cout << "Deleting fifo pipe at: " << fifo_path << std::endl;
        system(("rm -f " + fifo_path).c_str());
        system(("mkfifo " + fifo_path).c_str());
        // Just start the server without driver
        std::string cmd = "indiserver -p " + std::to_string(port) + " -m 100 -v -f " + fifo_path + " > /tmp/indiserver.log 2>&1 &";
        std::cout << cmd << std::endl;
        std::cout << "Started INDI server on port " << port << std::endl;
        system(cmd.c_str());
    }

    void stop_server()
    {
        std::string cmd = "killall indiserver >/dev/null 2>&1";
        int res = system(cmd.c_str());
        if (res == 0)
        {
            std::cout << "INDI server terminated successfully" << std::endl;
        }
        else
        {
            std::cerr << "Failed to terminate indiserver, error code is " << res << std::endl;
        }
    }

    bool is_running()
    {
        std::string output = "";
        FILE *pipe = popen(("ps -ef | grep indiserver | grep -v grep | wc -l").c_str(), "r");
        if (!pipe)
            return false;
        char buffer[128];
        while (!feof(pipe))
        {
            if (fgets(buffer, 128, pipe) != NULL)
                output += buffer;
        }
        pclose(pipe);
        return (output != "0");
    }

    void start_driver(INDIDeviceContainer driver)
    {
        std::string cmd = "start " + driver.binary;

        if (driver.skeleton != "")
        {
            cmd += " -s \"" + driver.skeleton + "\"";
        }

        cmd = std::regex_replace(cmd, std::regex("\""), "\\\"");
        std::string full_cmd = "echo \"" + cmd + "\" > " + fifo_path;
        std::cout << full_cmd << std::endl;
        system(full_cmd.c_str());
        std::cout << "Started driver : " << driver.name << std::endl;

        running_drivers.emplace(driver.label, driver);
    }

    void stop_driver(INDIDeviceContainer driver)
    {
        std::string cmd = "stop " + driver.binary;

        if (driver.binary.find("@") == std::string::npos)
        {
            cmd += " -n \"" + driver.label + "\"";
        }

        cmd = std::regex_replace(cmd, std::regex("\""), "\\\"");
        std::string full_cmd = "echo \"" + cmd + "\" > " + fifo_path;
        std::cout << full_cmd << std::endl;
        system(full_cmd.c_str());
        std::cout << "Stop running driver : " << driver.label << std::endl;

        running_drivers.erase(driver.label);
    }

    void set_prop(std::string dev, std::string prop, std::string element, std::string value)
    {
        std::stringstream ss;
        ss << "indi_setprop " << dev << "." << prop << "." << element << "=" << value;
        std::string cmd = ss.str();
        system(cmd.c_str());
    }

    std::string get_prop(std::string dev, std::string prop, std::string element)
    {
        std::stringstream ss;
        ss << "indi_getprop " << dev << "." << prop << "." << element;
        std::string cmd = ss.str();
        std::array<char, 128> buffer;
        std::string result = "";
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
        if (!pipe)
        {
            throw std::runtime_error("popen() failed!");
        }
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
        {
            result += buffer.data();
        }
        return result.substr(result.find('=') + 1, result.length()).substr(0, result.length() - 2);
    }

    std::string get_state(std::string dev, std::string prop)
    {
        return get_prop(dev, prop, "_STATE");
    }

    std::map<std::string, INDIDeviceContainer> get_running_drivers()
    {
        return running_drivers;
    }

    static std::vector<std::map<std::string, std::string>> get_devices()
    {
        std::vector<std::map<std::string, std::string>> devices;
        std::string cmd = "indi_getprop *.CONNECTION.CONNECT";
        std::array<char, 128> buffer;
        std::string result = "";
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
        if (!pipe)
        {
            throw std::runtime_error("popen() failed!");
        }
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
        {
            result += buffer.data();
        }
        std::vector<std::string> lines = {"", ""};
        for (std::string token : result)
        {
            if (token == '\n')
            {
                std::map<std::string, std::string> device;
                std::stringstream ss(lines[0]);
                std::string item;
                while (getline(ss, item, '.'))
                {
                    device["device"] = item;
                }
                device["connected"] = (lines[1] == "On") ? "true" : "false";
                devices.push_back(device);
                lines = {"", ""};
            }
            else if (token == '=')
            {
                lines[1] = lines[1].substr(0, lines[1].length() - 1);
            }
            else if (token == ' ')
            {
                continue;
            }
            else
            {
                lines[(lines[0] == "") ? 0 : 1] += token;
            }
        }
        return devices;
    }
};

int main()
{

    std::string path = "/usr/share/indi";
    INDIDriverCollection collection(path);

    collection.getByName()

    // Access the drivers collection as needed
    INDIDeviceContainer *driver1 = collection.getByLabel("Label1");
    if (driver1)
    {
        std::cout << "Driver Name: " << driver1->name << std::endl;
        std::cout << "Driver Binary: " << driver1->binary << std::endl;
        // Other properties...
    }

    // Access families
    std::map<std::string, std::vector<std::string>> families = collection.getFamilies();
    for (const auto &family : families)
    {
        std::cout << "Family: " << family.first << std::endl;
        for (const std::string &label : family.second)
        {
            std::cout << " - " << label << std::endl;
        }
    }
    INDIManager manager;

    // 启动INDI服务器
    manager.start_server();

    // 获取已连接的设备
    std::vector<std::map<std::string, std::string>> devices = INDIManager::get_devices();
    for (auto device : devices)
    {
        std::cout << "Found device: " << device["device"] << ", connected: " << device["connected"] << std::endl;
    }

    // 获取Autoguider的状态
    std::string state = manager.get_state("Autoguider", "CONNECTION");
    std::cout << "Autoguider state: " << state << std::endl;

    // 设置Autoguider的望远镜方向为赤道
    manager.set_prop("Autoguider", "TELESCOPE_EQUATORIAL_EOD_COORD", "EQUATORIAL_EOD_COORD", "On");

    // 停止INDI服务器
    manager.stop_server();

    return 0;
}