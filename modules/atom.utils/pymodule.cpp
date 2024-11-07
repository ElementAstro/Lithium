#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "atom/utils/aes.hpp"
#include "atom/utils/argsview.hpp"
#include "atom/utils/bit.hpp"
#include "atom/utils/difflib.hpp"
#include "atom/utils/error_stack.hpp"
#include "atom/utils/lcg.hpp"
#include "atom/utils/qdatetime.hpp"
#include "atom/utils/qprocess.hpp"
#include "atom/utils/qtimer.hpp"
#include "atom/utils/qtimezone.hpp"
#include "atom/utils/random.hpp"
#include "atom/utils/time.hpp"
#include "atom/utils/uuid.hpp"
#include "atom/utils/xml.hpp"

namespace py = pybind11;
using namespace atom::utils;

template <typename Engine, typename Distribution>
void bind_random(py::module &m, const std::string &name) {
    using RandomType = Random<Engine, Distribution>;
    py::class_<RandomType>(m, name.c_str())
        .def(py::init<typename RandomType::ResultType,
                      typename RandomType::ResultType>(),
             py::arg("min"), py::arg("max"))
        .def(py::init<typename RandomType::EngineType::result_type,
                      typename RandomType::DistributionType::param_type>(),
             py::arg("seed"), py::arg("params"))
        .def("seed", &RandomType::seed,
             py::arg("value") = std::random_device{}())
        .def("__call__", py::overload_cast<>(&RandomType::operator()))
        //.def("__call__",
        //     py::overload_cast<const typename RandomType::ParamType &>(
        //         &RandomType::operator(), py::const_))
        .def("generate", &RandomType::template generate<typename std::vector<
                             typename RandomType::ResultType>::iterator>)
        .def("vector", &RandomType::vector)
        .def("param", &RandomType::param)
        .def("engine", &RandomType::engine,
             py::return_value_policy::reference_internal)
        .def("distribution", &RandomType::distribution,
             py::return_value_policy::reference_internal);
}

