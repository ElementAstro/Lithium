/*
 * _pybind.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-13

Description: Python Binding of Atom-Driver

**************************************************/

#include <pybind11/pybind11.h>

#include "camera.hpp"
#include "device.hpp"
#include "filterwheel.hpp"
#include "focuser.hpp"
#include "solver.hpp"
#include "telescope.hpp"

#include "pid/pid.hpp"

namespace py = pybind11;

PYBIND11_MODULE(atom_driver, m) {
    m.doc() = "Atom Driver Python Binding";

    py::class_<AtomDriver>(m, "AtomDriver")
        .def(py::init<const std::string &>())           // 默认构造函数
        .def("initialize", &AtomDriver::initialize)     // initialize 方法
        .def("connect", &AtomDriver::connect)           // connect 方法
        .def("disconnect", &AtomDriver::disconnect)     // disconnect 方法
        .def("reconnect", &AtomDriver::reconnect)       // reconnect 方法
        .def("isConnected", &AtomDriver::isConnected)   // isConnected 方法
        .def("Connect", &AtomDriver::Connect)           // Connect 方法
        .def("Disconnect", &AtomDriver::Disconnect)     // Disconnect 方法
        .def("Reconnect", &AtomDriver::Reconnect)       // Reconnect 方法
        .def("IsConnected", &AtomDriver::IsConnected);  // IsConnected 方法

    py::class_<AtomCamera, AtomDriver>(m, "AtomCamera")
        .def(py::init<const std::string &>())              // 默认构造函数
        .def("initialize", &AtomCamera::initialize)        // initialize 方法
        .def("connect", &AtomCamera::connect)              // connect 方法
        .def("disconnect", &AtomCamera::disconnect)        // disconnect 方法
        .def("reconnect", &AtomCamera::reconnect)          // reconnect 方法
        .def("isConnected", &AtomCamera::isConnected)      // isConnected 方法
        .def("startExposure", &AtomCamera::startExposure)  // startExposure 方法
        .def("abortExposure", &AtomCamera::abortExposure)  // abortExposure 方法
        .def("getExposureStatus",
             &AtomCamera::getExposureStatus)  // getExposureStatus 方法
        .def("getExposureResult",
             &AtomCamera::getExposureResult)  // getExposureResult 方法
        .def("saveExposureResult",
             &AtomCamera::saveExposureResult)  // saveExposureResult 方法
        .def("startVideo", &AtomCamera::startVideo)  // startVideo 方法
        .def("stopVideo", &AtomCamera::stopVideo)    // stopVideo 方法
        .def("getVideoStatus",
             &AtomCamera::getVideoStatus)  // getVideoStatus 方法
        .def("getVideoResult",
             &AtomCamera::getVideoResult)  // getVideoResult 方法
        .def("saveVideoResult",
             &AtomCamera::saveVideoResult)  // saveVideoResult 方法
        .def("startCooling", &AtomCamera::startCooling)  // startCooling 方法
        .def("stopCooling", &AtomCamera::stopCooling)    // stopCooling 方法
        .def("getCoolingStatus",
             &AtomCamera::getCoolingStatus)  // getCoolingStatus 方法
        .def("isCoolingAvailable",
             &AtomCamera::isCoolingAvailable)  // isCoolingAvailable 方法
        .def("getTemperature",
             &AtomCamera::getTemperature)  // getTemperature 方法
        .def("getCoolingPower",
             &AtomCamera::getCoolingPower)  // getCoolingPower 方法
        .def("setTemperature",
             &AtomCamera::setTemperature)  // setTemperature 方法
        .def("setCoolingPower",
             &AtomCamera::setCoolingPower)     // setCoolingPower 方法
        .def("getGain", &AtomCamera::getGain)  // getGain 方法
        .def("setGain", &AtomCamera::setGain)  // setGain 方法
        .def("isGainAvailable",
             &AtomCamera::isGainAvailable)         // isGainAvailable 方法
        .def("getOffset", &AtomCamera::getOffset)  // getOffset 方法
        .def("setOffset", &AtomCamera::setOffset)  // setOffset 方法
        .def("isOffsetAvailable",
             &AtomCamera::isOffsetAvailable)  // isOffsetAvailable 方法
        .def("getISO", &AtomCamera::getISO)   // getISO 方法
        .def("setISO", &AtomCamera::setISO)   // setISO 方法
        .def("isISOAvailable",
             &AtomCamera::isISOAvailable)        // isISOAvailable 方法
        .def("getFrame", &AtomCamera::getFrame)  // getFrame 方法
        .def("setFrame", &AtomCamera::setFrame)  // setFrame 方法
        .def("isFrameSettingAvailable",
             &AtomCamera::isFrameSettingAvailable)    // isFrameSettingAvailable
                                                      // 方法
        .def("getBinning", &AtomCamera::getBinning)   // getBinning 方法
        .def("setBinning", &AtomCamera::setBinning);  // setBinning 方法

    py::class_<Telescope, AtomDriver>(m, "Telescope")
        .def(py::init<const std::string &>())             // 默认构造函数
        .def("connect", &Telescope::connect)              // connect 方法
        .def("disconnect", &Telescope::disconnect)        // disconnect 方法
        .def("reconnect", &Telescope::reconnect)          // reconnect 方法
        .def("isConnected", &Telescope::isConnected)      // isConnected 方法
        .def("SlewTo", &Telescope::SlewTo)                // SlewTo 方法
        .def("Abort", &Telescope::Abort)                  // Abort 方法
        .def("isSlewing", &Telescope::isSlewing)          // isSlewing 方法
        .def("getCurrentRA", &Telescope::getCurrentRA)    // getCurrentRA 方法
        .def("getCurrentDec", &Telescope::getCurrentDec)  // getCurrentDec 方法
        .def("StartTracking", &Telescope::StartTracking)  // StartTracking 方法
        .def("StopTracking", &Telescope::StopTracking)    // StopTracking 方法
        .def("setTrackingMode",
             &Telescope::setTrackingMode)  // setTrackingMode 方法
        .def("setTrackingSpeed",
             &Telescope::setTrackingSpeed)  // setTrackingSpeed 方法
        .def("getTrackingMode",
             &Telescope::getTrackingMode)  // getTrackingMode 方法
        .def("getTrackingSpeed",
             &Telescope::getTrackingSpeed)      // getTrackingSpeed 方法
        .def("Home", &Telescope::Home)          // Home 方法
        .def("isAtHome", &Telescope::isAtHome)  // isAtHome 方法
        .def("setHomePosition",
             &Telescope::setHomePosition)  // setHomePosition 方法
        .def("isHomeAvailable",
             &Telescope::isHomeAvailable)       // isHomeAvailable 方法
        .def("Park", &Telescope::Park)          // Park 方法
        .def("Unpark", &Telescope::Unpark)      // Unpark 方法
        .def("isAtPark", &Telescope::isAtPark)  // isAtPark 方法
        .def("setParkPosition",
             &Telescope::setParkPosition)  // setParkPosition 方法
        .def("isParkAvailable",
             &Telescope::isParkAvailable);  // isParkAvailable 方法

    // 继承 AtomDriver 的 Focuser 类
    py::class_<Focuser, AtomDriver>(m, "Focuser")
        .def(py::init<const std::string &>())             // 默认构造函数
        .def("connect", &Focuser::connect)                // connect 方法
        .def("disconnect", &Focuser::disconnect)          // disconnect 方法
        .def("reconnect", &Focuser::reconnect)            // reconnect 方法
        .def("isConnected", &Focuser::isConnected)        // isConnected 方法
        .def("moveTo", &Focuser::moveTo)                  // moveTo 方法
        .def("moveToAbsolute", &Focuser::moveToAbsolute)  // moveToAbsolute 方法
        .def("moveStep", &Focuser::moveStep)              // moveStep 方法
        .def("moveStepAbsolute",
             &Focuser::moveStepAbsolute)        // moveStepAbsolute 方法
        .def("AbortMove", &Focuser::AbortMove)  // AbortMove 方法
        .def("getMaxPosition", &Focuser::getMaxPosition)  // getMaxPosition 方法
        .def("setMaxPosition", &Focuser::setMaxPosition)  // setMaxPosition 方法
        .def("isGetTemperatureAvailable",
             &Focuser::isGetTemperatureAvailable)  // isGetTemperatureAvailable
                                                   // 方法
        .def("getTemperature", &Focuser::getTemperature)  // getTemperature 方法
        .def("isAbsoluteMoveAvailable",
             &Focuser::isAbsoluteMoveAvailable)  // isAbsoluteMoveAvailable 方法
        .def("isManualMoveAvailable",
             &Focuser::isManualMoveAvailable)  // isManualMoveAvailable 方法
        .def("getCurrentPosition",
             &Focuser::getCurrentPosition)  // getCurrentPosition 方法
        .def("haveBacklash", &Focuser::haveBacklash)  // haveBacklash 方法
        .def("setBacklash", &Focuser::setBacklash);   // setBacklash 方法

    py::class_<Filterwheel, AtomDriver>(m, "Filterwheel")
        .def(py::init<const std::string &>())           // 默认构造函数
        .def("connect", &Filterwheel::connect)          // connect 方法
        .def("disconnect", &Filterwheel::disconnect)    // disconnect 方法
        .def("reconnect", &Filterwheel::reconnect)      // reconnect 方法
        .def("isConnected", &Filterwheel::isConnected)  // isConnected 方法
        .def("moveTo", &Filterwheel::moveTo)            // moveTo 方法
        .def("getCurrentPosition",
             &Filterwheel::getCurrentPosition);  // getCurrentPosition 方法

    py::class_<PID>(m, "PID")
        .def(py::init<double, double, double, double, double,
                      double>())  // 构造函数
        .def("setIntegratorLimits",
             &PID::setIntegratorLimits)     // setIntegratorLimits 方法
        .def("setTau", &PID::setTau)        // setTau 方法
        .def("calculate", &PID::calculate)  // calculate 方法
        .def("propotionalTerm", &PID::propotionalTerm)  // propotionalTerm 方法
        .def("integralTerm", &PID::integralTerm)        // integralTerm 方法
        .def("derivativeTerm", &PID::derivativeTerm);   // derivativeTerm 方法

    py::class_<Solver, AtomDriver>(m, "Solver")
        .def(py::init<const std::string &>()) // 构造函数
        .def("solveImage", &Solver::solveImage) // solveImage 方法
        .def("getSolveResult", &Solver::getSolveResult) // getSolveResult 方法
        .def("getSolveStatus", &Solver::getSolveStatus) // getSolveStatus 方法
        .def("setSolveParams", &Solver::setSolveParams) // setSolveParams 方法
        .def("getSolveParams", &Solver::getSolveParams); // getSolveParams 方法
}
