#include "data.hpp"

#include <cmath>

#include "atom/type/json.hpp"

Accum::Accum() { reset(); }
void Accum::reset() {
    n_ = 0;
    a_ = q_ = peak_ = 0.;
}
void Accum::add(double x) {
    double ax = fabs(x);
    if (ax > peak_) {
        peak_ = ax;
    }
    ++n_;
    double d = x - a_;
    a_ += d / (double)n_;
    q_ += (x - a_) * d;
}
auto Accum::mean() const -> double { return a_; }
auto Accum::stdev() const -> double {
    return n_ >= 1 ? sqrt(q_ / (double)n_) : 0.0;
}
auto Accum::peak() const -> double { return peak_; }

void to_json(json &j, const Accum &acc) {
    j = json{{"n", acc.n_}, {"a", acc.a_}, {"q", acc.q_}, {"peak", acc.peak_}};
}

// 反序列化函数
void from_json(const json &j, Accum &acc) {
    j.at("n").get_to(acc.n_);
    j.at("a").get_to(acc.a_);
    j.at("q").get_to(acc.q_);
    j.at("peak").get_to(acc.peak_);
}

void to_json(json &j, const SettleProgress &sp) {
    j = json{{"done", sp.done},
             {"distance", sp.distance},
             {"settlePx", sp.settlePx},
             {"time", sp.time},
             {"settleTime", sp.settleTime},
             {"status", sp.status},
             {"error", sp.error}};
}

void from_json(const json &j, SettleProgress &sp) {
    j.at("done").get_to(sp.done);
    j.at("distance").get_to(sp.distance);
    j.at("settlePx").get_to(sp.settlePx);
    j.at("time").get_to(sp.time);
    j.at("settleTime").get_to(sp.settleTime);
    j.at("status").get_to(sp.status);
    j.at("error").get_to(sp.error);
}

void to_json(json &j, const GuideStats &gs) {
    j = json{{"rmsTot", gs.rmsTot},
             {"rmsRa", gs.rmsRa},
             {"rmsDec", gs.rmsDec},
             {"peakRa", gs.peakRa},
             {"peakDec", gs.peakDec}};
}

// 反序列化函数
void from_json(const json &j, GuideStats &gs) {
    j.at("rmsTot").get_to(gs.rmsTot);
    j.at("rmsRa").get_to(gs.rmsRa);
    j.at("rmsDec").get_to(gs.rmsDec);
    j.at("peakRa").get_to(gs.peakRa);
    j.at("peakDec").get_to(gs.peakDec);
}
