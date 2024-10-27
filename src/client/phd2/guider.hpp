#ifndef LITHIUM_CLIENT_PHD2_GUIDER_HPP
#define LITHIUM_CLIENT_PHD2_GUIDER_HPP

#include <string>
#include <vector>

#include "atom/macro.hpp"

#include "atom/type/json_fwd.hpp"
using json = nlohmann::json;

class SettleProgress;
class GuideStats;

// Guider - a C++ wrapper for the PHD2 server API
//    https://github.com/OpenPHDGuiding/phd2/wiki/EventMonitoring
//
class Guider {
    class Impl;
    Impl *m_rep_;

public:
    // the constuctor takes the host name an instance number for the PHD2
    // server. Call Connect() to establish the connection to PHD2.
    explicit Guider(const char *hostname, unsigned int phd2_instance = 1);

    // The destructor will disconnect from PHD2
    ~Guider();

    // when any of the API methods below fail they will return false, and
    // additional error information can be retrieved by calling LastError()
    [[nodiscard]] auto lastError() const -> std::string;

    // connect to PHD2 -- you'll need to call Connect before calling any of the
    // server API methods below
    auto connect() -> bool;

    // disconnect from PHD2. The Guider destructor will do this automatically.
    void disconnect();

    // these two member functions are for raw JSONRPC method invocation.
    // Generally you won't need to use these functions as it is much more
    // convenient to use the higher-level methods below
    auto call(const std::string &method) -> json;
    auto call(const std::string &method, const json &params) -> json;

    // Start guiding with the given settling parameters. PHD2 takes care of
    // looping exposures, guide star selection, and settling. Call
    // CheckSettling() periodically to see when settling is complete.
    auto guide(double settlePixels, double settleTime,
               double settleTimeout) -> bool;

    // Dither guiding with the given dither amount and settling parameters. Call
    // CheckSettling() periodically to see when settling is complete.
    auto dither(double ditherPixels, double settlePixels, double settleTime,
                double settleTimeout) -> bool;

    // Check if phd2 is currently in the process of settling after a Guide or
    // Dither
    auto isSettling(bool *val) -> bool;

    // Get the progress of settling
    auto checkSettling(SettleProgress *s) -> bool;

    // Get the guider statistics since guiding started. Frames captured while
    // settling is in progress are excluded from the stats.
    auto getStats(GuideStats *stats) -> bool;

    // stop looping and guiding
    auto stopCapture(unsigned int timeoutSeconds = 10) -> bool;

    // start looping exposures
    auto loop(unsigned int timeoutSeconds = 10) -> bool;

    // get the guider pixel scale in arc-seconds per pixel
    auto pixelScale(double *result) -> bool;

    // get a list of the Equipment Profile names
    auto getEquipmentProfiles(std::vector<std::string> *profiles) -> bool;

    // connect the equipment in an equipment profile
    auto connectEquipment(const char *profileName) -> bool;

    // disconnect equipment
    auto disconnectEquipment() -> bool;

    // get the AppState
    // (https://github.com/OpenPHDGuiding/phd2/wiki/EventMonitoring#appstate)
    // and current guide error
    auto getStatus(std::string *appState, double *avgDist) -> bool;

    // check if currently guiding
    auto isGuiding(bool *result) -> bool;

    // pause guiding (looping exposures continues)
    auto pause() -> bool;

    // un-pause guiding
    auto unpause() -> bool;

    // save the current guide camera frame (FITS format), returning the name of
    // the file in *filename. The caller will need to remove the file when done.
    auto saveImage(std::string *filename) -> bool;
};

#endif  // LITHIUM_CLIENT_PHD2_GUIDER_HPP
