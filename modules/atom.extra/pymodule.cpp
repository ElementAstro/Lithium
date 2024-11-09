#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>
#include <pybind11/stl.h>

#include "atom/extra/beast/http.hpp"
#include "atom/extra/beast/ws.hpp"

#if __has_include(<boost/charconv.hpp>)
#include "atom/extra/boost/charconv.hpp"
#endif
#include "atom/extra/boost/locale.hpp"
#include "atom/extra/boost/math.hpp"
#include "atom/extra/boost/regex.hpp"
#include "atom/extra/boost/system.hpp"
#include "atom/extra/boost/uuid.hpp"

#include "atom/extra/inicpp/inicpp.hpp"

namespace py = pybind11;
using namespace boost::numeric::ublas;
using namespace boost::system;

PYBIND11_MODULE(extra, m) {
    m.doc() = "Python bindings for Atom Extra Module";



/*
     py::class_<error_category>(m, "ErrorCategory")
        .def("name", &error_category::name)
        .def("default_error_condition",
             &error_category::default_error_condition)
        .def("equivalent", py::overload_cast<int, const error_condition &>(
                               &error_category::equivalent, py::const_))
        .def("equivalent", py::overload_cast<const error_code &, int>(
                               &error_category::equivalent, py::const_))
        .def("message",
             py::overload_cast<int>(&error_category::message, py::const_))
        .def("message", py::overload_cast<int, char *, std::size_t>(
                            &error_category::message, py::const_))
        .def("failed", &error_category::failed);

    py::class_<error_condition>(m, "ErrorCondition")
        .def(py::init<>())
        .def(py::init<int, const error_category &>())
        .def("assign", &error_condition::assign)
        .def("clear", &error_condition::clear)
        .def("value", &error_condition::value)
        .def("category", &error_condition::category)
        .def("message",
             py::overload_cast<>(&error_condition::message, py::const_))
        .def("message", py::overload_cast<char *, std::size_t>(
                            &error_condition::message, py::const_))
        .def("failed", &error_condition::failed);

    py::class_<error_code>(m, "ErrorCode")
        .def(py::init<>())
        .def(py::init<int, const error_category &>())
        .def("assign", &error_code::assign)
        .def("clear", &error_code::clear)
        .def("value", &error_code::value)
        .def("category", &error_code::category)
        .def("default_error_condition", &error_code::default_error_condition)
        .def("message", py::overload_cast<>(&error_code::message, py::const_))
        .def("message", py::overload_cast<char *, std::size_t>(
                            &error_code::message, py::const_))
        .def("failed", &error_code::failed);

      py::class_<HttpClient>(m, "HttpClient")
        .def(py::init<net::io_context &>(), py::arg("ioc"),
             "Constructs an HttpClient with the given I/O context")
        .def("set_default_header", &HttpClient::setDefaultHeader,
             py::arg("key"), py::arg("value"),
             "Sets a default header for all requests")
        .def("set_timeout", &HttpClient::setTimeout, py::arg("timeout"),
             "Sets the timeout duration for the HTTP operations")
        .def(
            "request", &HttpClient::request<http::string_body>,
            py::arg("method"), py::arg("host"), py::arg("port"),
            py::arg("target"), py::arg("version") = 11,
            py::arg("content_type") = "", py::arg("body") = "",
            py::arg("headers") = std::unordered_map<std::string, std::string>(),
            "Sends a synchronous HTTP request")
        .def(
            "async_request",
            &HttpClient::asyncRequest<
                http::string_body,
                std::function<void(beast::error_code,
                                   http::response<http::string_body>)>>,
            py::arg("method"), py::arg("host"), py::arg("port"),
            py::arg("target"), py::arg("handler"), py::arg("version") = 11,
            py::arg("content_type") = "", py::arg("body") = "",
            py::arg("headers") = std::unordered_map<std::string, std::string>(),
            "Sends an asynchronous HTTP request")
        .def(
            "json_request", &HttpClient::jsonRequest, py::arg("method"),
            py::arg("host"), py::arg("port"), py::arg("target"),
            py::arg("json_body") = json(),
            py::arg("headers") = std::unordered_map<std::string, std::string>(),
            "Sends a synchronous HTTP request with a JSON body and returns a "
            "JSON response")
        .def(
            "async_json_request",
            &HttpClient::asyncJsonRequest<
                std::function<void(beast::error_code, json)>>,
            py::arg("method"), py::arg("host"), py::arg("port"),
            py::arg("target"), py::arg("handler"),
            py::arg("json_body") = json(),
            py::arg("headers") = std::unordered_map<std::string, std::string>(),
            "Sends an asynchronous HTTP request with a JSON body and returns a "
            "JSON response")
        .def("upload_file", &HttpClient::uploadFile, py::arg("host"),
             py::arg("port"), py::arg("target"), py::arg("filepath"),
             py::arg("field_name") = "file", "Uploads a file to the server")
        .def("download_file", &HttpClient::downloadFile, py::arg("host"),
             py::arg("port"), py::arg("target"), py::arg("filepath"),
             "Downloads a file from the server")
        .def(
            "request_with_retry",
            &HttpClient::requestWithRetry<http::string_body>, py::arg("method"),
            py::arg("host"), py::arg("port"), py::arg("target"),
            py::arg("retry_count") = 3, py::arg("version") = 11,
            py::arg("content_type") = "", py::arg("body") = "",
            py::arg("headers") = std::unordered_map<std::string, std::string>(),
            "Sends a synchronous HTTP request with retry logic")
        .def(
            "batch_request", &HttpClient::batchRequest<http::string_body>,
            py::arg("requests"),
            py::arg("headers") = std::unordered_map<std::string, std::string>(),
            "Sends multiple synchronous HTTP requests in a batch")
        .def(
            "async_batch_request",
            &HttpClient::asyncBatchRequest<std::function<void(
                std::vector<http::response<http::string_body>>)>>,
            py::arg("requests"), py::arg("handler"),
            py::arg("headers") = std::unordered_map<std::string, std::string>(),
            "Sends multiple asynchronous HTTP requests in a batch")
        .def("run_with_thread_pool", &HttpClient::runWithThreadPool,
             py::arg("num_threads"), "Runs the I/O context with a thread pool")
        .def("async_download_file",
             &HttpClient::asyncDownloadFile<
                 std::function<void(beast::error_code, bool)>>,
             py::arg("host"), py::arg("port"), py::arg("target"),
             py::arg("filepath"), py::arg("handler"),
             "Asynchronously downloads a file from the server");

    py::class_<WSClient>(m, "WSClient")
        .def(py::init<net::io_context &>(), py::arg("ioc"),
             "Constructs a WSClient with the given I/O context")
        .def("set_timeout", &WSClient::setTimeout, py::arg("timeout"),
             "Sets the timeout duration for the WebSocket operations")
        .def("set_reconnect_options", &WSClient::setReconnectOptions,
             py::arg("retries"), py::arg("interval"),
             "Sets the reconnection options")
        .def("set_ping_interval", &WSClient::setPingInterval,
             py::arg("interval"), "Sets the interval for sending ping messages")
        .def("connect", &WSClient::connect, py::arg("host"), py::arg("port"),
             "Connects to the WebSocket server")
        .def("send", &WSClient::send, py::arg("message"),
             "Sends a message to the WebSocket server")
        .def("receive", &WSClient::receive,
             "Receives a message from the WebSocket server")
        .def("close", &WSClient::close, "Closes the WebSocket connection")
        // TODO: Implement async_connect
        //.def("async_connect",
        //     &WSClient::asyncConnect<std::function<void(beast::error_code)>>,
        //     py::arg("host"), py::arg("port"), py::arg("handler"),
        //     "Asynchronously connects to the WebSocket server")
        .def("async_send",
             &WSClient::asyncSend<
                 std::function<void(beast::error_code, std::size_t)>>,
             py::arg("message"), py::arg("handler"),
             "Asynchronously sends a message to the WebSocket server")
        .def("async_receive",
             &WSClient::asyncReceive<
                 std::function<void(beast::error_code, std::string)>>,
             py::arg("handler"),
             "Asynchronously receives a message from the WebSocket server")
        .def("async_close",
             &WSClient::asyncClose<std::function<void(beast::error_code)>>,
             py::arg("handler"),
             "Asynchronously closes the WebSocket connection")
        .def("async_send_json", &WSClient::asyncSendJson, py::arg("jdata"),
             py::arg("handler"),
             "Asynchronously sends a JSON object to the WebSocket server")
        .def("async_receive_json",
             &WSClient::asyncReceiveJson<
                 std::function<void(beast::error_code, json)>>,
             py::arg("handler"),
             "Asynchronously receives a JSON object from the WebSocket server");

*/

#if __has_include(<boost/charconv.hpp>)
    py::enum_<atom::extra::boost::NumberFormat>(m, "NumberFormat")
        .value("GENERAL", atom::extra::boost::NumberFormat::GENERAL)
        .value("SCIENTIFIC", atom::extra::boost::NumberFormat::SCIENTIFIC)
        .value("FIXED", atom::extra::boost::NumberFormat::FIXED)
        .value("HEX", atom::extra::boost::NumberFormat::HEX);

    py::class_<atom::extra::boost::FormatOptions>(m, "FormatOptions")
        .def(py::init<>())
        .def_readwrite("format", &atom::extra::boost::FormatOptions::format)
        .def_readwrite("precision",
                       &atom::extra::boost::FormatOptions::precision)
        .def_readwrite("uppercase",
                       &atom::extra::boost::FormatOptions::uppercase)
        .def_readwrite("thousands_separator",
                       &atom::extra::boost::FormatOptions::thousandsSeparator);

    py::class_<atom::extra::boost::BoostCharConv>(m, "BoostCharConv")
        .def_static("int_to_string",
                    &atom::extra::boost::BoostCharConv::intToString<int>,
                    "Convert an integer to a string", py::arg("value"),
                    py::arg("base") = atom::extra::boost::DEFAULT_BASE,
                    py::arg("options") = atom::extra::boost::FormatOptions())
        .def_static("float_to_string",
                    &atom::extra::boost::BoostCharConv::floatToString<float>,
                    "Convert a floating-point number to a string",
                    py::arg("value"),
                    py::arg("options") = atom::extra::boost::FormatOptions())
        .def_static("string_to_int",
                    &atom::extra::boost::BoostCharConv::stringToInt<int>,
                    "Convert a string to an integer", py::arg("str"),
                    py::arg("base") = atom::extra::boost::DEFAULT_BASE)
        .def_static("string_to_float",
                    &atom::extra::boost::BoostCharConv::stringToFloat<float>,
                    "Convert a string to a floating-point number",
                    py::arg("str"))
        .def_static("to_string",
                    &atom::extra::boost::BoostCharConv::toString<int>,
                    "Convert a value to a string", py::arg("value"),
                    py::arg("options") = atom::extra::boost::FormatOptions())
        .def_static("from_string",
                    &atom::extra::boost::BoostCharConv::fromString<int>,
                    "Convert a string to a value", py::arg("str"),
                    py::arg("base") = atom::extra::boost::DEFAULT_BASE)
        .def_static(
            "special_value_to_string",
            &atom::extra::boost::BoostCharConv::specialValueToString<float>,
            "Convert special floating-point values (NaN, Inf) to strings",
            py::arg("value"));
#endif

    py::class_<atom::extra::boost::LocaleWrapper>(m, "LocaleWrapper")
        .def(py::init<const std::string &>(), py::arg("locale_name") = "",
             "Constructs a LocaleWrapper object with the specified locale")
        .def_static("to_utf8", &atom::extra::boost::LocaleWrapper::toUtf8,
                    py::arg("str"), py::arg("from_charset"),
                    "Converts a string to UTF-8 encoding")
        .def_static("from_utf8", &atom::extra::boost::LocaleWrapper::fromUtf8,
                    py::arg("str"), py::arg("to_charset"),
                    "Converts a UTF-8 encoded string to another character set")
        .def_static("normalize", &atom::extra::boost::LocaleWrapper::normalize,
                    py::arg("str"),
                    py::arg("norm") = ::boost::locale::norm_default,
                    "Normalizes a Unicode string")
        .def_static("tokenize", &atom::extra::boost::LocaleWrapper::tokenize,
                    py::arg("str"), py::arg("locale_name") = "",
                    "Tokenizes a string into words")
        .def_static("translate", &atom::extra::boost::LocaleWrapper::translate,
                    py::arg("str"), py::arg("domain"),
                    py::arg("locale_name") = "",
                    "Translates a string to the specified locale")
        .def("to_upper", &atom::extra::boost::LocaleWrapper::toUpper,
             py::arg("str"), "Converts a string to uppercase")
        .def("to_lower", &atom::extra::boost::LocaleWrapper::toLower,
             py::arg("str"), "Converts a string to lowercase")
        .def("to_title", &atom::extra::boost::LocaleWrapper::toTitle,
             py::arg("str"), "Converts a string to title case")
        .def("compare", &atom::extra::boost::LocaleWrapper::compare,
             py::arg("str1"), py::arg("str2"),
             "Compares two strings using locale-specific collation rules")
        .def_static("format_date",
                    &atom::extra::boost::LocaleWrapper::formatDate,
                    py::arg("date_time"), py::arg("format"),
                    "Formats a date and time according to the specified format")
        .def_static("format_number",
                    &atom::extra::boost::LocaleWrapper::formatNumber,
                    py::arg("number"), py::arg("precision") = 2,
                    "Formats a number with the specified precision")
        .def_static("format_currency",
                    &atom::extra::boost::LocaleWrapper::formatCurrency,
                    py::arg("amount"), py::arg("currency"),
                    "Formats a currency amount");
    // TODO: Implement regex_replace
    //.def_static("regex_replace",
    //            &atom::extra::boost::LocaleWrapper::regexReplace,
    //            py::arg("str"), py::arg("regex"), py::arg("format"),
    //            "Replaces occurrences of a regex pattern in a string with
    //            " "a format string")
    //.def("format", &atom::extra::boost::LocaleWrapper::format<std::string>,
    //     py::arg("format_string"), py::kwargs(),
    //     "Formats a string with named arguments");

    /*
    TODO: Uncomment this after fixing the Boost.Python issue
         py::class_<unbounded_array<int, std::allocator<int>>>(m,
                                                              "UnboundedArrayInt")
            .def(py::init<>())
            .def(py::init<size_t>())
            .def(py::init<size_t, int>())
            .def("resize",
                 (void(unbounded_array<int, std::allocator<int>>::*)(size_t)) &
                     unbounded_array<int, std::allocator<int>>::resize)
            .def("resize",
                 (void(unbounded_array<int, std::allocator<int>>::*)(size_t,
    int)) & unbounded_array<int, std::allocator<int>>::resize) .def("size",
    &unbounded_array<int, std::allocator<int>>::size) .def("__getitem__",
                 [](const unbounded_array<int, std::allocator<int>> &a, size_t
    i) { if (i >= a.size()) throw py::index_error(); return a[i];
                 })
            .def("__setitem__",
                 [](unbounded_array<int, std::allocator<int>> &a, size_t i, int
    v) { if (i >= a.size()) throw py::index_error(); a[i] = v;
                 })
            .def("__len__", &unbounded_array<int, std::allocator<int>>::size);

        py::class_<matrix<double, row_major, unbounded_array<double>>>(m,
    "Matrix") .def(py::init<>()) .def(py::init<size_t, size_t>())
            .def(py::init<size_t, size_t, double>())
            .def("size1",
                 &matrix<double, row_major, unbounded_array<double>>::size1)
            .def("size2",
                 &matrix<double, row_major, unbounded_array<double>>::size2)
            .def("resize",
                 &matrix<double, row_major, unbounded_array<double>>::resize)
            .def("clear",
                 &matrix<double, row_major, unbounded_array<double>>::clear)
            .def(
                "insert_element",
                &matrix<double, row_major,
    unbounded_array<double>>::insert_element) .def("erase_element",
                 &matrix<double, row_major,
    unbounded_array<double>>::erase_element) .def("__getitem__",
                 [](const matrix<double, row_major, unbounded_array<double>> &m,
                    std::pair<size_t, size_t> index) {
                     return m(index.first, index.second);
                 })
            .def("__setitem__",
                 [](matrix<double, row_major, unbounded_array<double>> &m,
                    std::pair<size_t, size_t> index,
                    double value) { m(index.first, index.second) = value; });

        py::class_<vector<double, unbounded_array<double>>>(m, "Vector")
            .def(py::init<>())
            .def(py::init<vector<double, unbounded_array<double>>::size_type>())
            .def(py::init<
                 vector<double, unbounded_array<double>>::size_type,
                 const vector<double, unbounded_array<double>>::value_type &>())
            .def("size", &vector<double, unbounded_array<double>>::size)
            .def("resize", &vector<double, unbounded_array<double>>::resize)
            .def("clear", &vector<double, unbounded_array<double>>::clear)
            .def("__getitem__",
                 [](const vector<double, unbounded_array<double>> &v,
                    vector<double, unbounded_array<double>>::size_type i) {
                     if (i >= v.size())
                         throw py::index_error();
                     return v[i];
                 })
            .def("__setitem__",
                 [](vector<double, unbounded_array<double>> &v,
                    vector<double, unbounded_array<double>>::size_type i,
                    double val) {
                     if (i >= v.size())
                         throw py::index_error();
                     v[i] = val;
                 })
            .def("__len__", &vector<double, unbounded_array<double>>::size)
            .def("__repr__", [](const vector<double, unbounded_array<double>>
    &v) { std::ostringstream oss; oss << "Vector(["; for (size_t i = 0; i <
    v.size(); ++i) { if (i > 0) oss << ", "; oss << v[i];
                }
                oss << "])";
                return oss.str();
            });

    */

    py::class_<atom::extra::boost::SpecialFunctions<double>>(m,
                                                             "SpecialFunctions")
        .def_static("beta", &atom::extra::boost::SpecialFunctions<double>::beta,
                    "Compute the beta function")
        .def_static("gamma",
                    &atom::extra::boost::SpecialFunctions<double>::gamma,
                    "Compute the gamma function")
        .def_static("digamma",
                    &atom::extra::boost::SpecialFunctions<double>::digamma,
                    "Compute the digamma function")
        .def_static("erf", &atom::extra::boost::SpecialFunctions<double>::erf,
                    "Compute the error function")
        .def_static("bessel_j",
                    &atom::extra::boost::SpecialFunctions<double>::besselJ,
                    "Compute the Bessel function of the first kind")
        .def_static("legendre_p",
                    &atom::extra::boost::SpecialFunctions<double>::legendreP,
                    "Compute the Legendre polynomial");

    py::class_<atom::extra::boost::Statistics<double>>(m, "Statistics")
        .def_static("mean", &atom::extra::boost::Statistics<double>::mean,
                    "Compute the mean of a dataset")
        .def_static("variance",
                    &atom::extra::boost::Statistics<double>::variance,
                    "Compute the variance of a dataset")
        .def_static("skewness",
                    &atom::extra::boost::Statistics<double>::skewness,
                    "Compute the skewness of a dataset")
        .def_static("kurtosis",
                    &atom::extra::boost::Statistics<double>::kurtosis,
                    "Compute the kurtosis of a dataset");

    py::class_<atom::extra::boost::Distributions<double>::NormalDistribution>(
        m, "NormalDistribution")
        .def(py::init<double, double>(), py::arg("mean"), py::arg("stddev"))
        .def(
            "pdf",
            &atom::extra::boost::Distributions<double>::NormalDistribution::pdf,
            "Compute the probability density function (PDF)")
        .def(
            "cdf",
            &atom::extra::boost::Distributions<double>::NormalDistribution::cdf,
            "Compute the cumulative distribution function (CDF)")
        .def("quantile",
             &atom::extra::boost::Distributions<
                 double>::NormalDistribution::quantile,
             "Compute the quantile (inverse CDF)");

    py::class_<atom::extra::boost::Distributions<double>::StudentTDistribution>(
        m, "StudentTDistribution")
        .def(py::init<double>(), py::arg("degrees_of_freedom"))
        .def("pdf",
             &atom::extra::boost::Distributions<
                 double>::StudentTDistribution::pdf,
             "Compute the probability density function (PDF)")
        .def("cdf",
             &atom::extra::boost::Distributions<
                 double>::StudentTDistribution::cdf,
             "Compute the cumulative distribution function (CDF)")
        .def("quantile",
             &atom::extra::boost::Distributions<
                 double>::StudentTDistribution::quantile,
             "Compute the quantile (inverse CDF)");

    py::class_<atom::extra::boost::Distributions<double>::PoissonDistribution>(
        m, "PoissonDistribution")
        .def(py::init<double>(), py::arg("mean"))
        .def("pdf",
             &atom::extra::boost::Distributions<
                 double>::PoissonDistribution::pdf,
             "Compute the probability density function (PDF)")
        .def("cdf",
             &atom::extra::boost::Distributions<
                 double>::PoissonDistribution::cdf,
             "Compute the cumulative distribution function (CDF)");

    py::class_<
        atom::extra::boost::Distributions<double>::ExponentialDistribution>(
        m, "ExponentialDistribution")
        .def(py::init<double>(), py::arg("lambda"))
        .def("pdf",
             &atom::extra::boost::Distributions<
                 double>::ExponentialDistribution::pdf,
             "Compute the probability density function (PDF)")
        .def("cdf",
             &atom::extra::boost::Distributions<
                 double>::ExponentialDistribution::cdf,
             "Compute the cumulative distribution function (CDF)");

    py::class_<atom::extra::boost::NumericalIntegration<double>>(
        m, "NumericalIntegration")
        .def_static(
            "trapezoidal",
            &atom::extra::boost::NumericalIntegration<double>::trapezoidal,
            "Compute the integral of a function using the trapezoidal rule");

    m.def("factorial", &atom::extra::boost::factorial<double>,
          "Compute the factorial of a number");

    py::class_<atom::extra::boost::Optimization<double>>(m, "Optimization")
        .def_static(
            "golden_section_search",
            &atom::extra::boost::Optimization<double>::goldenSectionSearch,
            "Perform one-dimensional golden section search to find the minimum "
            "of a function")
        .def_static(
            "newton_raphson",
            &atom::extra::boost::Optimization<double>::newtonRaphson,
            "Perform Newton-Raphson method to find the root of a function");

    /*
    py::class_<atom::extra::boost::LinearAlgebra<double>>(m, "LinearAlgebra")
            .def_static(
                "solve_linear_system",
                &atom::extra::boost::LinearAlgebra<double>::solveLinearSystem,
                "Solve a linear system of equations Ax = b")
            .def_static("determinant",
                        &atom::extra::boost::LinearAlgebra<double>::determinant,
                        "Compute the determinant of a matrix")
            .def_static("multiply",
                        &atom::extra::boost::LinearAlgebra<double>::multiply,
                        "Multiply two matrices")
        .def_static("transpose",
                    &atom::extra::boost::LinearAlgebra<double>::transpose,
                    "Compute the transpose of a matrix");

    */

    py::class_<atom::extra::boost::ODESolver<double>>(m, "ODESolver")
        .def_static("runge_kutta4",
                    &atom::extra::boost::ODESolver<double>::rungeKutta4,
                    "Solve an ODE using the 4th order Runge-Kutta method");

    py::class_<atom::extra::boost::FinancialMath<double>>(m, "FinancialMath")
        .def_static(
            "black_scholes_call",
            &atom::extra::boost::FinancialMath<double>::blackScholesCall,
            "Compute the price of a European call option using the "
            "Black-Scholes formula")
        .def_static(
            "modified_duration",
            &atom::extra::boost::FinancialMath<double>::modifiedDuration,
            "Compute the modified duration of a bond")
        .def_static("bond_price",
                    &atom::extra::boost::FinancialMath<double>::bondPrice,
                    "Compute the price of a bond")
        .def_static(
            "implied_volatility",
            &atom::extra::boost::FinancialMath<double>::impliedVolatility,
            "Compute the implied volatility of an option");

    py::class_<atom::extra::boost::RegexWrapper>(m, "RegexWrapper")
        .def(py::init<std::string_view,
                      ::boost::regex_constants::syntax_option_type>(),
             py::arg("pattern"),
             py::arg("flags") = ::boost::regex_constants::normal)
        .def("match", &atom::extra::boost::RegexWrapper::match<std::string>,
             "Match the given string against the regex pattern", py::arg("str"))
        .def("search", &atom::extra::boost::RegexWrapper::search<std::string>,
             "Search the given string for the first match of the regex pattern",
             py::arg("str"))
        .def("search_all",
             &atom::extra::boost::RegexWrapper::searchAll<std::string>,
             "Search the given string for all matches of the regex pattern",
             py::arg("str"))
        .def("replace",
             &atom::extra::boost::RegexWrapper::replace<std::string,
                                                        std::string>,
             "Replace all matches of the regex pattern in the given string "
             "with the replacement string",
             py::arg("str"), py::arg("replacement"))
        .def("split", &atom::extra::boost::RegexWrapper::split<std::string>,
             "Split the given string by the regex pattern", py::arg("str"))
        // TODO: Uncomment this after fixing the issue
        // .def("match_groups",
        //      &atom::extra::boost::RegexWrapper::matchGroups<std::string>,
        //      "Match the given string and return the groups of each match",
        //      py::arg("str"))
        //.def("for_each_match",
        //     &atom::extra::boost::RegexWrapper::forEachMatch<
        //         std::string, std::function<void(const ::boost::smatch &)>>,
        //     "Apply a function to each match of the regex pattern in the given "
        //     "string",
        //     py::arg("str"), py::arg("func"))
        .def("get_pattern", &atom::extra::boost::RegexWrapper::getPattern,
             "Get the regex pattern as a string")
        .def("set_pattern", &atom::extra::boost::RegexWrapper::setPattern,
             "Set a new regex pattern with optional flags", py::arg("pattern"),
             py::arg("flags") = ::boost::regex_constants::normal)
        .def("named_captures",
             &atom::extra::boost::RegexWrapper::namedCaptures<std::string>,
             "Match the given string and return the named captures",
             py::arg("str"))
        .def("is_valid",
             &atom::extra::boost::RegexWrapper::isValid<std::string>,
             "Check if the given string is a valid match for the regex pattern",
             py::arg("str"))
        .def("replace_callback",
             &atom::extra::boost::RegexWrapper::replaceCallback<std::string>,
             "Replace all matches of the regex pattern in the given string "
             "using a callback function",
             py::arg("str"), py::arg("callback"))
        .def_static("escape_string",
                    &atom::extra::boost::RegexWrapper::escapeString,
                    "Escape special characters in the given string for use in "
                    "a regex pattern",
                    py::arg("str"))
        .def("benchmark_match",
             &atom::extra::boost::RegexWrapper::benchmarkMatch<std::string>,
             "Benchmark the match operation for the given string over a number "
             "of iterations",
             py::arg("str"), py::arg("iterations") = 1000)
        .def_static(
            "is_valid_regex", &atom::extra::boost::RegexWrapper::isValidRegex,
            "Check if the given regex pattern is valid", py::arg("pattern"));

    py::class_<atom::extra::boost::Error>(m, "Error")
        .def(py::init<>(), "Default constructor")
        .def(py::init<const ::boost::system::error_code &>(),
             py::arg("error_code"),
             "Constructs an Error from a Boost.System error code")
        .def(py::init<int, const ::boost::system::error_category &>(),
             py::arg("error_value"), py::arg("error_category"),
             "Constructs an Error from an error value and category")
        .def("value", &atom::extra::boost::Error::value, "Gets the error value")
        .def("category", &atom::extra::boost::Error::category,
             "Gets the error category")
        .def("message", &atom::extra::boost::Error::message,
             "Gets the error message")
        .def("__bool__", &atom::extra::boost::Error::operator bool,
             "Checks if the error code is valid")
        .def("to_boost_error_code",
             &atom::extra::boost::Error::toBoostErrorCode,
             "Converts to a Boost.System error code")
        .def("__eq__", &atom::extra::boost::Error::operator==,
             "Equality operator")
        .def("__ne__", &atom::extra::boost::Error::operator!=,
             "Inequality operator");

    py::class_<atom::extra::boost::Exception, std::system_error>(m, "Exception")
        .def(py::init<const atom::extra::boost::Error &>(), py::arg("error"),
             "Constructs an Exception from an Error")
        .def("error", &atom::extra::boost::Exception::error,
             "Gets the associated Error");

    /*
        py::class_<atom::extra::boost::Result<void>>(m, "ResultVoid")
            .def(py::init<>(), "Default constructor")
            .def(py::init<atom::extra::boost::Error>(), py::arg("error"),
       "Constructs a Result with an Error") .def("has_value",
       &atom::extra::boost::Result<void>::hasValue, "Checks if the Result has a
       value") .def("error",
       py::overload_cast<>(&atom::extra::boost::Result<void>::error,
       py::const_), "Gets the associated Error") .def("__bool__",
       &atom::extra::boost::Result<void>::operator bool, "Checks if the Result
       has a value");

        py::class_<atom::extra::boost::Result<std::string>>(m, "ResultString")
            .def(py::init<std::string>(), py::arg("value"), "Constructs a Result
       with a value") .def(py::init<atom::extra::boost::Error>(),
       py::arg("error"), "Constructs a Result with an Error") .def("has_value",
       &atom::extra::boost::Result<std::string>::hasValue, "Checks if the Result
       has a value") .def("value",
       py::overload_cast<>(&atom::extra::boost::Result<std::string>::value,
       py::const_), "Gets the result value") .def("error",
       py::overload_cast<>(&atom::extra::boost::Result<std::string>::error,
       py::const_), "Gets the associated Error") .def("__bool__",
       &atom::extra::boost::Result<std::string>::operator bool, "Checks if the
       Result has a value");

        m.def("make_result", [](const std::function<std::string()>& func) {
            return atom::extra::boost::makeResult(func);
        }, "Creates a Result from a function");
    */

    py::class_<atom::extra::boost::UUID>(m, "UUID")
        .def(py::init<>(),
             "Default constructor that generates a random UUID (v4)")
        .def(py::init<const std::string &>(), py::arg("str"),
             "Constructs a UUID from a string representation")
        .def(py::init<const ::boost::uuids::uuid &>(), py::arg("uuid"),
             "Constructs a UUID from a Boost.UUID object")
        .def("to_string", &atom::extra::boost::UUID::toString,
             "Converts the UUID to a string representation")
        .def("is_nil", &atom::extra::boost::UUID::isNil,
             "Checks if the UUID is nil (all zeros)")
        .def("__eq__", &atom::extra::boost::UUID::operator==,
             "Checks if this UUID is equal to another UUID")
        .def(
            "__lt__",
            [](const atom::extra::boost::UUID &self,
               const atom::extra::boost::UUID &other) { return self < other; },
            "Less than comparison for UUIDs")
        .def(
            "__le__",
            [](const atom::extra::boost::UUID &self,
               const atom::extra::boost::UUID &other) { return self <= other; },
            "Less than or equal comparison for UUIDs")
        .def(
            "__gt__",
            [](const atom::extra::boost::UUID &self,
               const atom::extra::boost::UUID &other) { return self > other; },
            "Greater than comparison for UUIDs")
        .def(
            "__ge__",
            [](const atom::extra::boost::UUID &self,
               const atom::extra::boost::UUID &other) { return self >= other; },
            "Greater than or equal comparison for UUIDs")
        .def("format", &atom::extra::boost::UUID::format,
             "Formats the UUID as a string enclosed in curly braces")
        .def("to_bytes", &atom::extra::boost::UUID::toBytes,
             "Converts the UUID to a vector of bytes")
        .def_static("from_bytes", &atom::extra::boost::UUID::fromBytes,
                    py::arg("bytes"), "Constructs a UUID from a span of bytes")
        .def("to_uint64", &atom::extra::boost::UUID::toUint64,
             "Converts the UUID to a 64-bit unsigned integer")
        .def_static("namespace_dns", &atom::extra::boost::UUID::namespaceDNS,
                    "Gets the DNS namespace UUID")
        .def_static("namespace_url", &atom::extra::boost::UUID::namespaceURL,
                    "Gets the URL namespace UUID")
        .def_static("namespace_oid", &atom::extra::boost::UUID::namespaceOID,
                    "Gets the OID namespace UUID")
        .def_static("v3", &atom::extra::boost::UUID::v3,
                    py::arg("namespace_uuid"), py::arg("name"),
                    "Generates a version 3 (MD5) UUID based on a namespace "
                    "UUID and a name")
        .def_static("v5", &atom::extra::boost::UUID::v5,
                    py::arg("namespace_uuid"), py::arg("name"),
                    "Generates a version 5 (SHA-1) UUID based on a namespace "
                    "UUID and a name")
        .def("version", &atom::extra::boost::UUID::version,
             "Gets the version of the UUID")
        .def("variant", &atom::extra::boost::UUID::variant,
             "Gets the variant of the UUID")
        .def_static("v1", &atom::extra::boost::UUID::v1,
                    "Generates a version 1 (timestamp-based) UUID")
        .def_static("v4", &atom::extra::boost::UUID::v4,
                    "Generates a version 4 (random) UUID")
        .def("to_base64", &atom::extra::boost::UUID::toBase64,
             "Converts the UUID to a Base64 string representation")
        .def("get_timestamp", &atom::extra::boost::UUID::getTimestamp,
             "Gets the timestamp from a version 1 UUID")
        .def(
            "__hash__",
            [](const atom::extra::boost::UUID &self) {
                return std::hash<atom::extra::boost::UUID>()(self);
            },
            "Hash function for UUIDs");

    py::class_<inicpp::IniFileBase<std::less<>>>(m, "IniFile")
        .def(py::init<>(), "Default constructor")
        .def(py::init<const std::string &>(), py::arg("filename"),
             "Constructs an IniFileBase from a file")
        .def(py::init<std::istream &>(), py::arg("iss"),
             "Constructs an IniFileBase from an input stream")
        .def("set_field_sep", &inicpp::IniFileBase<std::less<>>::setFieldSep,
             py::arg("sep"), "Sets the field separator character")
        .def("set_comment_prefixes",
             &inicpp::IniFileBase<std::less<>>::setCommentPrefixes,
             py::arg("comment_prefixes"), "Sets the comment prefixes")
        .def("set_escape_char",
             &inicpp::IniFileBase<std::less<>>::setEscapeChar, py::arg("esc"),
             "Sets the escape character")
        .def("set_multi_line_values",
             &inicpp::IniFileBase<std::less<>>::setMultiLineValues,
             py::arg("enable"), "Enables or disables multi-line values")
        .def("allow_overwrite_duplicate_fields",
             &inicpp::IniFileBase<std::less<>>::allowOverwriteDuplicateFields,
             py::arg("allowed"),
             "Allows or disallows overwriting duplicate fields")
        .def("decode",
             py::overload_cast<std::istream &>(
                 &inicpp::IniFileBase<std::less<>>::decode),
             py::arg("iss"), "Decodes an INI file from an input stream")
        .def("decode",
             py::overload_cast<const std::string &>(
                 &inicpp::IniFileBase<std::less<>>::decode),
             py::arg("content"), "Decodes an INI file from a string")
        .def("load", &inicpp::IniFileBase<std::less<>>::load,
             py::arg("file_name"),
             "Loads and decodes an INI file from a file path")
        // .def("encode", py::overload_cast<std::ostream&
        // const>(&inicpp::IniFileBase<std::less<>>::encode, py::const_),
        // py::arg("oss"), "Encodes the INI file to an output stream")
        .def("encode",
             py::overload_cast<>(&inicpp::IniFileBase<std::less<>>::encode,
                                 py::const_),
             "Encodes the INI file to a string and returns it")
        .def("save", &inicpp::IniFileBase<std::less<>>::save,
             py::arg("file_name"), "Saves the INI file to a given file path");

    py::class_<inicpp::IniFileBase<inicpp::StringInsensitiveLess>>(
        m, "IniFileCaseInsensitive")
        .def(py::init<>(), "Default constructor")
        .def(py::init<const std::string &>(), py::arg("filename"),
             "Constructs an IniFileBase from a file")
        .def(py::init<std::istream &>(), py::arg("iss"),
             "Constructs an IniFileBase from an input stream")
        .def("set_field_sep",
             &inicpp::IniFileBase<inicpp::StringInsensitiveLess>::setFieldSep,
             py::arg("sep"), "Sets the field separator character")
        .def("set_comment_prefixes",
             &inicpp::IniFileBase<
                 inicpp::StringInsensitiveLess>::setCommentPrefixes,
             py::arg("comment_prefixes"), "Sets the comment prefixes")
        .def("set_escape_char",
             &inicpp::IniFileBase<inicpp::StringInsensitiveLess>::setEscapeChar,
             py::arg("esc"), "Sets the escape character")
        .def("set_multi_line_values",
             &inicpp::IniFileBase<
                 inicpp::StringInsensitiveLess>::setMultiLineValues,
             py::arg("enable"), "Enables or disables multi-line values")
        .def("allow_overwrite_duplicate_fields",
             &inicpp::IniFileBase<
                 inicpp::StringInsensitiveLess>::allowOverwriteDuplicateFields,
             py::arg("allowed"),
             "Allows or disallows overwriting duplicate fields")
        .def("decode",
             py::overload_cast<std::istream &>(
                 &inicpp::IniFileBase<inicpp::StringInsensitiveLess>::decode),
             py::arg("iss"), "Decodes an INI file from an input stream")
        .def("decode",
             py::overload_cast<const std::string &>(
                 &inicpp::IniFileBase<inicpp::StringInsensitiveLess>::decode),
             py::arg("content"), "Decodes an INI file from a string")
        .def("load", &inicpp::IniFileBase<inicpp::StringInsensitiveLess>::load,
             py::arg("file_name"),
             "Loads and decodes an INI file from a file path")
        // .def("encode", py::overload_cast<std::ostream&
        // const>(&inicpp::IniFileBase<inicpp::StringInsensitiveLess>::encode,
        // py::const_), py::arg("oss"), "Encodes the INI file to an output
        // stream")
        .def("encode",
             py::overload_cast<>(
                 &inicpp::IniFileBase<inicpp::StringInsensitiveLess>::encode,
                 py::const_),
             "Encodes the INI file to a string and returns it")
        .def("save", &inicpp::IniFileBase<inicpp::StringInsensitiveLess>::save,
             py::arg("file_name"), "Saves the INI file to a given file path");
}
