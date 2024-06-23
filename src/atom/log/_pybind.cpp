#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "logger.hpp"

namespace py = pybind11;

PYBIND11_MODULE(log_manager, m) {
    py::class_<lithium::LogEntry>(m, "LogEntry")
        .def_readwrite("file_name", &lithium::LogEntry::fileName)
        .def_readwrite("line_number", &lithium::LogEntry::lineNumber)
        .def_readwrite("message", &lithium::LogEntry::message);

    py::class_<lithium::LoggerManager>(m, "LoggerManager")
        .def(py::init<>())
        .def("scan_logs_folder", &lithium::LoggerManager::scanLogsFolder)
        .def("search_logs", &lithium::LoggerManager::searchLogs)
        .def("upload_file", &lithium::LoggerManager::uploadFile)
        .def("analyze_logs", &lithium::LoggerManager::analyzeLogs);
}