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
#include "shared_memory.hpp"
#include "sockethub.hpp"
#include "udp_server.hpp"

#if ENABLE_SSHCLIENT
#include "sshclient.hpp"
#endif
namespace py = pybind11;

using namespace Atom::Connection;

template <typename T>
void bind_shared_memory(py::module &m, const std::string &type_name) {
    py::class_<SharedMemory<T>>(m, ("shared_memory_" + type_name).c_str())
        .def(py::init<const std::string &, bool>())
        .def("write", &SharedMemory<T>::write, py::arg("data"),
             py::arg("timeout") = std::chrono::milliseconds(0))
        .def("read", &SharedMemory<T>::read,
             py::arg("timeout") = std::chrono::milliseconds(0))
        .def("clear", &SharedMemory<T>::clear)
        .def("isOccupied", &SharedMemory<T>::isOccupied);
}

PYBIND11_MODULE(atom_connection, m) {
    m.doc() = "Atom Connection Python Binding";

    py::class_<FifoClient>(m, "FifoClient")
        .def(py::init<const std::string &>())
        .def("write", &FifoClient::write)
        .def("read", &FifoClient::read);

    py::class_<FIFOServer>(m, "FIFOServer")
        .def(py::init<const std::string &>())
        .def("sendMessage", &FIFOServer::sendMessage);

    bind_shared_memory<int>(m, "int");
    bind_shared_memory<float>(m, "float");
    bind_shared_memory<double>(m, "double");

    py::class_<UdpSocketHub>(m, "UdpSocketHub")
        .def(py::init<>())
        .def("start", &UdpSocketHub::start, py::arg("port"))
        .def("stop", &UdpSocketHub::stop)
        .def("addHandler", &UdpSocketHub::addHandler)
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
