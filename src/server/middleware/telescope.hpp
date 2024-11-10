#ifndef LITHIUM_SERVER_MIDDLEWARE_TELESCOPE_HPP
#define LITHIUM_SERVER_MIDDLEWARE_TELESCOPE_HPP

namespace lithium::middleware {
void mountMoveWest();
void mountMoveEast();
void mountMoveNorth();
void mountMoveSouth();
void mountMoveAbort();
void mountPark();
void mountTrack();
void mountHome();
void mountSync();
void mountSpeedSwitch();
void mountGoto(double ra, double dec);
}  // namespace lithium::middleware 

#endif  // LITHIUM_SERVER_MIDDLEWARE_TELESCOPE_HPP