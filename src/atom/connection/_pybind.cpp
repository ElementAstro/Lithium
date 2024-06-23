/*
 * _pybind.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-13

Description: Python Binding of Atom-Connection

**************************************************/

#include <pybind11/pybind11.h>

#include "fifoclient.hpp"
#include "fifoserver.hpp"
#include "sockethub.hpp"
#include "udp_server.hpp"

#if ENABLE_SSHCLIENT
#include "sshclient.hpp"
#endif
namespace py = pybind11;

using namespace atom::connection;

PYBIND11_EMBEDDED_MODULE(atom_connection, m) {
    m.doc() = "Atom Connection Python Binding";

    py::class_<FifoClient>(m, "FifoClient")
        .def(py::init<const std::string &>())
        .def("write", &FifoClient::write)
        .def("read", &FifoClient::read);

    py::class_<FIFOServer>(m, "FIFOServer")
        .def(py::init<const std::string &>())
        .def("sendMessage", &FIFOServer::sendMessage);
    py::class_<UdpSocketHub>(m, "UdpSocketHub")
        .def(py::init<>())
        .def("start", &UdpSocketHub::start, py::arg("port"))
        .def("stop", &UdpSocketHub::stop)
        .def("isRunning", &UdpSocketHub::isRunning)
        .def("addMessageHandler", &UdpSocketHub::addMessageHandler)
        .def("removeMessageHandler", &UdpSocketHub::removeMessageHandler)
        .def("sendTo", &UdpSocketHub::sendTo, py::arg("message"), py::arg("ip"),
             py::arg("port"));

#if ENABLE_SSHCLIENT
    py::class_<SSHClient>(m, "SSHClient")
        .def(py::init<const std::string &, int>())
        .def("Connect", &SSHClient::Connect, py::arg("username"),
             py::arg("password"), py::arg("timeout") = 10)
        .def("IsConnected", &SSHClient::IsConnected)
        .def("Disconnect", &SSHClient::Disconnect)
        .def("ExecuteCommand", &SSHClient::ExecuteCommand)
        .def("ExecuteCommands", &SSHClient::ExecuteCommands)
        .def("FileExists", &SSHClient::FileExists)
        .def("CreateDirectory", &SSHClient::CreateDirectory,
             py::arg("remote_path"), py::arg("mode") = S_NORMAL)
        .def("RemoveFile", &SSHClient::RemoveFile)
        .def("RemoveDirectory", &SSHClient::RemoveDirectory)
        .def("ListDirectory", &SSHClient::ListDirectory)
        .def("Rename", &SSHClient::Rename)
        .def("GetFileInfo", &SSHClient::GetFileInfo)
        .def("DownloadFile", &SSHClient::DownloadFile)
        .def("UploadFile", &SSHClient::UploadFile);
#endif

    py::class_<SocketHub>(m, "SocketHub")
        .def(py::init<>())
        .def("start", &SocketHub::start)
        .def("stop", &SocketHub::stop)
        .def("addHandler", &SocketHub::addHandler);
}