PYBIND11_MODULE(diff, m) {
    m.def("encryptAES", &encryptAES, py::arg("plaintext"), py::arg("key"),
          py::arg("iv"), py::arg("tag"),
          "Encrypts the input plaintext using the AES algorithm.");
    m.def("decryptAES", &decryptAES, py::arg("ciphertext"), py::arg("key"),
          py::arg("iv"), py::arg("tag"),
          "Decrypts the input ciphertext using the AES algorithm.");
    m.def("compress", &compress, py::arg("data"),
          "Compresses the input data using the Zlib library.");
    m.def("decompress", &decompress, py::arg("data"),
          "Decompresses the input data using the Zlib library.");
    m.def("calculateSha256", &calculateSha256, py::arg("filename"),
          "Calculates the SHA-256 hash of a file.");
    m.def("calculateSha224", &calculateSha224, py::arg("data"),
          "Calculates the SHA-224 hash of a string.");
    m.def("calculateSha384", &calculateSha384, py::arg("data"),
          "Calculates the SHA-384 hash of a string.");
    m.def("calculateSha512", &calculateSha512, py::arg("data"),
          "Calculates the SHA-512 hash of a string.");

    py::class_<atom::utils::ArgumentParser>(m, "ArgumentParser")
        .def(py::init<>())
        .def(py::init<std::string>())
        .def("set_description", &atom::utils::ArgumentParser::setDescription)
        .def("set_epilog", &atom::utils::ArgumentParser::setEpilog)
        .def("add_argument", &atom::utils::ArgumentParser::addArgument,
             py::arg("name"),
             py::arg("type") = atom::utils::ArgumentParser::ArgType::AUTO,
             py::arg("required") = false, py::arg("default_value") = std::any(),
             py::arg("help") = "",
             py::arg("aliases") = std::vector<std::string>(),
             py::arg("is_positional") = false,
             py::arg("nargs") = atom::utils::ArgumentParser::Nargs())
        .def("add_flag", &atom::utils::ArgumentParser::addFlag, py::arg("name"),
             py::arg("help") = "",
             py::arg("aliases") = std::vector<std::string>())
        .def("add_subcommand", &atom::utils::ArgumentParser::addSubcommand)
        .def("add_mutually_exclusive_group",
             &atom::utils::ArgumentParser::addMutuallyExclusiveGroup)
        .def("add_argument_from_file",
             &atom::utils::ArgumentParser::addArgumentFromFile)
        .def("set_file_delimiter",
             &atom::utils::ArgumentParser::setFileDelimiter)
        .def("parse", &atom::utils::ArgumentParser::parse)
        .def("get_flag", &atom::utils::ArgumentParser::getFlag)
        .def("get_subcommand_parser",
             &atom::utils::ArgumentParser::getSubcommandParser)
        .def("print_help", &atom::utils::ArgumentParser::printHelp);

    py::enum_<atom::utils::ArgumentParser::ArgType>(m, "ArgType")
        .value("STRING", atom::utils::ArgumentParser::ArgType::STRING)
        .value("INTEGER", atom::utils::ArgumentParser::ArgType::INTEGER)
        .value("UNSIGNED_INTEGER",
               atom::utils::ArgumentParser::ArgType::UNSIGNED_INTEGER)
        .value("LONG", atom::utils::ArgumentParser::ArgType::LONG)
        .value("UNSIGNED_LONG",
               atom::utils::ArgumentParser::ArgType::UNSIGNED_LONG)
        .value("FLOAT", atom::utils::ArgumentParser::ArgType::FLOAT)
        .value("DOUBLE", atom::utils::ArgumentParser::ArgType::DOUBLE)
        .value("BOOLEAN", atom::utils::ArgumentParser::ArgType::BOOLEAN)
        .value("FILEPATH", atom::utils::ArgumentParser::ArgType::FILEPATH)
        .value("AUTO", atom::utils::ArgumentParser::ArgType::AUTO)
        .export_values();

    py::enum_<atom::utils::ArgumentParser::NargsType>(m, "NargsType")
        .value("NONE", atom::utils::ArgumentParser::NargsType::NONE)
        .value("OPTIONAL", atom::utils::ArgumentParser::NargsType::OPTIONAL)
        .value("ZERO_OR_MORE",
               atom::utils::ArgumentParser::NargsType::ZERO_OR_MORE)
        .value("ONE_OR_MORE",
               atom::utils::ArgumentParser::NargsType::ONE_OR_MORE)
        .value("CONSTANT", atom::utils::ArgumentParser::NargsType::CONSTANT)
        .export_values();

    py::class_<atom::utils::ArgumentParser::Nargs>(m, "Nargs")
        .def(py::init<>())
        .def(py::init<atom::utils::ArgumentParser::NargsType, int>(),
             py::arg("type"), py::arg("count") = 1)
        .def_readwrite("type", &atom::utils::ArgumentParser::Nargs::type)
        .def_readwrite("count", &atom::utils::ArgumentParser::Nargs::count);

    m.def("create_mask", &createMask<uint32_t>, py::arg("bits"),
          "Creates a bitmask with the specified number of bits set to 1.");
    m.def("count_bytes", &countBytes<uint32_t>, py::arg("value"),
          "Counts the number of set bits (1s) in the given value.");
    m.def("reverse_bits", &reverseBits<uint32_t>, py::arg("value"),
          "Reverses the bits in the given value.");
    m.def("rotate_left", &rotateLeft<uint32_t>, py::arg("value"),
          py::arg("shift"),
          "Performs a left rotation on the bits of the given value.");
    m.def("rotate_right", &rotateRight<uint32_t>, py::arg("value"),
          py::arg("shift"),
          "Performs a right rotation on the bits of the given value.");
    m.def("merge_masks", &mergeMasks<uint32_t>, py::arg("mask1"),
          py::arg("mask2"), "Merges two bitmasks into one.");
    m.def("split_mask", &splitMask<uint32_t>, py::arg("mask"),
          py::arg("position"), "Splits a bitmask into two parts.");

    py::class_<SequenceMatcher>(m, "SequenceMatcher")
        .def(py::init<const std::string &, const std::string &>())
        .def("set_seqs", &SequenceMatcher::setSeqs)
        .def("ratio", &SequenceMatcher::ratio)
        .def("get_matching_blocks", &SequenceMatcher::getMatchingBlocks)
        .def("get_opcodes", &SequenceMatcher::getOpcodes);

    py::class_<Differ>(m, "Differ")
        .def_static("compare", &Differ::compare)
        .def_static("unified_diff", &Differ::unifiedDiff);

    py::class_<HtmlDiff>(m, "HtmlDiff")
        .def_static("make_file", &HtmlDiff::makeFile)
        .def_static("make_table", &HtmlDiff::makeTable);

    m.def("get_close_matches", &getCloseMatches);

    py::class_<atom::error::ErrorInfo>(m, "ErrorInfo")
        .def(py::init<>())
        .def_readwrite("errorMessage", &atom::error::ErrorInfo::errorMessage)
        .def_readwrite("moduleName", &atom::error::ErrorInfo::moduleName)
        .def_readwrite("functionName", &atom::error::ErrorInfo::functionName)
        .def_readwrite("line", &atom::error::ErrorInfo::line)
        .def_readwrite("fileName", &atom::error::ErrorInfo::fileName)
        .def_readwrite("timestamp", &atom::error::ErrorInfo::timestamp)
        .def_readwrite("uuid", &atom::error::ErrorInfo::uuid)
        .def("__repr__", [](const atom::error::ErrorInfo &e) {
            return "<ErrorInfo errorMessage='" + e.errorMessage +
                   "' moduleName='" + e.moduleName + "' functionName='" +
                   e.functionName + "' line=" + std::to_string(e.line) +
                   " fileName='" + e.fileName +
                   "' timestamp=" + std::to_string(e.timestamp) + " uuid='" +
                   e.uuid + "'>";
        });

    py::class_<atom::error::ErrorStack,
               std::shared_ptr<atom::error::ErrorStack>>(m, "ErrorStack")
        .def(py::init<>())
        .def_static("create_shared", &atom::error::ErrorStack::createShared)
        .def_static("create_unique", &atom::error::ErrorStack::createUnique)
        .def("insert_error", &atom::error::ErrorStack::insertError)
        .def("set_filtered_modules",
             &atom::error::ErrorStack::setFilteredModules)
        .def("clear_filtered_modules",
             &atom::error::ErrorStack::clearFilteredModules)
        .def("print_filtered_error_stack",
             &atom::error::ErrorStack::printFilteredErrorStack)
        .def("get_filtered_errors_by_module",
             &atom::error::ErrorStack::getFilteredErrorsByModule)
        .def("get_compressed_errors",
             &atom::error::ErrorStack::getCompressedErrors);

    py::class_<LCG>(m, "LCG")
        .def(py::init<LCG::result_type>(),
             py::arg("seed") = static_cast<LCG::result_type>(
                 std::chrono::steady_clock::now().time_since_epoch().count()))
        .def("next", &LCG::next,
             "Generates the next random number in the sequence.")
        .def("seed", &LCG::seed, py::arg("new_seed"),
             "Seeds the generator with a new seed value.")
        .def("save_state", &LCG::saveState, py::arg("filename"),
             "Saves the current state of the generator to a file.")
        .def("load_state", &LCG::loadState, py::arg("filename"),
             "Loads the state of the generator from a file.")
        .def("next_int", &LCG::nextInt, py::arg("min") = 0,
             py::arg("max") = std::numeric_limits<int>::max(),
             "Generates a random integer within a specified range.")
        .def("next_double", &LCG::nextDouble, py::arg("min") = 0.0,
             py::arg("max") = 1.0,
             "Generates a random double within a specified range.")
        .def("next_bernoulli", &LCG::nextBernoulli,
             py::arg("probability") = 0.5,
             "Generates a random boolean value based on a specified "
             "probability.")
        .def("next_gaussian", &LCG::nextGaussian, py::arg("mean") = 0.0,
             py::arg("stddev") = 1.0,
             "Generates a random number following a Gaussian (normal) "
             "distribution.")
        .def("next_poisson", &LCG::nextPoisson, py::arg("lambda") = 1.0,
             "Generates a random number following a Poisson distribution.")
        .def("next_exponential", &LCG::nextExponential, py::arg("lambda") = 1.0,
             "Generates a random number following an Exponential distribution.")
        .def("next_geometric", &LCG::nextGeometric,
             py::arg("probability") = 0.5,
             "Generates a random number following a Geometric distribution.")
        .def("next_gamma", &LCG::nextGamma, py::arg("shape"),
             py::arg("scale") = 1.0,
             "Generates a random number following a Gamma distribution.")
        .def("next_beta", &LCG::nextBeta, py::arg("alpha"), py::arg("beta"),
             "Generates a random number following a Beta distribution.")
        .def("next_chi_squared", &LCG::nextChiSquared,
             py::arg("degrees_of_freedom"),
             "Generates a random number following a Chi-Squared distribution.")
        .def("next_hypergeometric", &LCG::nextHypergeometric, py::arg("total"),
             py::arg("success"), py::arg("draws"),
             "Generates a random number following a Hypergeometric "
             "distribution.")
        .def("next_discrete", &LCG::nextDiscrete, py::arg("weights"),
             "Generates a random index based on a discrete distribution.")
        .def("next_multinomial", &LCG::nextMultinomial, py::arg("trials"),
             py::arg("probabilities"), "Generates a multinomial distribution.")
        .def("shuffle", &LCG::shuffle<int>, py::arg("data"),
             "Shuffles a vector of data.")
        .def("sample", &LCG::sample<int>, py::arg("data"),
             py::arg("sample_size"), "Samples a subset of data from a vector.")
        .def_static("min", &LCG::min,
                    "Returns the minimum value that can be generated.")
        .def_static("max", &LCG::max,
                    "Returns the maximum value that can be generated.");

    py::class_<QDateTime>(m, "QDateTime")
        .def(py::init<>(), "Default constructor for QDateTime.")
        .def(
            py::init<const std::string &, const std::string &>(),
            py::arg("dateTimeString"), py::arg("format"),
            "Constructs a QDateTime object from a date-time string and format.")
        .def_static("currentDateTime",
                    py::overload_cast<>(&QDateTime::currentDateTime),
                    "Returns the current date and time.")
        .def_static(
            "fromString",
            py::overload_cast<const std::string &, const std::string &>(
                &QDateTime::fromString),
            py::arg("dateTimeString"), py::arg("format"),
            "Constructs a QDateTime object from a date-time string and format.")
        .def("toString",
             py::overload_cast<const std::string &>(&QDateTime::toString,
                                                    py::const_),
             py::arg("format"),
             "Converts the QDateTime object to a string in the specified "
             "format.")
        .def("toTimeT", &QDateTime::toTimeT,
             "Converts the QDateTime object to a std::time_t value.")
        .def("isValid", &QDateTime::isValid,
             "Checks if the QDateTime object is valid.")
        .def("addDays", &QDateTime::addDays, py::arg("days"),
             "Adds a number of days to the QDateTime object.")
        .def("addSecs", &QDateTime::addSecs, py::arg("seconds"),
             "Adds a number of seconds to the QDateTime object.")
        .def("daysTo", &QDateTime::daysTo, py::arg("other"),
             "Computes the number of days between the current QDateTime object "
             "and another QDateTime object.")
        .def("secsTo", &QDateTime::secsTo, py::arg("other"),
             "Computes the number of seconds between the current QDateTime "
             "object and another QDateTime object.")
        .def(py::self < py::self)
        .def(py::self <= py::self)
        .def(py::self > py::self)
        .def(py::self >= py::self)
        .def(py::self == py::self)
        .def(py::self != py::self);

    py::class_<QProcess>(m, "QProcess")
        .def(py::init<>(), "Default constructor for QProcess.")
        .def("set_working_directory", &QProcess::setWorkingDirectory,
             py::arg("dir"), "Sets the working directory for the process.")
        .def("set_environment", &QProcess::setEnvironment, py::arg("env"),
             "Sets the environment variables for the process.")
        .def(
            "start", &QProcess::start, py::arg("program"), py::arg("args"),
            "Starts the external process with the given program and arguments.")
        .def("wait_for_started", &QProcess::waitForStarted,
             py::arg("timeoutMs") = -1, "Waits for the process to start.")
        .def("wait_for_finished", &QProcess::waitForFinished,
             py::arg("timeoutMs") = -1, "Waits for the process to finish.")
        .def("is_running", &QProcess::isRunning,
             "Checks if the process is currently running.")
        .def("write", &QProcess::write, py::arg("data"),
             "Writes data to the process's standard input.")
        .def("read_all_standard_output", &QProcess::readAllStandardOutput,
             "Reads all available data from the process's standard output.")
        .def("read_all_standard_error", &QProcess::readAllStandardError,
             "Reads all available data from the process's standard error.")
        .def("terminate", &QProcess::terminate, "Terminates the process.");

    py::class_<ElapsedTimer>(m, "ElapsedTimer")
        .def(py::init<>(), "Default constructor.")
        .def("start", &ElapsedTimer::start, "Start or restart the timer.")
        .def("invalidate", &ElapsedTimer::invalidate, "Invalidate the timer.")
        .def("is_valid", &ElapsedTimer::isValid,
             "Check if the timer has been started and is valid.")
        .def("elapsed_ns", &ElapsedTimer::elapsedNs,
             "Get elapsed time in nanoseconds.")
        .def("elapsed_us", &ElapsedTimer::elapsedUs,
             "Get elapsed time in microseconds.")
        .def("elapsed_ms", &ElapsedTimer::elapsedMs,
             "Get elapsed time in milliseconds.")
        .def("elapsed_sec", &ElapsedTimer::elapsedSec,
             "Get elapsed time in seconds.")
        .def("elapsed_min", &ElapsedTimer::elapsedMin,
             "Get elapsed time in minutes.")
        .def("elapsed_hrs", &ElapsedTimer::elapsedHrs,
             "Get elapsed time in hours.")
        .def("elapsed", &ElapsedTimer::elapsed,
             "Get elapsed time in milliseconds (same as elapsedMs).")
        .def("has_expired", &ElapsedTimer::hasExpired, py::arg("ms"),
             "Check if a specified duration (in milliseconds) has passed.")
        .def("remaining_time_ms", &ElapsedTimer::remainingTimeMs, py::arg("ms"),
             "Get the remaining time until the specified duration (in "
             "milliseconds) has passed.")
        .def_static(
            "current_time_ms", &ElapsedTimer::currentTimeMs,
            "Get the current absolute time in milliseconds since epoch.")
        .def(py::self < py::self)
        .def(py::self > py::self)
        .def(py::self <= py::self)
        .def(py::self >= py::self)
        .def(py::self == py::self)
        .def(py::self != py::self);

    py::class_<ElapsedTimer>(m, "ElapsedTimer")
        .def(py::init<>(), "Default constructor.")
        .def("start", &ElapsedTimer::start, "Start or restart the timer.")
        .def("invalidate", &ElapsedTimer::invalidate, "Invalidate the timer.")
        .def("is_valid", &ElapsedTimer::isValid,
             "Check if the timer has been started and is valid.")
        .def("elapsed_ns", &ElapsedTimer::elapsedNs,
             "Get elapsed time in nanoseconds.")
        .def("elapsed_us", &ElapsedTimer::elapsedUs,
             "Get elapsed time in microseconds.")
        .def("elapsed_ms", &ElapsedTimer::elapsedMs,
             "Get elapsed time in milliseconds.")
        .def("elapsed_sec", &ElapsedTimer::elapsedSec,
             "Get elapsed time in seconds.")
        .def("elapsed_min", &ElapsedTimer::elapsedMin,
             "Get elapsed time in minutes.")
        .def("elapsed_hrs", &ElapsedTimer::elapsedHrs,
             "Get elapsed time in hours.")
        .def("elapsed", &ElapsedTimer::elapsed,
             "Get elapsed time in milliseconds (same as elapsedMs).")
        .def("has_expired", &ElapsedTimer::hasExpired, py::arg("ms"),
             "Check if a specified duration (in milliseconds) has passed.")
        .def("remaining_time_ms", &ElapsedTimer::remainingTimeMs, py::arg("ms"),
             "Get the remaining time until the specified duration (in "
             "milliseconds) has passed.")
        .def_static(
            "current_time_ms", &ElapsedTimer::currentTimeMs,
            "Get the current absolute time in milliseconds since epoch.")
        .def(py::self < py::self)
        .def(py::self > py::self)
        .def(py::self <= py::self)
        .def(py::self >= py::self)
        .def(py::self == py::self)
        .def(py::self != py::self);

    bind_random<std::mt19937, std::uniform_int_distribution<int>>(m,
                                                                  "RandomInt");
    bind_random<std::mt19937, std::uniform_real_distribution<double>>(
        m, "RandomDouble");

    m.def("get_timestamp_string", &getTimestampString,
          "Retrieves the current timestamp as a formatted string.");
    m.def("convert_to_china_time", &convertToChinaTime, py::arg("utcTimeStr"),
          "Converts a UTC time string to China Standard Time (CST, UTC+8).");
    m.def("get_china_timestamp_string", &getChinaTimestampString,
          "Retrieves the current China Standard Time (CST) as a formatted "
          "timestamp string.");
    m.def("timestamp_to_string", &timeStampToString, py::arg("timestamp"),
          "Converts a timestamp to a formatted string.");
    m.def("to_string", &toString, py::arg("tm"), py::arg("format"),
          "Converts a `tm` structure to a formatted string.");
    m.def("get_utc_time", &getUtcTime,
          "Retrieves the current UTC time as a formatted string.");
    m.def("timestamp_to_time", &timestampToTime, py::arg("timestamp"),
          "Converts a timestamp to a `tm` structure.");

    py::class_<UUID>(m, "UUID")
        .def(py::init<>(), "Constructs a new UUID with a random value.")
        .def(py::init<const std::array<uint8_t, 16> &>(), py::arg("data"),
             "Constructs a UUID from a given 16-byte array.")
        .def("to_string", &UUID::toString,
             "Converts the UUID to a string representation.")
        .def_static("from_string", &UUID::fromString, py::arg("str"),
                    "Creates a UUID from a string representation.")
        .def("get_data", &UUID::getData,
             "Retrieves the underlying data of the UUID.")
        .def("version", &UUID::version, "Gets the version of the UUID.")
        .def("variant", &UUID::variant, "Gets the variant of the UUID.")
        .def_static(
            "generate_v3", &UUID::generateV3, py::arg("namespace_uuid"),
            py::arg("name"),
            "Generates a version 3 UUID using the MD5 hashing algorithm.")
        .def_static(
            "generate_v5", &UUID::generateV5, py::arg("namespace_uuid"),
            py::arg("name"),
            "Generates a version 5 UUID using the SHA-1 hashing algorithm.")
        .def_static("generate_v1", &UUID::generateV1,
                    "Generates a version 1, time-based UUID.")
        .def_static("generate_v4", &UUID::generateV4,
                    "Generates a version 4, random UUID.")
        .def(py::self == py::self)
        .def(py::self != py::self)
        .def(py::self < py::self)
        .def(py::self > py::self)
        .def(py::self <= py::self)
        .def(py::self >= py::self)
        .def("__str__", &UUID::toString);

    m.def("generate_unique_uuid", &generateUniqueUUID,
          "Generates a unique UUID and returns it as a string.");

    py::class_<atom::utils::XMLReader>(m, "XMLReader")
        .def(py::init<const std::string &>())
        .def("get_child_element_names",
             &atom::utils::XMLReader::getChildElementNames)
        .def("get_element_text", &atom::utils::XMLReader::getElementText)
        .def("get_attribute_value", &atom::utils::XMLReader::getAttributeValue)
        .def("get_root_element_names",
             &atom::utils::XMLReader::getRootElementNames)
        .def("has_child_element", &atom::utils::XMLReader::hasChildElement)
        .def("get_child_element_text",
             &atom::utils::XMLReader::getChildElementText)
        .def("get_child_element_attribute_value",
             &atom::utils::XMLReader::getChildElementAttributeValue)
        .def("get_value_by_path", &atom::utils::XMLReader::getValueByPath)
        .def("get_attribute_value_by_path",
             &atom::utils::XMLReader::getAttributeValueByPath)
        .def("has_child_element_by_path",
             &atom::utils::XMLReader::hasChildElementByPath)
        .def("get_child_element_text_by_path",
             &atom::utils::XMLReader::getChildElementTextByPath)
        .def("get_child_element_attribute_value_by_path",
             &atom::utils::XMLReader::getChildElementAttributeValueByPath)
        .def("save_to_file", &atom::utils::XMLReader::saveToFile);
}
