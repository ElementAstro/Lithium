#include "guider_impl.hpp"

#include <format>

#include "atom/log/loguru.hpp"

Guider::Impl::Impl(const char *hostname, unsigned int phd2_instance)
    : m_host_(hostname),
      m_instance_(phd2_instance),
      m_terminate_(ATOMIC_VAR_INIT(false)) {}

Guider::Impl::~Impl() { disconnect(); }

auto Guider::Impl::connect() -> bool {
    disconnect();

    unsigned short port = 4400 + m_instance_ - 1;
    if (!m_conn_.connect(m_host_.c_str(), port)) {
        error = std::format("Could not connect to PHD2 instance {} on {}",
                            m_instance_, m_host_);
        return false;
    }

    m_terminate_ = false;
    m_worker_ = std::thread(&Impl::worker, this);

    return true;
}

void Guider::Impl::disconnect() {
    if (m_worker_.joinable()) {
        m_terminate_ = true;
        m_conn_.terminate();
        m_worker_.join();
    }

    m_conn_.disconnect();
}

static void accumGetStats(GuideStats *stats, const Accum &ra,
                          const Accum &dec) {
    stats->rmsRa = ra.stdev();
    stats->rmsDec = dec.stdev();
    stats->peakRa = ra.peak();
    stats->peakDec = dec.peak();
}

static auto isGuiding_(const std::string &st) -> bool {
    return st == "Guiding" || st == "LostLock";
}

void Guider::Impl::handleEvent(const json &ev) {
    std::string event;
    if (ev.contains("Event")) {
        event = ev["Event"].get<std::string>();
    }
    if (event == "AppState") {
        std::unique_lock lock(m_mutex_);
        AppState_ = ev["State"].get<std::string>();
        if (isGuiding_(AppState_)) {
            AvgDist_ = 0.;  // until we get a GuideStep event
        }
    } else if (event == "Version") {
        std::unique_lock lock(m_mutex_);
        Version_ = ev["PHDVersion"].get<std::string>();
        PHDSubver_ = ev["PHDSubver"].get<std::string>();
    } else if (event == "StartGuiding") {
        accum_active_ = true;
        accum_ra_.reset();
        accum_dec_.reset();

        GuideStats stats{};
        accumGetStats(&stats, accum_ra_, accum_dec_);

        {
            std::unique_lock lock(m_mutex_);
            Stats_ = stats;
        }
    } else if (event == "GuideStep") {
        GuideStats stats{};
        if (accum_active_) {
            accum_ra_.add(ev["RADistanceRaw"].get<double>());
            accum_dec_.add(ev["DECDistanceRaw"].get<double>());
            accumGetStats(&stats, accum_ra_, accum_dec_);
        }
        std::unique_lock lock(m_mutex_);
        AppState_ = "Guiding";
        AvgDist_ = ev["AvgDist"].get<double>();
        if (accum_active_) {
            Stats_ = stats;
        }
    } else if (event == "SettleBegin") {
        accum_active_ =
            false;  // exclude GuideStep messages from stats while settling
    } else if (event == "Settling") {
        std::unique_ptr<SettleProgress> s(new SettleProgress());
        s->done = false;
        s->distance = ev["Distance"].get<double>();
        s->settlePx = settle_px_;
        s->time = ev["Time"].get<double>();
        s->settleTime = ev["SettleTime"].get<double>();
        s->status = 0;
        {
            std::unique_lock lock(m_mutex_);
            mSettle_.swap(s);
        }
    } else if (event == "SettleDone") {
        accum_active_ = true;
        accum_ra_.reset();
        accum_dec_.reset();

        GuideStats stats{};
        accumGetStats(&stats, accum_ra_, accum_dec_);

        std::unique_ptr<SettleProgress> s(new SettleProgress());
        s->done = true;
        s->status = ev["Status"].get<int>();
        s->error = ev["Error"].get<std::string>();

        {
            std::unique_lock lock(m_mutex_);
            mSettle_.swap(s);
            Stats_ = stats;
        }
    } else if (event == "Paused") {
        std::unique_lock lock(m_mutex_);
        AppState_ = "Paused";
    } else if (event == "StartCalibration") {
        std::unique_lock lock(m_mutex_);
        AppState_ = "Calibrating";
    } else if (event == "LoopingExposures") {
        std::unique_lock lock(m_mutex_);
        AppState_ = "Looping";
    } else if (event == "LoopingExposuresStopped" ||
               event == "GuidingStopped") {
        std::unique_lock lock(m_mutex_);
        AppState_ = "Stopped";
    } else if (event == "StarLost") {
        std::unique_lock lock(m_mutex_);
        AppState_ = "LostLock";
        AvgDist_ = ev["AvgDist"].get<double>();
    } else {
        LOG_F(INFO, "Unhandled event: {}", event);
    }
}

