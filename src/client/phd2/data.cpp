#include "data.hpp"

#include <cmath>

#include "atom/type/json.hpp"

Accum::Accum() { reset(); }
void Accum::reset() {
    n_ = 0;
    a_ = q_ = peak_ = 0.;
}
void Accum::add(double value) {
    double absValue = fabs(value);
    if (absValue > peak_) {
        peak_ = absValue;
    }
    ++n_;
    double delta = value - a_;
    a_ += delta / (double)n_;
    q_ += (value - a_) * delta;
}
auto Accum::mean() const -> double { return a_; }
auto Accum::stdev() const -> double {
    return n_ >= 1 ? sqrt(q_ / (double)n_) : 0.0;
}
auto Accum::peak() const -> double { return peak_; }

void to_json(json &jsonObj, const Accum &acc) {
    jsonObj =
        json{{"n", acc.n_}, {"a", acc.a_}, {"q", acc.q_}, {"peak", acc.peak_}};
}

// 反序列化函数
void from_json(const json &jsonObj, Accum &acc) {
    jsonObj.at("n").get_to(acc.n_);
    jsonObj.at("a").get_to(acc.a_);
    jsonObj.at("q").get_to(acc.q_);
    jsonObj.at("peak").get_to(acc.peak_);
}

void to_json(json &jsonObj, const SettleProgress &settleProgress) {
    jsonObj = json{{"done", settleProgress.done},
                   {"distance", settleProgress.distance},
                   {"settlePx", settleProgress.settlePx},
                   {"time", settleProgress.time},
                   {"settleTime", settleProgress.settleTime},
                   {"status", settleProgress.status},
                   {"error", settleProgress.error}};
}

void from_json(const json &jsonObj, SettleProgress &settleProgress) {
    jsonObj.at("done").get_to(settleProgress.done);
    jsonObj.at("distance").get_to(settleProgress.distance);
    jsonObj.at("settlePx").get_to(settleProgress.settlePx);
    jsonObj.at("time").get_to(settleProgress.time);
    jsonObj.at("settleTime").get_to(settleProgress.settleTime);
    jsonObj.at("status").get_to(settleProgress.status);
    jsonObj.at("error").get_to(settleProgress.error);
}

void to_json(json &jsonObj, const GuideStats &guideStats) {
    jsonObj = json{{"rmsTot", guideStats.rmsTot},
                   {"rmsRa", guideStats.rmsRa},
                   {"rmsDec", guideStats.rmsDec},
                   {"peakRa", guideStats.peakRa},
                   {"peakDec", guideStats.peakDec}};
}

// 反序列化函数
void from_json(const json &jsonObj, GuideStats &guideStats) {
    jsonObj.at("rmsTot").get_to(guideStats.rmsTot);
    jsonObj.at("rmsRa").get_to(guideStats.rmsRa);
    jsonObj.at("rmsDec").get_to(guideStats.rmsDec);
    jsonObj.at("peakRa").get_to(guideStats.peakRa);
    jsonObj.at("peakDec").get_to(guideStats.peakDec);
}