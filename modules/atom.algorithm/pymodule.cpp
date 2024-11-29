#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "atom/algorithm/algorithm.hpp"
#include "atom/algorithm/annealing.hpp"
#include "atom/algorithm/base.hpp"
#include "atom/algorithm/bignumber.hpp"
#include "atom/algorithm/convolve.hpp"
#include "atom/algorithm/error_calibration.hpp"
#include "atom/algorithm/fnmatch.hpp"
#include "atom/algorithm/hash.hpp"
#include "atom/algorithm/huffman.hpp"
#include "atom/algorithm/math.hpp"
#include "atom/algorithm/matrix_compress.hpp"
#include "atom/algorithm/mhash.hpp"
#include "atom/algorithm/perlin.hpp"
#include "atom/algorithm/snowflake.hpp"
#include "atom/algorithm/tea.hpp"
#include "atom/algorithm/weight.hpp"

namespace py = pybind11;
using namespace atom::algorithm;

template <typename T>
void bind_advanced_error_calibration(py::module &m, const std::string &name) {
    py::class_<AdvancedErrorCalibration<T>>(m, name.c_str())
        .def(py::init<>())
        .def("linear_calibrate", &AdvancedErrorCalibration<T>::linearCalibrate)
        .def("polynomial_calibrate",
             &AdvancedErrorCalibration<T>::polynomialCalibrate)
        .def("apply", &AdvancedErrorCalibration<T>::apply)
        .def("print_parameters", &AdvancedErrorCalibration<T>::printParameters)
        .def("get_residuals", &AdvancedErrorCalibration<T>::getResiduals)
        .def("plot_residuals", &AdvancedErrorCalibration<T>::plotResiduals)
        .def("bootstrap_confidence_interval",
             &AdvancedErrorCalibration<T>::bootstrapConfidenceInterval)
        .def("outlier_detection",
             &AdvancedErrorCalibration<T>::outlierDetection)
        .def("cross_validation", &AdvancedErrorCalibration<T>::crossValidation)
        .def("get_slope", &AdvancedErrorCalibration<T>::getSlope)
        .def("get_intercept", &AdvancedErrorCalibration<T>::getIntercept)
        .def("get_r_squared", &AdvancedErrorCalibration<T>::getRSquared)
        .def("get_mse", &AdvancedErrorCalibration<T>::getMse)
        .def("get_mae", &AdvancedErrorCalibration<T>::getMae);
}

