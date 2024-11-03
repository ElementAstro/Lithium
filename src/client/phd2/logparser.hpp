#ifndef LITHIUM_CLIENT_PHD2_LOGPARSER_HPP
#define LITHIUM_CLIENT_PHD2_LOGPARSER_HPP

#include <cmath>
#include <cstring>
#include <ctime>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "atom/macro.hpp"

namespace lithium::client::phd2 {

/**
 * @enum WhichMount
 * @brief Enum representing the type of mount.
 */
enum class WhichMount { MOUNT, AO };

/**
 * @struct GuideEntry
 * @brief Structure representing a guide entry.
 */
struct GuideEntry {
    int frame{};       ///< Frame number.
    float dt{};        ///< Time delta.
    WhichMount mount;  ///< Type of mount.
    bool included{};   ///< Whether the entry is included.
    bool guiding{};    ///< Whether guiding is active.
    float dx{};        ///< Delta x.
    float dy{};        ///< Delta y.
    float raraw{};     ///< Raw RA value.
    float decraw{};    ///< Raw DEC value.
    float raguide{};   ///< Guide RA value.
    float decguide{};  ///< Guide DEC value.
    int radur{};       ///< RA duration or xstep.
    int decdur{};      ///< DEC duration or ystep.
    int mass{};        ///< Mass.
    float snr{};       ///< Signal-to-noise ratio.
    int err{};         ///< Error code.
    std::string info;  ///< Additional information.
} ATOM_ALIGNAS(128);

/**
 * @brief Checks if a star was found based on the error code.
 * @param err Error code.
 * @return True if the star was found, false otherwise.
 */
inline constexpr bool starWasFound(int err) { return err == 0 || err == 1; }

/**
 * @struct InfoEntry
 * @brief Structure representing an information entry.
 */
struct InfoEntry {
    int idx{};         ///< Index of the subsequent frame.
    int repeats{};     ///< Number of repeats.
    std::string info;  ///< Additional information.
} ATOM_ALIGNAS(64);

/**
 * @enum CalDirection
 * @brief Enum representing the calibration direction.
 */
enum class CalDirection {
    WEST,
    EAST,
    BACKLASH,
    NORTH,
    SOUTH,
};

/**
 * @struct CalibrationEntry
 * @brief Structure representing a calibration entry.
 */
struct CalibrationEntry {
    CalDirection direction;  ///< Calibration direction.
    int step;                ///< Step number.
    float dx;                ///< Delta x.
    float dy;                ///< Delta y.
} ATOM_ALIGNAS(16);

/**
 * @struct Limits
 * @brief Structure representing the limits.
 */
struct Limits {
    double minMo{};   ///< Minimum motion.
    double maxDur{};  ///< Maximum duration.
} ATOM_ALIGNAS(16);

/**
 * @struct Mount
 * @brief Structure representing a mount.
 */
struct Mount {
    bool isValid = false;    ///< Whether the mount is valid.
    double xRate = 1.0;      ///< X rate.
    double yRate = 1.0;      ///< Y rate.
    double xAngle = 0.0;     ///< X angle.
    double yAngle = M_PI_2;  ///< Y angle.
    Limits xlim;             ///< X limits.
    Limits ylim;             ///< Y limits.
} ATOM_ALIGNAS(128);

/**
 * @struct GraphInfo
 * @brief Structure representing graph information.
 */
struct GraphInfo {
    double hscale{};  ///< Horizontal scale (pixels per entry).
    double vscale{};  ///< Vertical scale.
    double maxOfs{};  ///< Maximum offset.
    double maxSnr{};  ///< Maximum signal-to-noise ratio.
    int maxMass{};    ///< Maximum mass.
    int xofs{};       ///< X offset relative to the 0th entry.
    int yofs{};       ///< Y offset.
    int xmin{};       ///< Minimum x value.
    int xmax{};       ///< Maximum x value.
    int width = 0;    ///< Width.
    double i0{};      ///< Initial value 0.
    double i1{};      ///< Initial value 1.

    /**
     * @brief Checks if the graph information is valid.
     * @return True if valid, false otherwise.
     */
    [[nodiscard]] constexpr bool isValid() const { return width != 0; }
} ATOM_ALIGNAS(128);

/**
 * @enum SectionType
 * @brief Enum representing the type of log section.
 */
enum class SectionType { CALIBRATION_SECTION, GUIDING_SECTION };

/**
 * @struct LogSectionLoc
 * @brief Structure representing the location of a log section.
 */
struct LogSectionLoc {
    SectionType type;  ///< Type of section.
    int idx;           ///< Index.

