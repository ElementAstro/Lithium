#ifndef LITHIUM_CLIENT_PHD2_DATA_HPP
#define LITHIUM_CLIENT_PHD2_DATA_HPP

#include "atom/macro.hpp"

#include "atom/type/json_fwd.hpp"

using json = nlohmann::json;

class Accum {
public:
    Accum();

    void reset();

    void add(double x);

    ATOM_NODISCARD auto mean() const -> double;
    ATOM_NODISCARD auto stdev() const -> double;
    ATOM_NODISCARD auto peak() const -> double;

    unsigned int n_{};
    double a_{};
    double q_{};
    double peak_{};
};

void to_json(json &j, const Accum &acc);

void from_json(const json &j, Accum &acc);

struct SettleProgress {
    bool done;
    double distance;
    double settlePx;
    double time;
    double settleTime;
    int status;
    std::string error;
} ATOM_ALIGNAS(128);

// guiding statistics information returned by Guider::GetStats()
struct GuideStats {
    double rmsTot;
    double rmsRa;
    double rmsDec;
    double peakRa;
    double peakDec;
} ATOM_ALIGNAS(64);

void to_json(json &j, const SettleProgress &sp);

// 反序列化函数
void from_json(const json &j, SettleProgress &sp);

void to_json(json &j, const GuideStats &gs);

// 反序列化函数
void from_json(const json &j, GuideStats &gs);

#endif
