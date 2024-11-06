#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "atom/web/address.hpp"
#include "atom/web/curl.hpp"
#include "atom/web/downloader.hpp"
#include "atom/web/httpparser.hpp"
#include "atom/web/time.hpp"
#include "atom/web/utils.hpp"

namespace py = pybind11;
using namespace atom::web;

PYBIND11_MODULE(web, m) {
    py::class_<Address, std::shared_ptr<Address>>(m, "Address")
        .def("parse", &Address::parse, "Parse address string",
             py::arg("address"))
        .def("print_address_type", &Address::printAddressType,
             "Print address type")
        .def("is_in_range", &Address::isInRange, "Check if address is in range",
             py::arg("start"), py::arg("end"))
        .def("to_binary", &Address::toBinary,
             "Convert address to binary representation")
        .def("get_address", &Address::getAddress, "Get address string")
        .def("is_equal", &Address::isEqual, "Check if two addresses are equal",
             py::arg("other"))
        .def("get_type", &Address::getType, "Get address type")
        .def("get_network_address", &Address::getNetworkAddress,
             "Get network address", py::arg("mask"))
        .def("get_broadcast_address", &Address::getBroadcastAddress,
             "Get broadcast address", py::arg("mask"))
        .def("is_same_subnet", &Address::isSameSubnet,
             "Check if two addresses are in the same subnet", py::arg("other"),
             py::arg("mask"))
        .def("to_hex", &Address::toHex,
             "Convert address to hexadecimal representation");

    py::class_<IPv4, Address, std::shared_ptr<IPv4>>(m, "IPv4")
        .def(py::init<>())
        .def(py::init<const std::string&>(), py::arg("address"))
        .def("parse", &IPv4::parse, "Parse IPv4 address", py::arg("address"))
        .def("print_address_type", &IPv4::printAddressType,
             "Print IPv4 address type")
        .def("is_in_range", &IPv4::isInRange,
             "Check if IPv4 address is in range", py::arg("start"),
             py::arg("end"))
        .def("to_binary", &IPv4::toBinary,
             "Convert IPv4 address to binary representation")
        .def("is_equal", &IPv4::isEqual,
             "Check if two IPv4 addresses are equal", py::arg("other"))
        .def("get_type", &IPv4::getType, "Get IPv4 address type")
        .def("get_network_address", &IPv4::getNetworkAddress,
             "Get IPv4 network address", py::arg("mask"))
        .def("get_broadcast_address", &IPv4::getBroadcastAddress,
             "Get IPv4 broadcast address", py::arg("mask"))
        .def("is_same_subnet", &IPv4::isSameSubnet,
             "Check if two IPv4 addresses are in the same subnet",
             py::arg("other"), py::arg("mask"))
        .def("to_hex", &IPv4::toHex,
             "Convert IPv4 address to hexadecimal representation")
        .def("parse_cidr", &IPv4::parseCIDR,
             "Parse CIDR formatted IPv4 address", py::arg("cidr"));

    py::class_<IPv6, Address, std::shared_ptr<IPv6>>(m, "IPv6")
        .def(py::init<>())
        .def(py::init<const std::string&>(), py::arg("address"))
        .def("parse", &IPv6::parse, "Parse IPv6 address", py::arg("address"))
        .def("print_address_type", &IPv6::printAddressType,
             "Print IPv6 address type")
        .def("is_in_range", &IPv6::isInRange,
             "Check if IPv6 address is in range", py::arg("start"),
             py::arg("end"))
        .def("to_binary", &IPv6::toBinary,
             "Convert IPv6 address to binary representation")
        .def("is_equal", &IPv6::isEqual,
             "Check if two IPv6 addresses are equal", py::arg("other"))
        .def("get_type", &IPv6::getType, "Get IPv6 address type")
        .def("get_network_address", &IPv6::getNetworkAddress,
             "Get IPv6 network address", py::arg("mask"))
        .def("get_broadcast_address", &IPv6::getBroadcastAddress,
             "Get IPv6 broadcast address", py::arg("mask"))
        .def("is_same_subnet", &IPv6::isSameSubnet,
             "Check if two IPv6 addresses are in the same subnet",
             py::arg("other"), py::arg("mask"))
        .def("to_hex", &IPv6::toHex,
             "Convert IPv6 address to hexadecimal representation")
        .def("parse_cidr", &IPv6::parseCIDR,
             "Parse CIDR formatted IPv6 address", py::arg("cidr"));

    py::class_<UnixDomain, Address, std::shared_ptr<UnixDomain>>(m,
                                                                 "UnixDomain")
        .def(py::init<>())
        .def(py::init<const std::string&>(), py::arg("path"))
        .def("parse", &UnixDomain::parse, "Parse Unix domain socket address",
             py::arg("path"))
        .def("print_address_type", &UnixDomain::printAddressType,
             "Print Unix domain socket address type")
        .def("is_in_range", &UnixDomain::isInRange,
             "Check if Unix domain socket address is in range",
             py::arg("start"), py::arg("end"))
        .def("to_binary", &UnixDomain::toBinary,
             "Convert Unix domain socket address to binary representation")
        .def("is_equal", &UnixDomain::isEqual,
             "Check if two Unix domain socket addresses are equal",
             py::arg("other"))
        .def("get_type", &UnixDomain::getType,
             "Get Unix domain socket address type")
        .def("get_network_address", &UnixDomain::getNetworkAddress,
             "Get Unix domain socket network address", py::arg("mask"))
        .def("get_broadcast_address", &UnixDomain::getBroadcastAddress,
             "Get Unix domain socket broadcast address", py::arg("mask"))
        .def("is_same_subnet", &UnixDomain::isSameSubnet,
             "Check if two Unix domain socket addresses are in the same subnet",
             py::arg("other"), py::arg("mask"))
        .def(
            "to_hex", &UnixDomain::toHex,
            "Convert Unix domain socket address to hexadecimal representation");

    py::class_<CurlWrapper>(m, "CurlWrapper")
        .def(py::init<>())
        .def("set_url", &CurlWrapper::setUrl, "Set the URL for the request",
             py::arg("url"))
        .def("set_request_method", &CurlWrapper::setRequestMethod,
             "Set the HTTP request method", py::arg("method"))
        .def("add_header", &CurlWrapper::addHeader,
             "Add a header to the request", py::arg("key"), py::arg("value"))
        .def("on_error", &CurlWrapper::onError, "Set the error callback",
             py::arg("callback"))
        .def("on_response", &CurlWrapper::onResponse,
             "Set the response callback", py::arg("callback"))
        .def("set_timeout", &CurlWrapper::setTimeout, "Set the request timeout",
             py::arg("timeout"))
        .def("set_follow_location", &CurlWrapper::setFollowLocation,
             "Set whether to follow redirects", py::arg("follow"))
        .def("set_request_body", &CurlWrapper::setRequestBody,
             "Set the request body", py::arg("data"))
        .def("set_upload_file", &CurlWrapper::setUploadFile,
             "Set the file to upload", py::arg("file_path"))
        .def("set_proxy", &CurlWrapper::setProxy,
             "Set the proxy for the request", py::arg("proxy"))
        .def("set_ssl_options", &CurlWrapper::setSSLOptions, "Set SSL options",
             py::arg("verify_peer"), py::arg("verify_host"))
        .def("perform", &CurlWrapper::perform, "Perform the HTTP request")
        .def("perform_async", &CurlWrapper::performAsync,
             "Perform the HTTP request asynchronously")
        .def("wait_all", &CurlWrapper::waitAll,
             "Wait for all asynchronous requests to complete")
        .def("set_max_download_speed", &CurlWrapper::setMaxDownloadSpeed,
             "Set the maximum download speed", py::arg("speed"));

    py::class_<DownloadManager>(m, "DownloadManager")
        .def(py::init<const std::string&>(), "Constructor",
             py::arg("task_file"))
        .def("add_task", &DownloadManager::addTask, "Add a download task",
             py::arg("url"), py::arg("filepath"), py::arg("priority") = 0)
        .def("remove_task", &DownloadManager::removeTask,
             "Remove a download task", py::arg("index"))
        .def("start", &DownloadManager::start, "Start download tasks",
             py::arg("thread_count") = std::thread::hardware_concurrency(),
             py::arg("download_speed") = 0)
        .def("pause_task", &DownloadManager::pauseTask, "Pause a download task",
             py::arg("index"))
        .def("resume_task", &DownloadManager::resumeTask,
             "Resume a paused download task", py::arg("index"))
        .def("get_downloaded_bytes", &DownloadManager::getDownloadedBytes,
             "Get the number of bytes downloaded for a task", py::arg("index"))
        .def("cancel_task", &DownloadManager::cancelTask,
             "Cancel a download task", py::arg("index"))
        .def("set_thread_count", &DownloadManager::setThreadCount,
             "Set the number of download threads", py::arg("thread_count"))
        .def("set_max_retries", &DownloadManager::setMaxRetries,
             "Set the maximum number of retries for download errors",
             py::arg("retries"))
        .def("on_download_complete", &DownloadManager::onDownloadComplete,
             "Register a callback for when a download completes",
             py::arg("callback"))
        .def("on_progress_update", &DownloadManager::onProgressUpdate,
             "Register a callback for when download progress updates",
             py::arg("callback"));

    py::class_<HttpHeaderParser>(m, "HttpHeaderParser")
        .def(py::init<>())
        .def("parse_headers", &HttpHeaderParser::parseHeaders,
             "Parse raw HTTP headers", py::arg("raw_headers"))
        .def("set_header_value", &HttpHeaderParser::setHeaderValue,
             "Set the value of a specific header field", py::arg("key"),
             py::arg("value"))
        .def("set_headers", &HttpHeaderParser::setHeaders,
             "Set multiple header fields at once", py::arg("headers"))
        .def("add_header_value", &HttpHeaderParser::addHeaderValue,
             "Add a new value to an existing header field", py::arg("key"),
             py::arg("value"))
        .def("get_header_values", &HttpHeaderParser::getHeaderValues,
             "Retrieve the values of a specific header field", py::arg("key"))
        .def("remove_header", &HttpHeaderParser::removeHeader,
             "Remove a specific header field", py::arg("key"))
        .def("get_all_headers", &HttpHeaderParser::getAllHeaders,
             "Retrieve all the parsed headers")
        .def("has_header", &HttpHeaderParser::hasHeader,
             "Check if a specific header field exists", py::arg("key"))
        .def("clear_headers", &HttpHeaderParser::clearHeaders,
             "Clear all the parsed headers");

    py::class_<TimeManager>(m, "TimeManager")
        .def(py::init<>())
        .def("get_system_time", &TimeManager::getSystemTime,
             "Get the current system time")
        .def("set_system_time", &TimeManager::setSystemTime,
             "Set the system time", py::arg("year"), py::arg("month"),
             py::arg("day"), py::arg("hour"), py::arg("minute"),
             py::arg("second"))
        .def("set_system_timezone", &TimeManager::setSystemTimezone,
             "Set the system timezone", py::arg("timezone"))
        .def("sync_time_from_rtc", &TimeManager::syncTimeFromRTC,
             "Synchronize the system time from the Real-Time Clock (RTC)")
        .def("get_ntp_time", &TimeManager::getNtpTime,
             "Get the Network Time Protocol (NTP) time from a specified "
             "hostname",
             py::arg("hostname"));

    m.def("is_port_in_use", &isPortInUse, "Check if a port is in use",
          py::arg("port"));
    m.def("check_and_kill_program_on_port", &checkAndKillProgramOnPort,
          "Check if there is any program running on the specified port and "
          "kill it if found",
          py::arg("port"));

#if defined(__linux__) || defined(__APPLE__)
    m.def("dump_addr_info", &dumpAddrInfo,
          "Dump address information from source to destination", py::arg("dst"),
          py::arg("src"));
    m.def("addr_info_to_string", &addrInfoToString,
          "Convert address information to string", py::arg("addr_info"),
          py::arg("json_format") = false);
    m.def("get_addr_info", &getAddrInfo,
          "Get address information for a given hostname and service",
          py::arg("hostname"), py::arg("service"));
    m.def("free_addr_info", &freeAddrInfo, "Free address information",
          py::arg("addr_info"));
    m.def("compare_addr_info", &compareAddrInfo,
          "Compare two address information structures", py::arg("addr_info1"),
          py::arg("addr_info2"));
    m.def("filter_addr_info", &filterAddrInfo,
          "Filter address information by family", py::arg("addr_info"),
          py::arg("family"));
    m.def("sort_addr_info", &sortAddrInfo, "Sort address information by family",
          py::arg("addr_info"));
#endif
}