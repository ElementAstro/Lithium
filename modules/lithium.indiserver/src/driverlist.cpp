#include "driverlist.hpp"

#include <tinyxml2.h>
#include <filesystem>

#include "atom/log/loguru.hpp"

using namespace tinyxml2;
using namespace std::filesystem;

auto loadXMLFile(const std::string& filename, XMLDocument& doc) -> bool {
    LOG_F(INFO, "Loading XML file: {}", filename);
    if (doc.LoadFile(filename.c_str()) != XML_SUCCESS) {
        LOG_F(ERROR, "Unable to load XML file: {}", filename);
        return false;
    }
    LOG_F(INFO, "Successfully loaded XML file: {}", filename);
    return true;
}

auto parseDriversList(const std::string& filename) -> std::vector<DevGroup> {
    LOG_F(INFO, "Parsing drivers list from file: {}", filename);
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
        LOG_F(INFO, "Found devGroup: {}", devGroup.group);
        devGroups.push_back(devGroup);
    }

    LOG_F(INFO, "Completed parsing drivers list from file: {}", filename);
    return devGroups;
}

auto parseDevicesFromPath(const std::string& path,
                          std::vector<Device>& devicesFrom)
    -> std::vector<DevGroup> {
    LOG_F(INFO, "Parsing devices from path: {}", path);
    std::vector<DevGroup> devGroups;

    for (const auto& entry : directory_iterator(path)) {
        if (entry.path().extension() == ".xml" &&
            entry.path().filename().string().substr(
                entry.path().filename().string().size() - 6) != "sk.xml") {
            LOG_F(INFO, "Processing XML file: {}", entry.path().string());
            XMLDocument doc;

            if (!loadXMLFile(entry.path().string(), doc)) {
                LOG_F(ERROR, "Unable to load XML file: {}",
                      entry.path().string());
                continue;
            }

            XMLElement* root = doc.RootElement();
            for (XMLElement* devGroupElem = root->FirstChildElement("devGroup");
                 devGroupElem != nullptr;
                 devGroupElem = devGroupElem->NextSiblingElement("devGroup")) {
                DevGroup devGroup;
                devGroup.group = devGroupElem->Attribute("group");
                LOG_F(INFO, "Found devGroup: {}", devGroup.group);

                for (XMLElement* deviceElem =
                         devGroupElem->FirstChildElement("device");
                     deviceElem != nullptr;
                     deviceElem = deviceElem->NextSiblingElement("device")) {
                    Device device;
                    device.label = deviceElem->Attribute("label");
                    LOG_F(INFO, "Found device: {}", device.label);

                    if (deviceElem->FindAttribute("manufacturer") != nullptr) {
                        device.manufacturer =
                            deviceElem->Attribute("manufacturer");
                        LOG_F(INFO, "Device manufacturer: {}",
                              device.manufacturer);
                    }

                    for (XMLElement* driverElem =
                             deviceElem->FirstChildElement();
                         driverElem != nullptr;
                         driverElem = driverElem->NextSiblingElement()) {
                        if (std::string(driverElem->Name()) == "driver") {
                            device.driverName = driverElem->GetText();
                            LOG_F(INFO, "Device driver: {}", device.driverName);
                        } else if (std::string(driverElem->Name()) ==
                                   "version") {
                            device.version = driverElem->GetText();
                            LOG_F(INFO, "Device version: {}", device.version);
                        }
                    }
                    devGroup.devices.push_back(device);
                    devicesFrom.push_back(device);
                }
                devGroups.push_back(devGroup);
            }
        }
    }

    LOG_F(INFO, "Completed parsing devices from path: {}", path);
    return devGroups;
}

auto mergeDeviceGroups(const DriversList& driversListFrom,
                       const std::vector<DevGroup>& devGroupsFromPath)
    -> DriversList {
    LOG_F(INFO, "Merging device groups");
    DriversList mergedList = driversListFrom;

    for (auto& devGroupXml : devGroupsFromPath) {
        for (auto& devGroupFrom : mergedList.devGroups) {
            if (devGroupXml.group == devGroupFrom.group) {
                LOG_F(INFO, "Merging devices into group: {}",
                      devGroupXml.group);
                devGroupFrom.devices.insert(devGroupFrom.devices.end(),
                                            devGroupXml.devices.begin(),
                                            devGroupXml.devices.end());
            }
        }
    }

    LOG_F(INFO, "Completed merging device groups");
    return mergedList;
}

auto readDriversListFromFiles(std::string_view filename, std::string_view path)
    -> std::tuple<DriversList, std::vector<DevGroup>, std::vector<Device>> {
    LOG_F(INFO, "Reading drivers list from files: {} and path: {}", filename,
          path);
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

    LOG_F(INFO, "Completed reading drivers list from files");
    return {driversListFrom, devGroupsFrom, devicesFrom};
}