template <typename T>
void bind_weight_selector(py::module &m, const std::string &name) {
    py::class_<WeightSelector<T>>(m, name.c_str())
        .def(py::init<std::span<const T>,
                      std::unique_ptr<
                          typename WeightSelector<T>::SelectionStrategy>>(),
             py::arg("input_weights"),
             py::arg("custom_strategy") = std::make_unique<
                 typename WeightSelector<T>::DefaultSelectionStrategy>())
        .def("set_selection_strategy", &WeightSelector<T>::setSelectionStrategy)
        .def("select", &WeightSelector<T>::select)
        .def("select_multiple", &WeightSelector<T>::selectMultiple)
        .def("update_weight", &WeightSelector<T>::updateWeight)
        .def("add_weight", &WeightSelector<T>::addWeight)
        .def("remove_weight", &WeightSelector<T>::removeWeight)
        .def("normalize_weights", &WeightSelector<T>::normalizeWeights)
        // .def("apply_function_to_weights",
        // &WeightSelector<T>::applyFunctionToWeights)
        .def("batch_update_weights", &WeightSelector<T>::batchUpdateWeights)
        .def("get_weight", &WeightSelector<T>::getWeight)
        .def("get_max_weight_index", &WeightSelector<T>::getMaxWeightIndex)
        .def("get_min_weight_index", &WeightSelector<T>::getMinWeightIndex)
        .def("size", &WeightSelector<T>::size)
        .def("get_weights", &WeightSelector<T>::getWeights)
        .def("get_total_weight", &WeightSelector<T>::getTotalWeight)
        .def("reset_weights", &WeightSelector<T>::resetWeights)
        .def("scale_weights", &WeightSelector<T>::scaleWeights)
        .def("get_average_weight", &WeightSelector<T>::getAverageWeight)
        .def("print_weights", &WeightSelector<T>::printWeights);

    py::class_<typename WeightSelector<T>::SelectionStrategy,
               std::shared_ptr<typename WeightSelector<T>::SelectionStrategy>>(
        m, (name + "SelectionStrategy").c_str())
        .def("select", &WeightSelector<T>::SelectionStrategy::select);

    py::class_<
        typename WeightSelector<T>::DefaultSelectionStrategy,
        typename WeightSelector<T>::SelectionStrategy,
        std::shared_ptr<typename WeightSelector<T>::DefaultSelectionStrategy>>(
        m, (name + "DefaultSelectionStrategy").c_str())
        .def(py::init<>());

    py::class_<typename WeightSelector<T>::BottomHeavySelectionStrategy,
               typename WeightSelector<T>::SelectionStrategy,
               std::shared_ptr<
                   typename WeightSelector<T>::BottomHeavySelectionStrategy>>(
        m, (name + "BottomHeavySelectionStrategy").c_str())
        .def(py::init<>());

    py::class_<
        typename WeightSelector<T>::RandomSelectionStrategy,
        typename WeightSelector<T>::SelectionStrategy,
        std::shared_ptr<typename WeightSelector<T>::RandomSelectionStrategy>>(
        m, (name + "RandomSelectionStrategy").c_str())
        .def(py::init<size_t>());

    py::class_<typename WeightSelector<T>::WeightedRandomSampler>(
        m, (name + "WeightedRandomSampler").c_str())
        .def(py::init<>())
        .def("sample", &WeightSelector<T>::WeightedRandomSampler::sample);

    py::class_<TopHeavySelectionStrategy<T>,
               typename WeightSelector<T>::SelectionStrategy,
               std::shared_ptr<TopHeavySelectionStrategy<T>>>(
        m, (name + "TopHeavySelectionStrategy").c_str())
        .def(py::init<>());
}