    /**
     * @brief Constructor for LogSectionLoc.
     * @param t Type of section.
     * @param ix Index.
     */
    LogSectionLoc(SectionType t, int ix) : type(t), idx(ix) {}
} ATOM_ALIGNAS(8);

/**
 * @struct LogSection
 * @brief Structure representing a log section.
 */
struct LogSection {
    std::string date;              ///< Date of the log section.
    std::time_t starts{};          ///< Start time.
    std::vector<std::string> hdr;  ///< Header information.

    /**
     * @brief Constructor for LogSection.
     * @param dt Date of the log section.
     */
    explicit LogSection(std::string dt) : date(std::move(dt)) {}
} ATOM_ALIGNAS(64);

/**
 * @struct GuideSession
 * @brief Structure representing a guide session.
 */
struct GuideSession : LogSection {
    using EntryVec = std::vector<GuideEntry>;
    using InfoVec = std::vector<InfoEntry>;

    double duration{};        ///< Duration of the session.
    double pixelScale = 1.0;  ///< Pixel scale.
    double declination{};     ///< Declination.
    EntryVec entries;         ///< Guide entries.
    InfoVec infos;            ///< Information entries.
    Mount ao;                 ///< AO mount.
    Mount mount;              ///< Mount.

    // Calculated statistics
    double rmsRa{};            ///< RMS RA.
    double rmsDec{};           ///< RMS DEC.
    double avgRa{}, avgDec{};  ///< Average RA and DEC.
    double theta{};            ///< Theta.
    double lx, ly;             ///< Lx and Ly.
    double elongation{};       ///< Elongation.
    double peakRa{};           ///< Peak RA.
    double peakDec{};          ///< Peak DEC.
    double driftRa{};          ///< Drift RA.
    double driftDec{};         ///< Drift DEC.
    double paerr{};            ///< PA error.

    GraphInfo mGinfo;  ///< Graph information.

    using LogSection::LogSection;

    /**
     * @brief Calculates the statistics for the guide session.
     */
    void calcStats();
} ATOM_PACKED;

/**
 * @struct CalDisplay
 * @brief Structure representing the calibration display.
 */
struct CalDisplay {
    bool valid = false;  ///< Whether the display is valid.
    int xofs = 0;        ///< X offset.
    int yofs = 0;        ///< Y offset.
    double scale = 1.0;  ///< Scale.
    double minScale{};   ///< Minimum scale.
    int firstWest{}, lastWest{}, firstNorth{},
        lastNorth{};  ///< Calibration steps.
} ATOM_ALIGNAS(64);

/**
 * @struct Calibration
 * @brief Structure representing a calibration.
 */
struct Calibration : LogSection {
    using EntryVec = std::vector<CalibrationEntry>;

    WhichMount device = WhichMount::MOUNT;  ///< Type of device.
    EntryVec entries;                       ///< Calibration entries.
    CalDisplay display;                     ///< Calibration display.

    using LogSection::LogSection;
} ATOM_ALIGNAS(128);

/**
 * @struct GuideLog
 * @brief Structure representing a guide log.
 */
struct GuideLog {
    using SessionVec = std::vector<GuideSession>;
    using CalibrationVec = std::vector<Calibration>;
    using SectionLocVec = std::vector<LogSectionLoc>;

    std::string phdVersion;       ///< PHD version.
    SessionVec sessions;          ///< Guide sessions.
    CalibrationVec calibrations;  ///< Calibrations.
    SectionLocVec sections;       ///< Log sections.
} ATOM_ALIGNAS(128);

/**
 * @class LogParser
 * @brief Class for parsing logs.
 */
class LogParser {
public:
    /**
     * @brief Parses the input stream and populates the guide log.
     * @param input_stream Input stream to parse.
     * @param log Guide log to populate.
     * @return True if parsing was successful, false otherwise.
     */
    static auto parse(std::istream& input_stream, GuideLog& log) -> bool;
};

}  // namespace lithium::client::phd2

#endif  // LITHIUM_CLIENT_PHD2_LOGPARSER_HPP