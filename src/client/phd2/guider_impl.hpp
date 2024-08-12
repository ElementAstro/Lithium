#ifndef LITHIUM_CLIENT_PHD2_GUIDER_IMPL_HPP
#define LITHIUM_CLIENT_PHD2_GUIDER_IMPL_HPP

#include <condition_variable>

#include "connection.hpp"
#include "data.hpp"
#include "guider.hpp"

#include "atom/type/json.hpp"

class Guider::Impl {
    std::string m_host_;
    unsigned int m_instance_;
    GuiderConnection m_conn_;

    std::thread m_worker_;

    std::atomic_bool m_terminate_;

    std::mutex m_mutex_;
    std::condition_variable m_cond_;
    json m_response_;

    // private to worker thread
    Accum accum_ra_;
    Accum accum_dec_;
    bool accum_active_{};
    double settle_px_{};

    std::string AppState_;
    double AvgDist_{};
    GuideStats Stats_{};
    std::string Version_;
    std::string PHDSubver_;
    std::unique_ptr<SettleProgress> mSettle_;

    void worker();
    void handleEvent(const json &ev);

public:
    std::string error;

    Impl(const char *hostname, unsigned int phd2_instance);
    ~Impl();

    auto connect() -> bool;
    void disconnect();

    auto call(const std::string &method) -> json;
    auto call(const std::string &method, const json &params) -> json;

    auto guide(double settlePixels, double settleTime,
               double settleTimeout) -> bool;
    auto dither(double ditherPixels, double settlePixels, double settleTime,
                double settleTimeout) -> bool;
    auto isSettling(bool *ret) -> bool;
    auto checkSettling(SettleProgress *s) -> bool;
    auto getStats(GuideStats *stats) -> bool;
    auto stopCapture(unsigned int timeoutSeconds = 10) -> bool;
    auto loop(unsigned int timeoutSeconds = 10) -> bool;
    auto pixelScale(double *result) -> bool;
    auto getEquipmentProfiles(std::vector<std::string> *profiles) -> bool;
    auto connectEquipment(const char *profileName) -> bool;
    auto disconnectEquipment() -> bool;
    auto getStatus(std::string *appState, double *avgDist) -> bool;
    auto isGuiding(bool *result) -> bool;
    auto pause() -> bool;
    auto unpause() -> bool;
    auto saveImage(std::string *filename) -> bool;
};

#endif
