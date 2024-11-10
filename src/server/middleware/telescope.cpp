#include "telescope.hpp"

#include "config/configor.hpp"

#include "atom/async/message_bus.hpp"
#include "atom/async/timer.hpp"
#include "atom/function/global_ptr.hpp"
#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"
#include "atom/utils/print.hpp"

#include "device/template/telescope.hpp"

#include "utils/constant.hpp"

namespace lithium::middleware {
void mountMoveWest() {
    LOG_F(INFO, "mountMoveWest: Entering function");
    std::shared_ptr<AtomTelescope> telescope;
    GET_OR_CREATE_PTR(telescope, AtomTelescope, Constants::MAIN_TELESCOPE)
    telescope->setTelescopeMoveWE(MotionEW::WEST);
    LOG_F(INFO, "mountMoveWest: Exiting function");
}

void mountMoveEast() {
    LOG_F(INFO, "mountMoveEast: Entering function");
    std::shared_ptr<AtomTelescope> telescope;
    GET_OR_CREATE_PTR(telescope, AtomTelescope, Constants::MAIN_TELESCOPE)
    telescope->setTelescopeMoveWE(MotionEW::EAST);
    LOG_F(INFO, "mountMoveEast: Exiting function");
}

void mountMoveNorth() {
    LOG_F(INFO, "mountMoveNorth: Entering function");
    std::shared_ptr<AtomTelescope> telescope;
    GET_OR_CREATE_PTR(telescope, AtomTelescope, Constants::MAIN_TELESCOPE)
    telescope->setTelescopeMoveNS(MotionNS::NORTH);
    LOG_F(INFO, "mountMoveNorth: Exiting function");
}

void mountMoveSouth() {
    LOG_F(INFO, "mountMoveSouth: Entering function");
    std::shared_ptr<AtomTelescope> telescope;
    GET_OR_CREATE_PTR(telescope, AtomTelescope, Constants::MAIN_TELESCOPE)
    telescope->setTelescopeMoveNS(MotionNS::SOUTH);
    LOG_F(INFO, "mountMoveSouth: Exiting function");
}

void mountMoveAbort() {
    LOG_F(INFO, "mountMoveAbort: Entering function");
    std::shared_ptr<AtomTelescope> telescope;
    GET_OR_CREATE_PTR(telescope, AtomTelescope, Constants::MAIN_TELESCOPE)
    telescope->setTelescopeAbortMotion();
    LOG_F(INFO, "mountMoveAbort: Exiting function");
}

void mountPark() {
    LOG_F(INFO, "mountPark: Entering function");
    std::shared_ptr<AtomTelescope> telescope;
    GET_OR_CREATE_PTR(telescope, AtomTelescope, Constants::MAIN_TELESCOPE)
    auto parkStatus = telescope->getTelescopePark();
    if (parkStatus) {
        telescope->setTelescopePark(false);
    } else {
        telescope->setTelescopePark(true);
    }
    parkStatus = telescope->getTelescopePark();
    std::shared_ptr<atom::async::MessageBus> messageBusPtr;
    GET_OR_CREATE_PTR(messageBusPtr, atom::async::MessageBus,
                      Constants::MESSAGE_BUS)
    messageBusPtr->publish("main",
                           "TelescopePark:{}"_fmt(parkStatus ? "ON" : "OFF"));
    LOG_F(INFO, "mountPark: Park status: %s",
          parkStatus ? "Parked" : "Unparked");
    LOG_F(INFO, "mountPark: Exiting function");
}

void mountTrack() {
    LOG_F(INFO, "mountTrack: Entering function");
    std::shared_ptr<AtomTelescope> telescope;
    GET_OR_CREATE_PTR(telescope, AtomTelescope, Constants::MAIN_TELESCOPE)
    auto trackStatus = telescope->getTelescopeTrackEnable();
    if (trackStatus) {
        telescope->setTelescopeTrackEnable(false);
    } else {
        telescope->setTelescopeTrackEnable(true);
    }
    trackStatus = telescope->getTelescopeTrackEnable();
    std::shared_ptr<atom::async::MessageBus> messageBusPtr;
    GET_OR_CREATE_PTR(messageBusPtr, atom::async::MessageBus,
                      Constants::MESSAGE_BUS)
    messageBusPtr->publish("main",
                           "TelescopeTrack:{}"_fmt(trackStatus ? "ON" : "OFF"));
    LOG_F(INFO, "mountTrack: Track status: %s",
          trackStatus ? "Tracking" : "Not tracking");
    LOG_F(INFO, "mountTrack: Exiting function");
}

void mountHome() {
    LOG_F(INFO, "mountHome: Entering function");
    std::shared_ptr<AtomTelescope> telescope;
    GET_OR_CREATE_PTR(telescope, AtomTelescope, Constants::MAIN_TELESCOPE)
    telescope->setTelescopeHomeInit("SLEWHOME");
    LOG_F(INFO, "mountHome: Exiting function");
}

void mountSync() {
    LOG_F(INFO, "mountSync: Entering function");
    std::shared_ptr<AtomTelescope> telescope;
    GET_OR_CREATE_PTR(telescope, AtomTelescope, Constants::MAIN_TELESCOPE)
    telescope->setTelescopeHomeInit("SYNCHOME");
    LOG_F(INFO, "mountSync: Exiting function");
}

void mountSpeedSwitch() {
    LOG_F(INFO, "mountSpeedSwitch: Entering function");
    std::shared_ptr<AtomTelescope> telescope;
    GET_OR_CREATE_PTR(telescope, AtomTelescope, Constants::MAIN_TELESCOPE)
    auto speed = telescope->getTelescopeSlewRate().value();
    std::shared_ptr<ConfigManager> configManager;
    GET_OR_CREATE_PTR(configManager, ConfigManager, Constants::CONFIG_MANAGER)
    int glTelescopeTotalSlewRate =
        configManager->getValue("/lithium/device/telescope/total_slew_rate")
            .value();
    if (speed == glTelescopeTotalSlewRate) {
        telescope->setTelescopeSlewRate(1);
    } else {
        telescope->setTelescopeSlewRate(speed + 1);
    }
    speed = telescope->getTelescopeSlewRate().value();
    std::shared_ptr<atom::async::MessageBus> messageBusPtr;
    GET_OR_CREATE_PTR(messageBusPtr, atom::async::MessageBus,
                      Constants::MESSAGE_BUS)
    messageBusPtr->publish("main", "MountSetSpeedSuccess:{}"_fmt(speed));
    LOG_F(INFO, "mountSpeedSwitch: Speed: %d", speed);
    LOG_F(INFO, "mountSpeedSwitch: Exiting function");
}

void mountGoto(double ra, double dec) {
    LOG_F(INFO, "mountGoto: Entering function with RA: %f, DEC: %f", ra, dec);
    std::shared_ptr<AtomTelescope> telescope;
    GET_OR_CREATE_PTR(telescope, AtomTelescope, Constants::MAIN_TELESCOPE)

    std::shared_ptr<atom::async::Timer> timer;
    GET_OR_CREATE_PTR(timer, atom::async::Timer, Constants::MAIN_TIMER)
    telescope->setTelescopeRADECJNOW(ra, dec);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    timer->setInterval(
        []() {
            std::shared_ptr<AtomTelescope> telescope;
            GET_OR_CREATE_PTR(telescope, AtomTelescope,
                              Constants::MAIN_TELESCOPE)
            auto status = telescope->getTelescopeStatus();
            if (status && status.value() != "Slewing") {
                LOG_F(INFO, "mountGoto: Goto status: %s",
                      status.value().c_str());
                std::shared_ptr<atom::async::MessageBus> messageBusPtr;
                GET_OR_CREATE_PTR(messageBusPtr, atom::async::MessageBus,
                                  Constants::MESSAGE_BUS)
                messageBusPtr->publish(
                    "main", "MountGotoStatus:{}"_fmt(status.value()));
            }
        },
        1000, 10, 0);
    LOG_F(INFO, "mountGoto: Exiting function");
}
}  // namespace lithium::middleware