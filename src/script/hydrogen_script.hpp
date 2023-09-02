#include <chaiscript/chaiscript.hpp>
#include <string>

#include "Hydrogen/core/camera.hpp"
#include "Hydrogen/core/device.hpp"
#include "Hydrogen/core/device_exception.hpp"
#include "Hydrogen/core/filterwheel.hpp"
#include "Hydrogen/core/focuser.hpp"
#include "Hydrogen/core/guider.hpp"
#include "Hydrogen/core/solver.hpp"
#include "Hydrogen/core/telescope.hpp"


CHAISCRIPT_MODULE_EXPORT chaiscript::ModulePtr create_chaiscript_device_module()
{
    chaiscript::ModulePtr m = std::make_shared<chaiscript::Module>();
    m->add(chaiscript::user_type<Device>(), "Device");
    m->add(chaiscript::user_type<Camera>(), "Camera");
    m->add(chaiscript::user_type<Focuser>(), "Focuser");
    m->add(chaiscript::user_type<Filterwheel>(), "Filterheel");
    m->add(chaiscript::user_type<Telescope>(), "Telescope");

    m->add(chaiscript::constructor<Device(const std::string &)>(), "Device");
    m->add(chaiscript::constructor<Camera(const std::string &)>(), "Camera");
    m->add(chaiscript::constructor<Focuser(const std::string &)>(), "Focuser");
    m->add(chaiscript::constructor<Filterwheel(const std::string &)>(), "Filterwheel");
    m->add(chaiscript::constructor<Telescope(const std::string &)>(), "Telescope");

    m->add(chaiscript::base_class<Device, Camera>());
    m->add(chaiscript::base_class<Device, Telescope>());
    m->add(chaiscript::base_class<Device, Focuser>());
    m->add(chaiscript::base_class<Device, Filterwheel>());

    m->add(chaiscript::fun(&Device::getStringProperty), "getStringProperty");
    m->add(chaiscript::fun(&Device::getNumberProperty), "getNumberProperty");
    m->add(chaiscript::fun(&Device::getBoolProperty), "getBoolProperty");
    m->add(chaiscript::fun(&Device::getTask), "getTask");
    m->add(chaiscript::fun(&Device::removeTask), "removeTask");
    m->add(chaiscript::fun(&Device::insertTask), "insertTask");
    m->add(chaiscript::fun(&Device::addObserver), "addObserver");
    m->add(chaiscript::fun(&Device::removeObserver), "removeObserver");
    m->add(chaiscript::fun(&Device::connect), "connect");
    m->add(chaiscript::fun(&Device::disconnect), "disconnect");
    m->add(chaiscript::fun(&Device::reconnect), "reconnect");
    m->add(chaiscript::fun(&Device::removeTask), "removeTask");
    m->add(chaiscript::fun(&Device::init), "init");
    m->add(chaiscript::fun(&Device::exportDeviceInfoToJson), "exportDeviceInfoToJson");

    m->add(chaiscript::fun(&Camera::startExposure), "startExposure");
    m->add(chaiscript::fun(&Camera::abortExposure), "abortExposure");
    m->add(chaiscript::fun(&Camera::getExposureResult), "getExposureResult");
    m->add(chaiscript::fun(&Camera::getExposureStatus), "getExposureStatus");
    m->add(chaiscript::fun(&Camera::startVideo), "startVideo");
    m->add(chaiscript::fun(&Camera::stopVideo), "stopVideo");
    m->add(chaiscript::fun(&Camera::getVideoResult), "getVideoResult");
    m->add(chaiscript::fun(&Camera::getVideoStatus), "getVideoStatus");
    m->add(chaiscript::fun(&Camera::getCoolingPower), "getCoolingPower");
    m->add(chaiscript::fun(&Camera::startCooling), "startCooling");
    m->add(chaiscript::fun(&Camera::stopCooling), "stopCooling");
    m->add(chaiscript::fun(&Camera::getTemperature), "getTemperature");
    m->add(chaiscript::fun(&Camera::connect), "connect");
    m->add(chaiscript::fun(&Camera::disconnect), "disconnect");
    m->add(chaiscript::fun(&Camera::reconnect), "reconnect");
    m->add(chaiscript::fun(&Camera::init), "init");
    m->add(chaiscript::fun(&Camera::getGain), "getGain");
    m->add(chaiscript::fun(&Camera::setGain), "setGain");
    m->add(chaiscript::fun(&Camera::getOffset), "getOffset");
    m->add(chaiscript::fun(&Camera::setOffset), "setOffset");
    m->add(chaiscript::fun(&Camera::getISO), "getISO");
    m->add(chaiscript::fun(&Camera::setISO), "setISO");
    m->add(chaiscript::fun(&Camera::getFrame), "getFrame");
    m->add(chaiscript::fun(&Camera::setFrame), "setFrame");

    return m;
}

CHAISCRIPT_MODULE_EXPORT chaiscript::ModulePtr create_chaiscript_hydrogen_module()
{
    chaiscript::ModulePtr m = std::make_shared<chaiscript::Module>();
    return m;
}