void Guider::Impl::worker() {
    while (!m_terminate_) {
        std::string line = m_conn_.readLine().value();
        if (line.empty()) {
            // todo: re-connect (?)
            break;
        }

        LOG_F(INFO, "Receive message: {}", line);

        std::istringstream is(line);
        json j;
        try {
            is >> j;
        } catch (const std::exception &ex) {
            LOG_F(ERROR, "error parsing json: {}", ex.what());
            continue;
        }

        if (j.contains("jsonrpc")) {
            // a response
            LOG_F(INFO, "Receive response: {}", line);
            std::unique_lock lock(m_mutex_);
            m_response_ = j;
            m_cond_.notify_one();
        } else {
            handleEvent(j);
        }
    }
}

static auto makeJsonrpc(const std::string &method,
                        const json &params) -> std::string {
    json req;

    req["method"] = method;
    req["id"] = 1;

    if (!params.is_null()) {
        if (params.is_array() || params.is_object()) {
            req["params"] = params;
        } else {
            // single non-null parameter
            json ary;
            ary.push_back(params);
            req["params"] = ary;
        }
    }

    std::ostringstream os;
    os << req.dump();
    return os.str();
}

static auto failed(const json &res) -> bool { return res.contains("error"); }

auto Guider::Impl::call(const std::string &method, const json &params) -> json {
    std::string s = makeJsonrpc(method, params);
    LOG_F(INFO, "Send message: {}", s);
    // send request
    m_conn_.writeLine(s);

    // wait for response

    std::unique_lock lock(m_mutex_);
    while (m_response_.is_null()) {
        m_cond_.wait(lock);
    }

    json response;
    m_response_.swap(response);

    if (failed(response)) {
        error = response["error"]["message"].get<std::string>();
    }

    return response;
}

auto Guider::Impl::call(const std::string &method) -> json {
    return call(method, json());
}

static auto settleParam(double settlePixels, double settleTime,
                        double settleTimeout) -> json {
    json s;
    s["pixels"] = settlePixels;
    s["time"] = settleTime;
    s["timeout"] = settleTimeout;
    return s;
}

auto Guider::Impl::guide(double settlePixels, double settleTime,
                         double settleTimeout) -> bool {
    {
        std::unique_ptr<SettleProgress> s(new SettleProgress());
        s->done = false;
        s->distance = 0.;
        s->settlePx = settlePixels;
        s->time = 0.;
        s->settleTime = settleTime;
        s->status = 0;
        std::unique_lock lock(m_mutex_);
        if (mSettle_ && !mSettle_->done) {
            error = "cannot guide while settling";
            return false;
        }
        mSettle_.swap(s);
    }

    json params;
    params.push_back(settleParam(settlePixels, settleTime, settleTimeout));
    params.push_back(false);  // don't force calibration

    json res = call("guide", params);

    if (!failed(res)) {
        settle_px_ = settlePixels;
        return true;
    }

    // failed - remove the settle state
    std::unique_ptr<SettleProgress> s;
    {
        std::unique_lock lock(m_mutex_);
        mSettle_.swap(s);
    }

    return false;
}

auto Guider::Impl::dither(double ditherPixels, double settlePixels,
                          double settleTime, double settleTimeout) -> bool {
    {
        std::unique_ptr<SettleProgress> s(new SettleProgress());
        s->done = false;
        s->distance = ditherPixels;
        s->settlePx = settlePixels;
        s->time = 0.;
        s->settleTime = settleTime;
        s->status = 0;
        std::unique_lock lock(m_mutex_);
        if (mSettle_ && !mSettle_->done) {
            error = "cannot dither while settling";
            return false;
        }
        mSettle_.swap(s);
    }

    json params;
    params.push_back(ditherPixels);
    params.push_back(false);
    params.push_back(settleParam(settlePixels, settleTime, settleTimeout));

    json ret = call("dither", params);

    if (!failed(ret)) {
        settle_px_ = settlePixels;
        return true;
    }

    // call failed - remove the settle state
    std::unique_ptr<SettleProgress> s;
    {
        std::unique_lock lock(m_mutex_);
        mSettle_.swap(s);
    }

    return false;
}

auto Guider::Impl::isSettling(bool *ret) -> bool {
    {
        std::unique_lock lock(m_mutex_);
        if (mSettle_) {
            *ret = true;
            return true;
        }
    }

    // for app init, initialize the settle state to a consistent value
    // as if Guide had been called

    json res = call("get_settling");
    if (failed(res)) {
        return false;
    }

    bool val = res["result"].get<bool>();

    if (val) {
        std::unique_ptr<SettleProgress> s(new SettleProgress());
        s->done = false;
        s->distance = -1.;
        s->settlePx = 0.;
        s->time = 0.;
        s->settleTime = 0.;
        s->status = 0;
        std::unique_lock lock(m_mutex_);
        if (!mSettle_) {
            mSettle_.swap(s);
        }
    }

    *ret = val;
    return true;
}

auto Guider::Impl::checkSettling(SettleProgress *s) -> bool {
    std::unique_ptr<SettleProgress> tmp;
    std::unique_lock lock(m_mutex_);

    if (!mSettle_) {
        error = "not settling";
        return false;
    }

    if (mSettle_->done) {
        mSettle_.swap(tmp);
        lock.unlock();
        // settle is done
        s->done = true;
        s->status = tmp->status;
        s->error = std::move(tmp->error);
        return true;
    }

    // settle in progress
    s->done = false;
    s->distance = mSettle_->distance;
    s->settlePx = settle_px_;
    s->time = mSettle_->time;
    s->settleTime = mSettle_->settleTime;

    return true;
}