PYBIND11_MODULE(algorithm, m) {
    py::class_<KMP>(m, "KMP")
        .def(py::init<std::string_view>())
        .def("search", &KMP::search)
        .def("set_pattern", &KMP::setPattern);

    py::class_<BoyerMoore>(m, "BoyerMoore")
        .def(py::init<std::string_view>())
        .def("search", &BoyerMoore::search)
        .def("set_pattern", &BoyerMoore::setPattern);

    py::class_<BloomFilter<1024>>(m, "BloomFilter")
        .def(py::init<std::size_t>())
        .def("insert", &BloomFilter<1024>::insert)
        .def("contains", &BloomFilter<1024>::contains);

    py::enum_<AnnealingStrategy>(m, "AnnealingStrategy")
        .value("LINEAR", AnnealingStrategy::LINEAR)
        .value("EXPONENTIAL", AnnealingStrategy::EXPONENTIAL)
        .value("LOGARITHMIC", AnnealingStrategy::LOGARITHMIC)
        .export_values();

    py::class_<TSP>(m, "TSP")
        .def(py::init<const std::vector<std::pair<double, double>> &>())
        .def("energy", &TSP::energy)
        .def("neighbor", &TSP::neighbor)
        .def("random_solution", &TSP::randomSolution);

    /*
        py::class_<SimulatedAnnealing<TSP, std::vector<int>>>(m,
                                                              "SimulatedAnnealing")
            .def(py::init<TSP&, AnnealingStrategy, int, double>())
            .def("set_cooling_schedule",
                 &SimulatedAnnealing<TSP, std::vector<int>>::setCoolingSchedule)
            .def("set_progress_callback",
                 &SimulatedAnnealing<TSP,
       std::vector<int>>::setProgressCallback) .def("set_stop_condition",
                 &SimulatedAnnealing<TSP, std::vector<int>>::setStopCondition)
            .def("optimize", &SimulatedAnnealing<TSP,
       std::vector<int>>::optimize) .def("get_best_energy",
                 &SimulatedAnnealing<TSP, std::vector<int>>::getBestEnergy);
    */

    m.def("base64_encode", &base64Encode, "Base64 encoding function");
    m.def("base64_decode", &base64Decode, "Base64 decoding function");
    m.def("xor_encrypt", &xorEncrypt, "Encrypt string using XOR algorithm");
    m.def("xor_decrypt", &xorDecrypt, "Decrypt string using XOR algorithm");

    py::class_<BigNumber>(m, "BigNumber")
        .def(py::init<std::string>())
        .def(py::init<long long>())
        .def("add", &BigNumber::add)
        .def("subtract", &BigNumber::subtract)
        .def("multiply", &BigNumber::multiply)
        .def("divide", &BigNumber::divide)
        .def("pow", &BigNumber::pow)
        .def("get_string", &BigNumber::getString)
        .def("set_string", &BigNumber::setString)
        .def("negate", &BigNumber::negate)
        .def("trim_leading_zeros", &BigNumber::trimLeadingZeros)
        .def("equals", py::overload_cast<const BigNumber &>(&BigNumber::equals,
                                                            py::const_))
        .def("equals", py::overload_cast<const long long &>(&BigNumber::equals,
                                                            py::const_))
        .def("equals", py::overload_cast<const std::string &>(
                           &BigNumber::equals, py::const_))
        .def("digits", &BigNumber::digits)
        .def("is_negative", &BigNumber::isNegative)
        .def("is_positive", &BigNumber::isPositive)
        .def("is_even", &BigNumber::isEven)
        .def("is_odd", &BigNumber::isOdd)
        .def("abs", &BigNumber::abs)
        .def("__str__", &BigNumber::getString)
        .def(py::self + py::self)
        .def(py::self - py::self)
        .def(py::self * py::self)
        .def(py::self / py::self)
        .def(py::self == py::self)
        .def(py::self > py::self)
        .def(py::self < py::self)
        .def(py::self >= py::self)
        .def(py::self <= py::self)
        .def("__iadd__", &BigNumber::operator+=)
        .def("__isub__", &BigNumber::operator-=)
        .def("__imul__", &BigNumber::operator*=)
        .def("__idiv__", &BigNumber::operator/=)
        .def("__neg__", &BigNumber::negate)
        .def("__abs__", &BigNumber::abs)
        .def("__len__", &BigNumber::digits)
        .def("__getitem__", &BigNumber::operator[])
        .def(
            "__iter__",
            [](const BigNumber &bn) {
                return py::make_iterator(bn.getString().begin(),
                                         bn.getString().end());
            },
            py::keep_alive<0, 1>());

    m.def("convolve", &convolve, "Perform 1D convolution operation",
          py::arg("input"), py::arg("kernel"));
    m.def("deconvolve", &deconvolve, "Perform 1D deconvolution operation",
          py::arg("input"), py::arg("kernel"));
    m.def("convolve2d", &convolve2D, "Perform 2D convolution operation",
          py::arg("input"), py::arg("kernel"), py::arg("num_threads") = 1);
    m.def("deconvolve2d", &deconvolve2D, "Perform 2D deconvolution operation",
          py::arg("signal"), py::arg("kernel"), py::arg("num_threads") = 1);
    m.def("dft2d", &dfT2D, "Perform 2D discrete Fourier transform",
          py::arg("signal"), py::arg("num_threads") = 1);
    m.def("idft2d", &idfT2D, "Perform 2D inverse discrete Fourier transform",
          py::arg("spectrum"), py::arg("num_threads") = 1);
    m.def("generate_gaussian_kernel", &generateGaussianKernel,
          "Generate 2D Gaussian kernel", py::arg("size"), py::arg("sigma"));
    m.def("apply_gaussian_filter", &applyGaussianFilter,
          "Apply Gaussian filter", py::arg("image"), py::arg("kernel"));

    bind_advanced_error_calibration<float>(m, "AdvancedErrorCalibrationFloat");
    bind_advanced_error_calibration<double>(m,
                                            "AdvancedErrorCalibrationDouble");

    m.def("fnmatch", &fnmatch, "Match string with specified pattern",
          py::arg("pattern"), py::arg("string"), py::arg("flags") = 0);
    m.def("filter",
          py::overload_cast<const std::vector<std::string> &, std::string_view,
                            int>(&filter),
          "Filter vector of strings based on specified pattern",
          py::arg("names"), py::arg("pattern"), py::arg("flags") = 0);
    m.def("filter",
          py::overload_cast<const std::vector<std::string> &,
                            const std::vector<std::string> &, int>(&filter),
          "Filter vector of strings based on multiple specified patterns",
          py::arg("names"), py::arg("patterns"), py::arg("flags") = 0);
    m.def("translate", &translate,
          "Translate pattern to different representation", py::arg("pattern"),
          py::arg("result"), py::arg("flags") = 0);

    m.def("compute_hash",
          py::overload_cast<const std::string &>(&computeHash<std::string>),
          "Compute hash value of a single hashable value");
    m.def("compute_hash",
          py::overload_cast<const std::vector<std::string> &>(
              &computeHash<std::string>),
          "Compute hash value of a vector of strings");
    m.def("compute_hash",
          py::overload_cast<const std::tuple<std::string, std::string> &>(
              &computeHash<std::string, std::string>),
          "Compute hash value of a tuple of strings");
    m.def("compute_hash",
          py::overload_cast<const std::array<std::string, 2> &>(
              &computeHash<std::string, 2>),
          "Compute hash value of an array of strings");
    //  m.def("compute_hash", py::overload_cast<const std::any&>(&computeHash),
    //  "Compute hash value of std::any");
    m.def("hash", &hash,
          "Compute hash value of a string using FNV-1a algorithm",
          py::arg("str"), py::arg("basis") = 2166136261U);
    m.def(
        "operator"
        "_hash",
        &operator""_hash, "Compute hash value of a string literal");

    py::class_<HuffmanNode, std::shared_ptr<HuffmanNode>>(m, "HuffmanNode")
        .def(py::init<char, int>())
        .def_readwrite("data", &HuffmanNode::data)
        .def_readwrite("frequency", &HuffmanNode::frequency)
        .def_readwrite("left", &HuffmanNode::left)
        .def_readwrite("right", &HuffmanNode::right);

    m.def("create_huffman_tree", &createHuffmanTree, "Create Huffman tree",
          py::arg("frequencies"));

    m.def("generate_huffman_codes", &generateHuffmanCodes,
          "Generate Huffman codes", py::arg("root"), py::arg("code"),
          py::arg("huffman_codes"));

    m.def("compress_data", &compressData, "Compress text", py::arg("text"),
          py::arg("huffman_codes"));

    m.def("decompress_data", &decompressData, "Decompress text",
          py::arg("compressed_text"), py::arg("root"));

    m.def("mul_div64", &mulDiv64,
          "Perform 64-bit multiplication and division operation",
          py::arg("operant"), py::arg("multiplier"), py::arg("divider"));
    m.def("safe_add", &safeAdd, "Perform safe addition operation", py::arg("a"),
          py::arg("b"));
    m.def("safe_mul", &safeMul, "Perform safe multiplication operation",
          py::arg("a"), py::arg("b"));
    m.def("rotl64", &rotl64, "Perform 64-bit integer left rotation operation",
          py::arg("n"), py::arg("c"));
    m.def("rotr64", &rotr64, "Perform 64-bit integer right rotation operation",
          py::arg("n"), py::arg("c"));
    m.def("clz64", &clz64, "Count leading zeros of a 64-bit integer",
          py::arg("x"));
    m.def("normalize", &normalize, "Normalize a 64-bit integer", py::arg("x"));
    m.def("safe_sub", &safeSub, "Perform safe subtraction operation",
          py::arg("a"), py::arg("b"));
    m.def("safe_div", &safeDiv, "Perform safe division operation", py::arg("a"),
          py::arg("b"));
    m.def("bit_reverse64", &bitReverse64,
          "Compute bitwise reversal of a 64-bit integer", py::arg("n"));
    m.def("approximate_sqrt", &approximateSqrt,
          "Approximate square root of a 64-bit integer", py::arg("n"));
    m.def("gcd64", &gcd64,
          "Compute greatest common divisor of two 64-bit integers",
          py::arg("a"), py::arg("b"));
    m.def("lcm64", &lcm64,
          "Compute least common multiple of two 64-bit integers", py::arg("a"),
          py::arg("b"));
    m.def("is_power_of_two", &isPowerOfTwo,
          "Check if a 64-bit integer is a power of two", py::arg("n"));
    m.def("next_power_of_two", &nextPowerOfTwo,
          "Compute the next power of two of a 64-bit integer", py::arg("n"));

    py::class_<MatrixCompressor>(m, "MatrixCompressor")
        .def_static("compress", &MatrixCompressor::compress, "Compress matrix",
                    py::arg("matrix"))
        .def_static("decompress", &MatrixCompressor::decompress,
                    "Decompress data to matrix", py::arg("compressed"),
                    py::arg("rows"), py::arg("cols"))
        .def_static("print_matrix", &MatrixCompressor::printMatrix,
                    "Print matrix", py::arg("matrix"))
        .def_static("generate_random_matrix",
                    &MatrixCompressor::generateRandomMatrix,
                    "Generate random matrix", py::arg("rows"), py::arg("cols"),
                    py::arg("charset") = "ABCD")
        .def_static("save_compressed_to_file",
                    &MatrixCompressor::saveCompressedToFile,
                    "Save compressed data to file", py::arg("compressed"),
                    py::arg("filename"))
        .def_static("load_compressed_from_file",
                    &MatrixCompressor::loadCompressedFromFile,
                    "Load compressed data from file", py::arg("filename"))
        .def_static("calculate_compression_ratio",
                    &MatrixCompressor::calculateCompressionRatio,
                    "Calculate compression ratio", py::arg("original"),
                    py::arg("compressed"))
        .def_static("downsample", &MatrixCompressor::downsample,
                    "Downsample matrix", py::arg("matrix"), py::arg("factor"))
        .def_static("upsample", &MatrixCompressor::upsample, "Upsample matrix",
                    py::arg("matrix"), py::arg("factor"))
        .def_static("calculate_mse", &MatrixCompressor::calculateMSE,
                    "Calculate mean squared error between two matrices",
                    py::arg("matrix1"), py::arg("matrix2"));

#if ATOM_ENABLE_DEBUG
    m.def("performance_test", &performanceTest,
          "Run performance test for matrix compression and decompression",
          py::arg("rows"), py::arg("cols"));
#endif

    m.def("hexstring_from_data", &hexstringFromData,
          "Convert string to hexadecimal string representation",
          py::arg("data"));
    m.def("data_from_hexstring", &dataFromHexstring,
          "Convert hexadecimal string representation to binary data",
          py::arg("data"));

    py::class_<MinHash>(m, "MinHash")
        .def(py::init<size_t>(), "Construct a MinHash object",
             py::arg("num_hashes"))
        .def(
            "compute_signature",
            [](const MinHash &self, const std::vector<std::string> &set) {
                return self.computeSignature(set);
            },
            "Compute MinHash signature for a given set", py::arg("set"))
        .def_static("jaccard_index", &MinHash::jaccardIndex,
                    "Compute Jaccard index between two sets", py::arg("sig1"),
                    py::arg("sig2"));

    m.def(
        "keccak256",
        [](const std::string &input) {
            auto hash = keccak256(
                reinterpret_cast<const uint8_t *>(input.data()), input.size());
            return std::vector<uint8_t>(hash.begin(), hash.end());
        },
        "Compute Keccak-256 hash value of input data", py::arg("input"));

    py::class_<PerlinNoise>(m, "PerlinNoise")
        .def(py::init<unsigned int>(), "Construct a PerlinNoise object",
             py::arg("seed") = std::default_random_engine::default_seed)
        .def("noise", &PerlinNoise::noise<double>, "Generate Perlin noise",
             py::arg("x"), py::arg("y"), py::arg("z"))
        .def("octave_noise", &PerlinNoise::octaveNoise<double>,
             "Generate octave Perlin noise", py::arg("x"), py::arg("y"),
             py::arg("z"), py::arg("octaves"), py::arg("persistence"))
        .def("generate_noise_map", &PerlinNoise::generateNoiseMap,
             "Generate noise map", py::arg("width"), py::arg("height"),
             py::arg("scale"), py::arg("octaves"), py::arg("persistence"),
             py::arg("lacunarity"),
             py::arg("seed") = std::default_random_engine::default_seed);

    constexpr uint64_t TWEPOCH = 1580504900000;
    using SnowflakeType = Snowflake<TWEPOCH>;

    py::class_<SnowflakeType>(m, "Snowflake")
        .def(py::init<>(),
             "Constructs a new Snowflake instance with a random secret key.")
        .def("init", &SnowflakeType::init, py::arg("worker_id"),
             py::arg("datacenter_id"),
             "Initializes the Snowflake generator with worker and datacenter "
             "IDs.")
        .def("nextid", &SnowflakeType::nextid, "Generates the next unique ID.")
        .def(
            "parse_id",
            [](const SnowflakeType &self, uint64_t encrypted_id) {
                uint64_t timestamp;
                uint64_t datacenterId;
                uint64_t workerId;
                uint64_t sequence;
                self.parseId(encrypted_id, timestamp, datacenterId, workerId,
                             sequence);
                return py::make_tuple(timestamp, datacenterId, workerId,
                                      sequence);
            },
            py::arg("encrypted_id"),
            "Parses an encrypted ID into its components: timestamp, datacenter "
            "ID, worker ID, and sequence.");

    m.def("tea_encrypt", &teaEncrypt,
          "Encrypt two 32-bit values using TEA algorithm", py::arg("value0"),
          py::arg("value1"), py::arg("key"));
    m.def("tea_decrypt", &teaDecrypt,
          "Decrypt two 32-bit values using TEA algorithm", py::arg("value0"),
          py::arg("value1"), py::arg("key"));
    m.def("xxtea_encrypt", &xxteaEncrypt,
          "Encrypt vector of 32-bit values using XXTEA algorithm",
          py::arg("input_data"), py::arg("input_key"));
    m.def("xxtea_decrypt", &xxteaDecrypt,
          "Decrypt vector of 32-bit values using XXTEA algorithm",
          py::arg("input_data"), py::arg("input_key"));
    m.def("xtea_encrypt", &xteaEncrypt,
          "Encrypt two 32-bit values using XTEA algorithm", py::arg("value0"),
          py::arg("value1"), py::arg("key"));
    m.def("xtea_decrypt", &xteaDecrypt,
          "Decrypt two 32-bit values using XTEA algorithm", py::arg("value0"),
          py::arg("value1"), py::arg("key"));
    m.def("to_uint32_vector", &toUint32Vector,
          "Convert byte array to vector of 32-bit unsigned integers",
          py::arg("data"));
    m.def("to_byte_array", &toByteArray,
          "Convert vector of 32-bit unsigned integers back to byte array",
          py::arg("data"));

    // TODO: Uncomment this after fixing the issue with std::span
    // bind_weight_selector<double>(m, "WeightSelectorDouble");
}
