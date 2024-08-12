#include "driverlist.hpp"

#include <tinyxml2.h>
#include <filesystem>

#include "atom/log/loguru.hpp"

using namespace tinyxml2;
using namespace std::filesystem;

auto loadXMLFile(const std::string& filename, XMLDocument& doc) -> bool {
    if (doc.LoadFile(filename.c_str()) != XML_SUCCESS) {
        LOG_F(ERROR, "Unable to load XML file: {}", filename);
        return false;
    }
    return true;
}

auto parseDriversList(const std::string& filename) -> std::vector<DevGroup> {
    std::vector<DevGroup> devGroups;
    XMLDocument doc;

    if (!loadXMLFile(filename, doc)) {
        return devGroups;
    }

    XMLElement* root = doc.RootElement();
    for (XMLElement* devGroupElem = root->FirstChildElement("devGroup");
         devGroupElem != nullptr;
         devGroupElem = devGroupElem->NextSiblingElement("devGroup")) {
        DevGroup devGroup;
        devGroup.group = devGroupElem->Attribute("group");
        devGroups.push_back(devGroup);
    }

    return devGroups;
}

auto parseDevicesFromPath(const std::string& path,
                          std::vector<Device>& devicesFrom)
    -> std::vector<DevGroup> {
    std::vector<DevGroup> devGroups;

    for (const auto& entry : directory_iterator(path)) {
        if (entry.path().extension() == ".xml" &&
            entry.path().filename().string().substr(
                entry.path().filename().string().size() - 6) != "sk.xml") {
            XMLDocument doc;

            if (!loadXMLFile(entry.path().string(), doc)) {
                LOG_F(ERROR, "Unable to load XML file: {}", entry.path());
                continue;
            }

            XMLElement* root = doc.RootElement();
            for (XMLElement* devGroupElem = root->FirstChildElement("devGroup");
                 devGroupElem != nullptr;
                 devGroupElem = devGroupElem->NextSiblingElement("devGroup")) {
                DevGroup devGroup;
                devGroup.group = devGroupElem->Attribute("group");

                for (XMLElement* deviceElem =
                         devGroupElem->FirstChildElement("device");
                     deviceElem != nullptr;
                     deviceElem = deviceElem->NextSiblingElement("device")) {
                    Device device;
                    device.label = deviceElem->Attribute("label");

                    if (deviceElem->FindAttribute("manufacturer") != nullptr) {
                        device.manufacturer =
                            deviceElem->Attribute("manufacturer");
                    }

                    for (XMLElement* driverElem =
                             deviceElem->FirstChildElement();
                         driverElem != nullptr;
                         driverElem = driverElem->NextSiblingElement()) {
                        if (std::string(driverElem->Name()) == "driver") {
                            device.driverName = driverElem->GetText();
                        } else if (std::string(driverElem->Name()) ==
                                   "version") {
                            device.version = driverElem->GetText();
                        }
                    }
                    devGroup.devices.push_back(device);
                    devicesFrom.push_back(device);
                }
                devGroups.push_back(devGroup);
            }
        }
    }

    return devGroups;
}

auto mergeDeviceGroups(const DriversList& driversListFrom,
                       const std::vector<DevGroup>& devGroupsFromPath)
    -> DriversList {
    DriversList mergedList = driversListFrom;

    for (auto& devGroupXml : devGroupsFromPath) {
        for (auto& devGroupFrom : mergedList.devGroups) {
            if (devGroupXml.group == devGroupFrom.group) {
                devGroupFrom.devices.insert(devGroupFrom.devices.end(),
                                            devGroupXml.devices.begin(),
                                            devGroupXml.devices.end());
            }
        }
    }

    return mergedList;
}

auto readDriversListFromFiles(std::string_view filename, std::string_view path)
    -> std::tuple<DriversList, std::vector<DevGroup>, std::vector<Device>> {
    DriversList driversListFrom;
    std::vector<DevGroup> devGroupsFrom;
    std::vector<Device> devicesFrom;

    if (!exists(path)) {
        LOG_F(ERROR, "Folder not found: {}", path);
        return {driversListFrom, devGroupsFrom, devicesFrom};
    }

    driversListFrom.devGroups = parseDriversList(filename.data());
    devGroupsFrom = parseDevicesFromPath(path.data(), devicesFrom);
    driversListFrom = mergeDeviceGroups(driversListFrom, devGroupsFrom);

    return {driversListFrom, devGroupsFrom, devicesFrom};
}
