#include <pybind11/chrono.h>
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "atom/connection/async_fifoclient.hpp"
#include "atom/connection/async_fifoserver.hpp"
#include "atom/connection/async_sockethub.hpp"
#include "atom/connection/async_udpclient.hpp"
#include "atom/connection/async_udpserver.hpp"

#include "atom/connection/fifoclient.hpp"
#include "atom/connection/fifoserver.hpp"
#include "atom/connection/sockethub.hpp"
#if __has_include(<libssh/libssh.h>)
#include "atom/connection/sshclient.hpp"
#endif
#include "atom/connection/sshserver.hpp"
#include "atom/connection/tcpclient.hpp"
#include "atom/connection/ttybase.hpp"
#include "atom/connection/udpclient.hpp"
#include "atom/connection/udpserver.hpp"

namespace py = pybind11;
using namespace atom::connection;

PYBIND11_MODULE(connection, m) {
    m.doc() = "Atom Connection Module";

    py::class_<atom::async::connection::FifoClient>(m, "FifoClient")
        .def(py::init<std::string>(), py::arg("fifo_path"))
        .def("write", &atom::async::connection::FifoClient::write,
             py::arg("data"), py::arg("timeout") = std::nullopt,
             "Writes data to the FIFO with an optional timeout.")
        .def("read", &atom::async::connection::FifoClient::read,
             py::arg("timeout") = std::nullopt,
             "Reads data from the FIFO with an optional timeout.")
        .def("is_open", &atom::async::connection::FifoClient::isOpen,
             "Checks if the FIFO is currently open.")
        .def("close", &atom::async::connection::FifoClient::close,
             "Closes the FIFO.");

    py::class_<atom::async::connection::FifoServer>(m, "FifoServer")
        .def(py::init<std::string_view>(), py::arg("fifo_path"))
        .def("start", &atom::async::connection::FifoServer::start,
             "Starts the server to listen for messages.")
        .def("stop", &atom::async::connection::FifoServer::stop,
             "Stops the server.")
        .def("is_running", &atom::async::connection::FifoServer::isRunning,
             "Checks if the server is running.");

    py::class_<atom::async::connection::SocketHub>(m, "SocketHub")
        .def(py::init<bool>(), py::arg("use_ssl") = false)
        .def("start", &atom::async::connection::SocketHub::start,
             py::arg("port"), "Starts the socket hub on the specified port.")
        .def("stop", &atom::async::connection::SocketHub::stop,
             "Stops the socket hub.")
        .def("add_handler", &atom::async::connection::SocketHub::addHandler,
             py::arg("handler"),
             "Adds a message handler for incoming messages.")
        .def("add_connect_handler",
             &atom::async::connection::SocketHub::addConnectHandler,
             py::arg("handler"), "Adds a handler for new connections.")
        .def("add_disconnect_handler",
             &atom::async::connection::SocketHub::addDisconnectHandler,
             py::arg("handler"), "Adds a handler for disconnections.")
        .def("broadcast_message",
             &atom::async::connection::SocketHub::broadcastMessage,
             py::arg("message"),
             "Broadcasts a message to all connected clients.")
        .def("send_message_to_client",
             &atom::async::connection::SocketHub::sendMessageToClient,
             py::arg("client_id"), py::arg("message"),
             "Sends a message to a specific client.")
        .def("is_running", &atom::async::connection::SocketHub::isRunning,
             "Checks if the socket hub is currently running.");

    py::class_<atom::async::connection::UdpClient>(m, "UdpClient")
        .def(py::init<>())
        .def("bind", &atom::async::connection::UdpClient::bind, py::arg("port"),
             "Binds the client to a specific port for receiving data.")
        .def("send", &atom::async::connection::UdpClient::send, py::arg("host"),
             py::arg("port"), py::arg("data"),
             "Sends data to a specified host and port.")
        .def("receive", &atom::async::connection::UdpClient::receive,
             py::arg("size"), py::arg("remoteHost"), py::arg("remotePort"),
             py::arg("timeout") = std::chrono::milliseconds::zero(),
             "Receives data from a remote host.")
        .def("set_on_data_received_callback",
             &atom::async::connection::UdpClient::setOnDataReceivedCallback,
             py::arg("callback"),
             "Sets the callback function to be called when data is received.")
        .def("set_on_error_callback",
             &atom::async::connection::UdpClient::setOnErrorCallback,
             py::arg("callback"),
             "Sets the callback function to be called when an error occurs.")
        .def("start_receiving",
             &atom::async::connection::UdpClient::startReceiving,
             py::arg("bufferSize"), "Starts receiving data asynchronously.")
        .def("stop_receiving",
             &atom::async::connection::UdpClient::stopReceiving,
             "Stops receiving data.");

    py::class_<atom::async::connection::UdpSocketHub>(m, "UdpSocketHub")
        .def(py::init<>())
        .def("start", &atom::async::connection::UdpSocketHub::start,
             py::arg("port"),
             "Starts the UDP socket hub and binds it to the specified port.")
        .def("stop", &atom::async::connection::UdpSocketHub::stop,
             "Stops the UDP socket hub.")
        .def("is_running", &atom::async::connection::UdpSocketHub::isRunning,
             "Checks if the UDP socket hub is currently running.")
        .def("add_message_handler",
             &atom::async::connection::UdpSocketHub::addMessageHandler,
             py::arg("handler"),
             "Adds a message handler function to the UDP socket hub.")
        .def("remove_message_handler",
             &atom::async::connection::UdpSocketHub::removeMessageHandler,
             py::arg("handler"),
             "Removes a message handler function from the UDP socket hub.")
        .def("send_to", &atom::async::connection::UdpSocketHub::sendTo,
             py::arg("message"), py::arg("ip"), py::arg("port"),
             "Sends a message to the specified IP address and port.");

    py::class_<FifoClient>(m, "FifoClient")
        .def(py::init<std::string>(), py::arg("fifo_path"))
        .def("write", &FifoClient::write, py::arg("data"),
             py::arg("timeout") = std::nullopt,
             "Writes data to the FIFO with an optional timeout.")
        .def("read", &FifoClient::read, py::arg("timeout") = std::nullopt,
             "Reads data from the FIFO with an optional timeout.")
        .def("is_open", &FifoClient::isOpen,
             "Checks if the FIFO is currently open.")
        .def("close", &FifoClient::close, "Closes the FIFO.");

    py::class_<FIFOServer>(m, "FIFOServer")
        .def(py::init<std::string_view>(), py::arg("fifo_path"))
        .def("send_message", &FIFOServer::sendMessage, py::arg("message"),
             "Sends a message through the FIFO pipe.")
        .def("start", &FIFOServer::start, "Starts the FIFO server.")
        .def("stop", &FIFOServer::stop, "Stops the FIFO server.")
        .def("is_running", &FIFOServer::isRunning,
             "Checks if the FIFO server is running.");

    py::class_<SocketHub>(m, "SocketHub")
        .def(py::init<>())
        .def("start", &SocketHub::start, py::arg("port"),
             "Starts the socket service on the specified port.")
        .def("stop", &SocketHub::stop, "Stops the socket service.")
        .def("add_handler", &SocketHub::addHandler, py::arg("handler"),
             "Adds a message handler for incoming messages.")
        .def("is_running", &SocketHub::isRunning,
             "Checks if the socket service is running.");

#if __has_include(<libssh/libssh.h>)
    py::class_<SSHClient>(m, "SSHClient")
        .def(py::init<const std::string&, int>(), py::arg("host"),
             py::arg("port") = DEFAULT_SSH_PORT)
        .def("connect", &SSHClient::connect, py::arg("username"),
             py::arg("password"), py::arg("timeout") = DEFAULT_TIMEOUT,
             "Connects to the SSH server with the specified username and "
             "password.")
        .def("is_connected", &SSHClient::isConnected,
             "Checks if the SSH client is connected to the server.")
        .def("disconnect", &SSHClient::disconnect,
             "Disconnects from the SSH server.")
        .def("execute_command", &SSHClient::executeCommand, py::arg("command"),
             py::arg("output"), "Executes a single command on the SSH server.")
        .def("execute_commands", &SSHClient::executeCommands,
             py::arg("commands"), py::arg("output"),
             "Executes multiple commands on the SSH server.")
        .def("file_exists", &SSHClient::fileExists, py::arg("remote_path"),
             "Checks if a file exists on the remote server.")
        .def("create_directory", &SSHClient::createDirectory,
             py::arg("remote_path"), py::arg("mode") = DEFAULT_MODE,
             "Creates a directory on the remote server.")
        .def("remove_file", &SSHClient::removeFile, py::arg("remote_path"),
             "Removes a file from the remote server.")
        .def("remove_directory", &SSHClient::removeDirectory,
             py::arg("remote_path"),
             "Removes a directory from the remote server.")
        .def("list_directory", &SSHClient::listDirectory,
             py::arg("remote_path"),
             "Lists the contents of a directory on the remote server.")
        .def("rename", &SSHClient::rename, py::arg("old_path"),
             py::arg("new_path"),
             "Renames a file or directory on the remote server.")
        .def("get_file_info", &SSHClient::getFileInfo, py::arg("remote_path"),
             py::arg("attrs"), "Retrieves file information for a remote file.")
        .def("download_file", &SSHClient::downloadFile, py::arg("remote_path"),
             py::arg("local_path"), "Downloads a file from the remote server.")
        .def("upload_file", &SSHClient::uploadFile, py::arg("local_path"),
             py::arg("remote_path"), "Uploads a file to the remote server.")
        .def("upload_directory", &SSHClient::uploadDirectory,
             py::arg("local_path"), py::arg("remote_path"),
             "Uploads a directory to the remote server.");
#endif

    py::class_<SshServer>(m, "SshServer")
        .def(py::init<const std::filesystem::path&>(), py::arg("config_file"))
        .def("start", &SshServer::start, "Starts the SSH server.")
        .def("stop", &SshServer::stop, "Stops the SSH server.")
        .def("is_running", &SshServer::isRunning,
             "Checks if the SSH server is running.")
        .def("set_port", &SshServer::setPort, py::arg("port"),
             "Sets the port on which the SSH server listens for connections.")
        .def("get_port", &SshServer::getPort,
             "Gets the port on which the SSH server is listening.")
        .def(
            "set_listen_address", &SshServer::setListenAddress,
            py::arg("address"),
            "Sets the address on which the SSH server listens for connections.")
        .def("get_listen_address", &SshServer::getListenAddress,
             "Gets the address on which the SSH server is listening.")
        .def("set_host_key", &SshServer::setHostKey, py::arg("key_file"),
             "Sets the host key file used for SSH connections.")
        .def("get_host_key", &SshServer::getHostKey,
             "Gets the path to the host key file.")
        .def("set_authorized_keys", &SshServer::setAuthorizedKeys,
             py::arg("key_files"),
             "Sets the list of authorized public key files for user "
             "authentication.")
        .def("get_authorized_keys", &SshServer::getAuthorizedKeys,
             "Gets the list of authorized public key files.")
        .def("allow_root_login", &SshServer::allowRootLogin, py::arg("allow"),
             "Enables or disables root login to the SSH server.")
        .def("is_root_login_allowed", &SshServer::isRootLoginAllowed,
             "Checks if root login is allowed.")
        .def("set_password_authentication",
             &SshServer::setPasswordAuthentication, py::arg("enable"),
             "Enables or disables password authentication for the SSH server.")
        .def("is_password_authentication_enabled",
             &SshServer::isPasswordAuthenticationEnabled,
             "Checks if password authentication is enabled.")
        .def("set_subsystem", &SshServer::setSubsystem, py::arg("name"),
             py::arg("command"),
             "Sets a subsystem for handling a specific command.")
        .def("remove_subsystem", &SshServer::removeSubsystem, py::arg("name"),
             "Removes a previously set subsystem by name.")
        .def("get_subsystem", &SshServer::getSubsystem, py::arg("name"),
             "Gets the command associated with a subsystem by name.");

    py::class_<TcpClient>(m, "TcpClient")
        .def(py::init<>())
        .def("connect", &TcpClient::connect, py::arg("host"), py::arg("port"),
             py::arg("timeout") = std::chrono::milliseconds::zero(),
             "Connects to a TCP server.")
        .def("disconnect", &TcpClient::disconnect,
             "Disconnects from the server.")
        .def("send", &TcpClient::send, py::arg("data"),
             "Sends data to the server.")
        .def("receive", &TcpClient::receive, py::arg("size"),
             py::arg("timeout") = std::chrono::milliseconds::zero(),
             "Receives data from the server.")
        .def("is_connected", &TcpClient::isConnected,
             "Checks if the client is connected to the server.")
        .def("get_error_message", &TcpClient::getErrorMessage,
             "Gets the error message in case of any error.")
        .def("set_on_connected_callback", &TcpClient::setOnConnectedCallback,
             py::arg("callback"),
             "Sets the callback function to be called when connected to the "
             "server.")
        .def("set_on_disconnected_callback",
             &TcpClient::setOnDisconnectedCallback, py::arg("callback"),
             "Sets the callback function to be called when disconnected from "
             "the server.")
        .def("set_on_data_received_callback",
             &TcpClient::setOnDataReceivedCallback, py::arg("callback"),
             "Sets the callback function to be called when data is received "
             "from the server.")
        .def("set_on_error_callback", &TcpClient::setOnErrorCallback,
             py::arg("callback"),
             "Sets the callback function to be called when an error occurs.")
        .def("start_receiving", &TcpClient::startReceiving,
             py::arg("buffer_size"), "Starts receiving data from the server.")
        .def("stop_receiving", &TcpClient::stopReceiving,
             "Stops receiving data from the server.");

    py::class_<TTYBase>(m, "TTYBase")
        .def(py::init<std::string_view>(), py::arg("driver_name"))
        .def("read", &TTYBase::read, py::arg("buffer"), py::arg("nbytes"),
             py::arg("timeout"), py::arg("nbytes_read"),
             "Reads data from the TTY device.")
        .def("read_section", &TTYBase::readSection, py::arg("buffer"),
             py::arg("nsize"), py::arg("stop_byte"), py::arg("timeout"),
             py::arg("nbytes_read"),
             "Reads a section of data from the TTY until a stop byte is "
             "encountered.")
        .def("write", &TTYBase::write, py::arg("buffer"), py::arg("nbytes"),
             py::arg("nbytes_written"), "Writes data to the TTY device.")
        .def("write_string", &TTYBase::writeString, py::arg("string"),
             py::arg("nbytes_written"), "Writes a string to the TTY device.")
        .def("connect", &TTYBase::connect, py::arg("device"),
             py::arg("bit_rate"), py::arg("word_size"), py::arg("parity"),
             py::arg("stop_bits"), "Connects to the specified TTY device.")
        .def("disconnect", &TTYBase::disconnect,
             "Disconnects from the TTY device.")
        .def("set_debug", &TTYBase::setDebug, py::arg("enabled"),
             "Enables or disables debugging information.")
        .def("get_error_message", &TTYBase::getErrorMessage, py::arg("code"),
             "Retrieves an error message corresponding to a given TTYResponse "
             "code.")
        .def("get_port_fd", &TTYBase::getPortFD,
             "Gets the file descriptor for the TTY port.");

    py::enum_<TTYBase::TTYResponse>(m, "TTYResponse")
        .value("OK", TTYBase::TTYResponse::OK)
        .value("ReadError", TTYBase::TTYResponse::ReadError)
        .value("WriteError", TTYBase::TTYResponse::WriteError)
        .value("SelectError", TTYBase::TTYResponse::SelectError)
        .value("Timeout", TTYBase::TTYResponse::Timeout)
        .value("PortFailure", TTYBase::TTYResponse::PortFailure)
        .value("ParamError", TTYBase::TTYResponse::ParamError)
        .value("Errno", TTYBase::TTYResponse::Errno)
        .value("Overflow", TTYBase::TTYResponse::Overflow);

    py::class_<UdpClient>(m, "UdpClient")
        .def(py::init<>())
        .def("bind", &UdpClient::bind, py::arg("port"),
             "Binds the client to a specific port for receiving data.")
        .def("send", &UdpClient::send, py::arg("host"), py::arg("port"),
             py::arg("data"), "Sends data to a specified host and port.")
        .def("receive", &UdpClient::receive, py::arg("size"),
             py::arg("remote_host"), py::arg("remote_port"),
             py::arg("timeout") = std::chrono::milliseconds::zero(),
             "Receives data from a remote host.")
        .def("set_on_data_received_callback",
             &UdpClient::setOnDataReceivedCallback, py::arg("callback"),
             "Sets the callback function to be called when data is received.")
        .def("set_on_error_callback", &UdpClient::setOnErrorCallback,
             py::arg("callback"),
             "Sets the callback function to be called when an error occurs.")
        .def("start_receiving", &UdpClient::startReceiving,
             py::arg("buffer_size"), "Starts receiving data asynchronously.")
        .def("stop_receiving", &UdpClient::stopReceiving,
             "Stops receiving data.");

    py::class_<UdpSocketHub>(m, "UdpSocketHub")
        .def(py::init<>())
        .def("start", &UdpSocketHub::start, py::arg("port"),
             "Starts the UDP socket hub and binds it to the specified port.")
        .def("stop", &UdpSocketHub::stop, "Stops the UDP socket hub.")
        .def("is_running", &UdpSocketHub::isRunning,
             "Checks if the UDP socket hub is currently running.")
        .def("add_message_handler", &UdpSocketHub::addMessageHandler,
             py::arg("handler"),
             "Adds a message handler function to the UDP socket hub.")
        .def("remove_message_handler", &UdpSocketHub::removeMessageHandler,
             py::arg("handler"),
             "Removes a message handler function from the UDP socket hub.")
        .def("send_to", &UdpSocketHub::sendTo, py::arg("message"),
             py::arg("ip"), py::arg("port"),
             "Sends a message to the specified IP address and port.");
}