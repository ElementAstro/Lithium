#include "logparser.hpp"

#include <array>
#include <iomanip>
#include <optional>
#include <regex>

#include "atom/utils/string.hpp"

namespace lithium::client::phd2 {
constexpr std::string_view VERSION_PREFIX("PHD2 version ");
constexpr std::string_view GUIDING_BEGINS("Guiding Begins at ");
constexpr std::string_view GUIDING_HEADING("Frame,Time,mount");
constexpr std::string_view MOUNT_KEY("Mount = ");
constexpr std::string_view AO_KEY("AO = ");
constexpr std::string_view PX_SCALE("Pixel scale = ");
constexpr std::string_view GUIDING_ENDS("Guiding Ends");
constexpr std::string_view INFO_KEY("INFO: ");
constexpr std::string_view CALIBRATION_BEGINS("Calibration Begins at ");
constexpr std::string_view CALIBRATION_HEADING("Direction,Step,dx,dy,x,y,Dist");
constexpr std::string_view CALIBRATION_ENDS("Calibration complete");
constexpr std::string_view XALGO("X guide algorithm = ");
constexpr std::string_view YALGO("Y guide algorithm = ");
constexpr std::string_view MINMOVE("Minimum move = ");

auto beforeLast(const std::string& s, char ch) -> std::string {
    if (auto pos = s.rfind(ch); pos != std::string::npos) {
        return s.substr(0, pos);
    }
    return s;
}

auto isEmpty(const std::string& s) -> bool {
    return s.find_first_not_of(" \t\r\n") == std::string::npos;
}

auto parseEntry(const std::string& line, GuideEntry& entry) -> bool {
    std::string_view strView = line;
    std::string_view delims = ",";

    auto tokenOpt = atom::utils::nstrtok(strView, delims);
    if (!tokenOpt) {
        return false;
    }
    long longValue;
    double doubleValue;
    try {
        longValue = atom::utils::stol(tokenOpt.value());
    } catch (const std::invalid_argument&) {
        return false;
    }
    entry.frame = static_cast<int>(longValue);

    tokenOpt = atom::utils::nstrtok(strView, delims);
    try {
        doubleValue = atom::utils::stod(tokenOpt.value());
    } catch (const std::invalid_argument&) {
        return false;
    } catch (const std::bad_optional_access&) {
        return false;
    }
    entry.dt = static_cast<float>(doubleValue);

    tokenOpt = atom::utils::nstrtok(strView, delims);
    if (!tokenOpt) {
        return false;
    }
    entry.mount =
        (tokenOpt.value() == "\"Mount\"") ? WhichMount::MOUNT : WhichMount::AO;

    auto parseFloatField = [&](float& field) -> bool {
        tokenOpt = atom::utils::nstrtok(strView, delims);
        if (tokenOpt && !tokenOpt->empty()) {
            try {
                field = static_cast<float>(atom::utils::stod(tokenOpt.value()));
            } catch (const std::invalid_argument&) {
                return false;
            }
            field = static_cast<float>(doubleValue);
        } else {
            field = 0.F;
        }
        return true;
    };

    auto parseIntField = [&](int& field) -> bool {
        tokenOpt = atom::utils::nstrtok(strView, delims);
        if (tokenOpt && !tokenOpt->empty()) {
            try {
                field = static_cast<int>(atom::utils::stol(tokenOpt.value()));
            } catch (const std::invalid_argument&) {
                return false;
            }
        } else {
            field = 0;
        }
        return true;
    };

    if (!(parseFloatField(entry.dx) && parseFloatField(entry.dy) &&
          parseFloatField(entry.raraw) && parseFloatField(entry.decraw) &&
          parseFloatField(entry.raguide) && parseFloatField(entry.decguide))) {
        return false;
    }

    if (!parseIntField(entry.radur)) {
        return false;
    }

    tokenOpt = atom::utils::nstrtok(strView, delims);
    if (tokenOpt && !tokenOpt->empty()) {
        if (tokenOpt->front() == 'W') {
            entry.radur = -entry.radur;
        } else if (tokenOpt->front() != 'E') {
            return false;
        }
    }

    if (!parseIntField(entry.decdur)) {
        return false;
    }

    tokenOpt = atom::utils::nstrtok(strView, delims);
    if (tokenOpt && !tokenOpt->empty()) {
        if (tokenOpt->front() == 'S') {
            entry.decdur = -entry.decdur;
        } else if (tokenOpt->front() != 'N') {
            return false;
        }
    }

    if (!parseIntField(entry.mass)) {
        return false;
    }
    if (!parseFloatField(entry.snr)) {
        return false;
    }
    if (!parseIntField(entry.err)) {
        return false;
    }

    tokenOpt = atom::utils::nstrtok(strView, delims);
    if (tokenOpt && !tokenOpt->empty()) {
        entry.info = tokenOpt.value();
        if (entry.info.size() >= 2) {
            entry.info = entry.info.substr(1, entry.info.size() - 2);
        }
    }

    return true;
}

// 解析信息条目
void parseInfo(const std::string& ln, GuideSession* s) {
    InfoEntry e;
    e.idx = static_cast<int>(s->entries.size());
    e.repeats = 1;
    e.info = ln.substr(INFO_KEY.size());

    if (e.info.starts_with("SETTLING STATE CHANGE, "))
        e.info = e.info.substr(23);
    else if (e.info.starts_with("Guiding parameter change, "))
        e.info = e.info.substr(26);

    if (e.info.starts_with("DITHER")) {
        if (auto pos = e.info.find(", new lock pos"); pos != std::string::npos)
            e.info = e.info.substr(0, pos);
    }

    if (e.info.ends_with("00")) {
        std::regex re("\\.[0-9]+?(0+)$");
        std::smatch match;
        if (std::regex_search(e.info, match, re) &&
            match.position(1) != std::string::npos)
            e.info = e.info.substr(0, match.position(1));
    }

    if (!s->infos.empty()) {
        auto& prev = s->infos.back();
        if (e.info == prev.info && e.idx >= prev.idx &&
            e.idx <= (prev.idx + prev.repeats)) {
            ++prev.repeats;
            return;
        }

        if (prev.idx == e.idx) {
            if (prev.info.find('=') != std::string::npos &&
                e.info.starts_with(beforeLast(prev.info, '='))) {
                prev = e;
                return;
            }
            if (e.info.starts_with("DITHER") &&
                prev.info.starts_with("SET LOCK POS")) {
                prev = e;
                return;
            }
        }
    }

    s->infos.push_back(e);
}

// 解析校准条目
auto parseCalibration(const std::string& line,
                      CalibrationEntry& entry) -> bool {
    std::string_view strView = line;
    std::string_view delims = ",";

    auto tokenOpt = atom::utils::nstrtok(strView, delims);
    if (!tokenOpt) {
        return false;
    }

    std::string token = std::string(tokenOpt.value());
    if (token == "West" || token == "Left") {
        entry.direction = CalDirection::WEST;
    } else if (token == "East") {
        entry.direction = CalDirection::EAST;
    } else if (token == "Backlash") {
        entry.direction = CalDirection::BACKLASH;
    } else if (token == "North" || token == "Up") {
        entry.direction = CalDirection::NORTH;
    } else if (token == "South") {
        entry.direction = CalDirection::SOUTH;
    } else {
        return false;
    }

    tokenOpt = atom::utils::nstrtok(strView, delims);
    if (!tokenOpt) {
        return false;
    }

    long longValue;
    entry.step = static_cast<int>(atom::utils::stod(tokenOpt.value()));

    double doubleValue;
    tokenOpt = atom::utils::nstrtok(strView, delims);
    entry.dx = static_cast<float>(atom::utils::stod(tokenOpt.value()));

    tokenOpt = atom::utils::nstrtok(strView, delims);
    entry.dx = static_cast<float>(atom::utils::stod(tokenOpt.value()));

    return true;
}

// 去除字符串末尾的空白字符
void rtrim(std::string& line) {
    if (auto pos = line.find_last_not_of(" \r\n\t");
        pos != std::string::npos && pos + 1 < line.size()) {
        line.erase(pos + 1);
    }
}

// 检查会话条目的时间是否单调递增
constexpr auto isMonotonic(const GuideSession& session) -> bool {
    const auto& entries = session.entries;
    return std::is_sorted(
        entries.begin(), entries.end(),
        [](const GuideEntry& a, const GuideEntry& b) { return a.dt < b.dt; });
}

// 插入信息条目
void insertInfo(GuideSession& session,
                std::vector<GuideEntry>::iterator entryPos,
                const std::string& info) {
    auto pos = std::find_if(
        session.infos.begin(), session.infos.end(), [&](const InfoEntry& e) {
            return session.entries[e.idx].frame >= entryPos->frame;
        });
    int idx =
        static_cast<int>(std::distance(session.entries.begin(), entryPos));
    InfoEntry infoEntry{idx, 1, info};
    session.infos.insert(pos, infoEntry);
}

// 校正非单调时间
void fixupNonMonotonic(GuideSession& session) {
    if (isMonotonic(session)) {
        return;
    }

    std::vector<double> intervals;
    for (auto it = session.entries.begin() + 1; it != session.entries.end();
         ++it) {
        if (auto interval = it->dt - (std::prev(it)->dt); interval > 0.0) {
            intervals.push_back(interval);
        }
    }

    if (intervals.empty()) {
        return;
    }

    std::nth_element(intervals.begin(),
                     intervals.begin() + intervals.size() / 2, intervals.end());
    double median = intervals[intervals.size() / 2];
    double correction = 0.0;

    for (auto it = session.entries.begin() + 1; it != session.entries.end();
         ++it) {
        double interval = it->dt + correction - std::prev(it)->dt;
        if (interval <= 0.0) {
            correction += median - interval;
            insertInfo(session, it, "Timestamp jumped backwards");
        }
        it->dt += static_cast<float>(correction);
    }
}

// 校正日志中的所有会话
void fixupNonMonotonic(GuideLog& log) {
    for (const auto& section : log.sections) {
        if (section.type == SectionType::GUIDING_SECTION) {
            fixupNonMonotonic(log.sessions[section.idx]);
        }
    }
}

// 解析Mount信息
void parseMount(const std::string& line, Mount& mount) {
    mount.isValid = true;
    auto parseField = [&](const std::string& key, double& field, double dflt) {
        if (auto pos = line.find(key); pos != std::string::npos) {
            std::string valueStr = line.substr(pos + key.size());
            field = atom::utils::stod(valueStr);
        }
    };

    parseField(", xAngle = ", mount.xAngle, 0.0);
    parseField(", xRate = ", mount.xRate, 1.0);
    parseField(", yAngle = ", mount.yAngle, M_PI_2);
    parseField(", yRate = ", mount.yRate, 1.0);

    if (mount.xRate < 0.05) {
        mount.xRate *= 1000.0;
    }
    if (mount.yRate < 0.05) {
        mount.yRate *= 1000.0;
    }
}

// 获取最小移动值
void getMinMo(const std::string& line, Limits* limits) {
    if (auto pos = line.find(MINMOVE); pos != std::string::npos) {
        try {
            limits->minMo = std::stod(line.c_str() + pos + MINMOVE.size());
        } catch (const std::invalid_argument&) {
            limits->minMo = 0.0;
        }
    }
}

// 解析日志
auto LogParser::parse(std::istream& input_stream, GuideLog& log) -> bool {
    log = GuideLog{};
    enum class State { SKIP, GUIDING_HDR, GUIDING, CAL_HDR, CALIBRATING };
    State state = State::SKIP;
    enum class HdrState { GLOBAL, AO, MOUNT };
    HdrState hdrState;
    char axis = ' ';
    GuideSession* session = nullptr;
    Calibration* calibration = nullptr;
    unsigned int lineNumber = 0;
    bool mountEnabled = false;

    std::string line;
    while (std::getline(input_stream, line)) {
        ++lineNumber;
        if (lineNumber % 200 == 0) { /* 可添加类似Yield的逻辑 */
        }

        rtrim(line);
        if (line.size() > 26) {
            line = line.substr(26);
        } else {
            line.clear();
        }

        switch (state) {
            case State::SKIP:
                if (line.starts_with(GUIDING_BEGINS)) {
                    state = State::GUIDING_HDR;
                    hdrState = HdrState::GLOBAL;
                    mountEnabled = false;
                    std::string dateStr = line.substr(GUIDING_BEGINS.size());
                    log.sessions.emplace_back(dateStr);
                    log.sections.emplace_back(
                        SectionType::GUIDING_SECTION,
                        static_cast<int>(log.sessions.size() - 1));
                    session = &log.sessions.back();
                    std::tm tm = {};
                    std::istringstream ss(dateStr);
                    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
                    if (!ss.fail()) {
                        session->starts = std::mktime(&tm);
                    }
                    break;
                }
                if (line.starts_with(CALIBRATION_BEGINS)) {
                    state = State::CAL_HDR;
                    std::string dateStr =
                        line.substr(CALIBRATION_BEGINS.size());
                    log.calibrations.emplace_back(dateStr);
                    log.sections.emplace_back(
                        SectionType::CALIBRATION_SECTION,
                        static_cast<int>(log.calibrations.size() - 1));
                    calibration = &log.calibrations.back();
                    std::tm tm = {};
                    std::istringstream ss(dateStr);
                    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
                    if (!ss.fail()) {
                        calibration->starts = std::mktime(&tm);
                    }
                    break;
                }
                if (line.starts_with(VERSION_PREFIX)) {
                    auto end =
                        line.find(", Log version ", VERSION_PREFIX.size());
                    if (end == std::string::npos) {
                        end = line.find_first_of(" \t\r\n",
                                                 VERSION_PREFIX.size());
                    }
                    if (end == std::string::npos) {
                        end = line.size();
                    }
                    log.phdVersion = line.substr(VERSION_PREFIX.size(),
                                                 end - VERSION_PREFIX.size());
                }
                break;

            case State::GUIDING_HDR:
                if (line.starts_with(GUIDING_HEADING)) {
                    state = State::GUIDING;
                    break;
                }
                if (line.starts_with(MOUNT_KEY)) {
                    parseMount(line, session->mount);
                    hdrState = HdrState::MOUNT;
                    if (auto pos = line.find(", guiding enabled, ");
                        pos != std::string::npos) {
                        mountEnabled = (line.compare(pos + 21, 4, "true") == 0);
                    }
                } else if (line.starts_with(AO_KEY)) {
                    parseMount(line, session->ao);
                    hdrState = HdrState::AO;
                } else if (line.starts_with(PX_SCALE)) {
                    auto pos = line.find("Pixel scale = ");
                    if (pos != std::string::npos) {
                        std::string sVal = line.substr(pos + 14);
                        try {
                            session->pixelScale = std::stod(sVal);
                        } catch (const std::invalid_argument&) {
                            session->pixelScale = 1.0;
                        }
                    }
                } else if (line.starts_with(XALGO)) {
                    getMinMo(line, (hdrState == HdrState::MOUNT)
                                       ? &session->mount.xlim
                                       : &session->ao.xlim);
                    axis = 'X';
                } else if (line.starts_with(YALGO)) {
                    getMinMo(line, (hdrState == HdrState::MOUNT)
                                       ? &session->mount.ylim
                                       : &session->ao.ylim);
                    axis = 'Y';
                } else if (line.starts_with(MINMOVE)) {
                    if (axis == 'X') {
                        getMinMo(line, (hdrState == HdrState::MOUNT)
                                           ? &session->mount.xlim
                                           : &session->ao.xlim);
                    } else if (axis == 'Y') {
                        getMinMo(line, (hdrState == HdrState::MOUNT)
                                           ? &session->mount.ylim
                                           : &session->ao.ylim);
                    }
                } else {
                    if (auto pos = line.find("Max RA duration = ");
                        pos != std::string::npos) {
                        auto& mnt = (hdrState == HdrState::MOUNT)
                                        ? session->mount
                                        : session->ao;
                        std::string sRa = line.substr(pos + 19);
                        try {
                            mnt.xlim.maxDur = std::stod(sRa);
                        } catch (const std::invalid_argument&) {
                            mnt.xlim.maxDur = 0.0;
                        }
                    }
                    if (auto pos = line.find("Max DEC duration = ");
                        pos != std::string::npos) {
                        auto& mnt = (hdrState == HdrState::MOUNT)
                                        ? session->mount
                                        : session->ao;
                        std::string sDec = line.substr(pos + 19);
                        try {
                            mnt.ylim.maxDur = std::stod(sDec);
                        } catch (const std::invalid_argument&) {
                            mnt.ylim.maxDur = 0.0;
                        }
                    }
                    if (line.starts_with("RA = ")) {
                        auto posHr = line.find(" hr, Dec = ");
                        if (posHr != std::string::npos) {
                            std::string sDec = line.substr(posHr + 10);
                            double dec;
                            try {
                                session->declination = dec * M_PI / 180.0;
                            } catch (const std::invalid_argument&) {
                                session->declination = 0.0;
                            }
                            session->declination = dec * M_PI / 180.0;
                        }
                    }
                }
                session->hdr.push_back(line);
                break;

            case State::GUIDING:
                if (isEmpty(line) || line.starts_with(GUIDING_ENDS)) {
                    if (!session->entries.empty()) {
                        session->duration = session->entries.back().dt;
                    }
                    session = nullptr;
                    state = State::SKIP;
                    break;
                }
                if (!line.empty() && (std::isdigit(line[0]) != 0)) {
                    GuideEntry entry;
                    if (parseEntry(line, entry)) {
                        if (!starWasFound(entry.err)) {
                            entry.included = false;
                            if (entry.info.empty()) {
                                entry.info = "Frame dropped";
                            }
                            parseInfo("INFO: " + entry.info, session);
                        } else {
                            entry.included = true;
                        }
                        entry.guiding = mountEnabled;
                        session->entries.push_back(entry);
                    }
                    break;
                }
                if (line.starts_with(INFO_KEY)) {
                    parseInfo(line, session);
                    if (auto pos = line.find("MountGuidingEnabled = ");
                        pos != std::string::npos) {
                        mountEnabled = (line.compare(pos + 22, 4, "true") == 0);
                    }
                }
                break;

            case State::CAL_HDR:
                if (line.starts_with(CALIBRATION_HEADING)) {
                    state = State::CALIBRATING;
                    break;
                }
                calibration->hdr.push_back(line);
                break;

            case State::CALIBRATING:
                if (isEmpty(line) || line.starts_with(CALIBRATION_ENDS)) {
                    state = State::SKIP;
                    break;
                }
                {
                    constexpr std::array<const char*, 7> KEYS = {
                        "West,",  "East,", "Backlash,", "North,",
                        "South,", "Left,", "Up,"};
                    bool isCalEntry = std::any_of(
                        KEYS.begin(), KEYS.end(),
                        [&](const auto& key) { return line.starts_with(key); });
                    if (isCalEntry) {
                        CalibrationEntry entry{};
                        if (parseCalibration(line, entry)) {
                            calibration->entries.push_back(entry);
                        }
                    } else {
                        calibration->hdr.push_back(line);
                    }
                }
                break;
        }
    }

    if ((session != nullptr) && !session->entries.empty()) {
        session->duration = session->entries.back().dt;
    }

    fixupNonMonotonic(log);
    return true;
}

void printGuideLog(const GuideLog& log) {
    std::cout << "PHD Version: " << log.phdVersion << "\n\n";

    for (const auto& session : log.sessions) {
        std::cout << "Pixel Scale: " << session.pixelScale << "\n";
        std::cout << "Mount: " << (session.mount.isValid ? "Valid" : "Invalid")
                  << "\n";
        std::cout << "AO: " << (session.ao.isValid ? "Valid" : "Invalid")
                  << "\n";

        std::cout << "Entries:\n";
        for (const auto& entry : session.entries) {
            std::cout << "  Frame: " << entry.frame << ", Time: " << entry.dt
                      << ", Mount: "
                      << (entry.mount == WhichMount::MOUNT ? "MOUNT" : "AO")
                      << ", dx: " << entry.dx << ", dy: " << entry.dy
                      << ", raraw: " << entry.raraw
                      << ", decraw: " << entry.decraw
                      << ", raguide: " << entry.raguide
                      << ", decguide: " << entry.decguide
                      << ", radur: " << entry.radur
                      << ", decdur: " << entry.decdur
                      << ", mass: " << entry.mass << ", snr: " << entry.snr
                      << ", err: " << entry.err << ", info: " << entry.info
                      << "\n";
        }

        std::cout << "Infos:\n";
        for (const auto& info : session.infos) {
            std::cout << "  Index: " << info.idx
                      << ", Repeats: " << info.repeats
                      << ", Info: " << info.info << "\n";
        }

        std::cout << "\n";
    }

    for (const auto& calibration : log.calibrations) {
        std::cout << "Entries:\n";
        for (const auto& entry : calibration.entries) {
            std::cout << "  Direction: ";
            switch (entry.direction) {
                case CalDirection::WEST:
                    std::cout << "West";
                    break;
                case CalDirection::EAST:
                    std::cout << "East";
                    break;
                case CalDirection::BACKLASH:
                    std::cout << "Backlash";
                    break;
                case CalDirection::NORTH:
                    std::cout << "North";
                    break;
                case CalDirection::SOUTH:
                    std::cout << "South";
                    break;
            }
            std::cout << ", Step: " << entry.step << ", dx: " << entry.dx
                      << ", dy: " << entry.dy << "\n";
        }
        std::cout << "\n";
    }
}
}  // namespace lithium::client::phd2