auto Guider::Impl::getStats(GuideStats *stats) -> bool {
    {
        std::unique_lock lock(m_mutex_);
        *stats = Stats_;
    }
    stats->rmsTot = hypot(stats->rmsRa, stats->rmsDec);
    return true;
}

auto Guider::Impl::stopCapture(unsigned int timeoutSeconds) -> bool {
    json res = call("stop_capture");
    if (failed(res)) {
        return false;
    }

    for (unsigned int i = 0; i < timeoutSeconds; i++) {
        std::string appstate;
        {
            std::unique_lock lock(m_mutex_);
            appstate = AppState_;
        }
        LOG_F(INFO, "StopCapture: AppState = {}", appstate);
        if (appstate == "Stopped") {
            return true;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    LOG_F(ERROR, "StopCapture: timed-out waiting for stopped");

    // hack! workaround bug where PHD2 sends a GuideStep after stop request and
    // fails to send GuidingStopped
    res = call("get_app_state");
    if (failed(res)) {
        return false;
    }

    std::string st = res["result"].get<std::string>();

    {
        std::unique_lock lock(m_mutex_);
        AppState_ = st;
    }

    if (st == "Stopped") {
        return true;
    }
    // end workaround

    error = std::format("guider did not stop capture after {} seconds!",
                        timeoutSeconds);
    return false;
}

auto Guider::Impl::loop(unsigned int timeoutSeconds) -> bool {
    {  // already looping?
        std::unique_lock lock(m_mutex_);
        if (AppState_ == "Looping") {
            return true;
        }
    }

    json res = call("get_exposure");
    if (failed(res)) {
        return false;
    }

    int exp = res["result"].get<int>();

    res = call("loop");
    if (failed(res)) {
        return false;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(exp));

    for (unsigned int i = 0; i < timeoutSeconds; i++) {
        {
            std::unique_lock lock(m_mutex_);
            if (AppState_ == "Looping") {
                return true;
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    error = "timed-out waiting for guiding to start looping";
    return false;
}

auto Guider::Impl::pixelScale(double *result) -> bool {
    json res = call("get_pixel_scale");

    if (failed(res)) {
        return false;
    }

    *result = res["result"].get<double>();
    return true;
}

auto Guider::Impl::getEquipmentProfiles(std::vector<std::string> *profiles)
    -> bool {
    json res = call("get_profiles");
    if (failed(res)) {
        return false;
    }

    profiles->clear();

    json ary = res["result"];
    for (const auto &p : ary) {
        std::string name = p["name"].get<std::string>();
        profiles->push_back(name);
    }

    return true;
}

auto Guider::Impl::connectEquipment(const char *profileName) -> bool {
    json res = call("get_profile");
    if (failed(res)) {
        return false;
    }
    json prof = res["result"];

    std::string profname(profileName);

    if (prof["name"].get<std::string>() != profname) {
        res = call("get_profiles");
        if (failed(res)) {
            return false;
        }
        json profiles = res["result"];
        int profid = -1;
        for (auto &profile : profiles) {
            std::string name = profile["name"].get<std::string>();
            LOG_F(INFO, "found profile {}", name);
            if (name == profname) {
                profid = profile.value("id", -1);
                LOG_F(INFO, "found profid {}", profid);
                break;
            }
        }
        if (profid == -1) {
            error = "invalid phd2 profile name: " + profname;
            return false;
        }

        if (!stopCapture()) {
            return false;
        }
        res = call("set_connected", json(false));
        if (failed(res)) {
            return false;
        }
        res = call("set_profile", json(profid));
        if (failed(res)) {
            return false;
        }
    }

    res = call("set_connected", json(true));
    return !failed(res);
}

auto Guider::Impl::disconnectEquipment() -> bool {
    if (!stopCapture()) {
        return false;
    }

    json res = call("set_connected", json(false));
    return !failed(res);
}

auto Guider::Impl::getStatus(std::string *appState, double *avgDist) -> bool {
    std::unique_lock lock(m_mutex_);
    *appState = AppState_;
    *avgDist = AvgDist_;
    return true;
}

auto Guider::Impl::isGuiding(bool *result) -> bool {
    std::string st;
    double dist;
    if (!getStatus(&st, &dist)) {
        return false;
    }
    *result = isGuiding_(st);
    return true;
}

auto Guider::Impl::pause() -> bool {
    return !failed(call("set_paused", json(true)));
}

auto Guider::Impl::unpause() -> bool {
    return !failed(call("set_paused", json(false)));
}

auto Guider::Impl::saveImage(std::string *filename) -> bool {
    json res = call("save_image");
    if (failed(res)) {
        return false;
    }
    *filename = res["result"]["filename"].get<std::string>();
    return true;
}
