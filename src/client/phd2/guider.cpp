#include "guider.hpp"
#include "connection.hpp"
#include "data.hpp"
#include "guider_impl.hpp"

#include <curl/curl.h>
#include <atomic>
#include <cmath>
#include <condition_variable>
#include <cstdarg>
#include <format>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <thread>

#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"

Guider::Guider(const char *hostname, unsigned int phd2_instance)
    : m_rep_(new Impl(hostname, phd2_instance)) {}

Guider::~Guider() { delete m_rep_; }

auto Guider::lastError() const -> std::string { return m_rep_->error; }

auto Guider::connect() -> bool { return m_rep_->connect(); }

void Guider::disconnect() { return m_rep_->disconnect(); }

auto Guider::call(const std::string &method) -> json {
    return m_rep_->call(method);
}

auto Guider::call(const std::string &method, const json &params) -> json {
    return m_rep_->call(method, params);
}

auto Guider::guide(double settlePixels, double settleTime,
                   double settleTimeout) -> bool {
    return m_rep_->guide(settlePixels, settleTime, settleTimeout);
}

auto Guider::dither(double ditherPixels, double settlePixels, double settleTime,
                    double settleTimeout) -> bool {
    return m_rep_->dither(ditherPixels, settlePixels, settleTime,
                          settleTimeout);
}

auto Guider::isSettling(bool *val) -> bool { return m_rep_->isSettling(val); }

auto Guider::checkSettling(SettleProgress *s) -> bool {
    return m_rep_->checkSettling(s);
}

auto Guider::getStats(GuideStats *stats) -> bool {
    return m_rep_->getStats(stats);
}

auto Guider::stopCapture(unsigned int timeoutSeconds) -> bool {
    return m_rep_->stopCapture(timeoutSeconds);
}

auto Guider::loop(unsigned int timeoutSeconds) -> bool {
    return m_rep_->loop(timeoutSeconds);
}

auto Guider::pixelScale(double *result) -> bool {
    return m_rep_->pixelScale(result);
}

auto Guider::getEquipmentProfiles(std::vector<std::string> *profiles) -> bool {
    return m_rep_->getEquipmentProfiles(profiles);
}

auto Guider::connectEquipment(const char *profileName) -> bool {
    return m_rep_->connectEquipment(profileName);
}

auto Guider::disconnectEquipment() -> bool {
    return m_rep_->disconnectEquipment();
}

auto Guider::getStatus(std::string *appState, double *avgDist) -> bool {
    return m_rep_->getStatus(appState, avgDist);
}

auto Guider::isGuiding(bool *result) -> bool {
    return m_rep_->isGuiding(result);
}

auto Guider::pause() -> bool { return m_rep_->pause(); }

auto Guider::unpause() -> bool { return m_rep_->unpause(); }

auto Guider::saveImage(std::string *filename) -> bool {
    return m_rep_->saveImage(filename);
}
