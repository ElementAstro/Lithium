#include "switch.hpp"
#include <stdexcept>

AlpacaSwitch::AlpacaSwitch(const std::string& address, int device_number,
                           const std::string& protocol)
    : AlpacaDevice(address, "switch", device_number, protocol) {}

int AlpacaSwitch::GetMaxSwitch() {
    return GetNumericProperty<int>("maxswitch");
}

bool AlpacaSwitch::CanWrite(int Id) {
    return GetSwitchProperty<bool>("canwrite", Id);
}

bool AlpacaSwitch::GetSwitch(int Id) {
    return GetSwitchProperty<bool>("getswitch", Id);
}

std::string AlpacaSwitch::GetSwitchDescription(int Id) {
    return Get("getswitchdescription", {{"Id", std::to_string(Id)}});
}

std::string AlpacaSwitch::GetSwitchName(int Id) {
    return Get("getswitchname", {{"Id", std::to_string(Id)}});
}

double AlpacaSwitch::GetSwitchValue(int Id) {
    return GetSwitchProperty<double>("getswitchvalue", Id);
}

double AlpacaSwitch::MaxSwitchValue(int Id) {
    return GetSwitchProperty<double>("maxswitchvalue", Id);
}

double AlpacaSwitch::MinSwitchValue(int Id) {
    return GetSwitchProperty<double>("minswitchvalue", Id);
}

void AlpacaSwitch::SetSwitch(int Id, bool State) {
    SetSwitchProperty("setswitch", Id, State);
}

void AlpacaSwitch::SetSwitchName(int Id, const std::string& Name) {
    SetSwitchProperty("setswitchname", Id, Name);
}

void AlpacaSwitch::SetSwitchValue(int Id, double Value) {
    SetSwitchProperty("setswitchvalue", Id, Value);
}

double AlpacaSwitch::SwitchStep(int Id) {
    return GetSwitchProperty<double>("switchstep", Id);
}