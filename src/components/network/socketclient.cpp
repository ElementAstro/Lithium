#include "socketclient.hpp"

#include <spdlog/spdlog.h>

TcpClient::TcpClient(const std::string &host, const std::string &port)
    : socket_(io_context_), host_(host), port_(port)
{
}

TcpClient::~TcpClient() {}

bool TcpClient::Connect()
{
    try
    {
        tcp::resolver resolver(io_context_);
        auto endpoints = resolver.resolve(host_, port_);

        boost::asio::connect(socket_, endpoints);
        spdlog::info("Connected to {}:{}", host_, port_);
        return true;
    }
    catch (std::exception &e)
    {
        spdlog::error("Failed to connect to {}:{}. Error message: {}", host_, port_, e.what());
        return false;
    }
}

bool TcpClient::Send(const json &data)
{
    try
    {
        std::string message = data.dump();
        boost::asio::write(socket_, boost::asio::buffer(message));
        spdlog::debug("Sent data to server: {}", message);
        return true;
    }
    catch (std::exception &e)
    {
        spdlog::error("Failed to send data to server. Error message: {}", e.what());
        return false;
    }
}

bool TcpClient::Receive(json &response, int timeout)
{
    try
    {
        if (timeout > 0)
        {
            //socket_.set_option(boost::asio::socket_base::receive_timeout(boost::asio::chrono::seconds(timeout)));
        }

        boost::asio::streambuf receive_buffer;
        boost::system::error_code error;

        size_t available_bytes = socket_.available(error);

        if (!error && available_bytes > 0)
        {
            boost::asio::read_until(socket_, receive_buffer, '\n');

            std::istream input(&receive_buffer);
            std::string message;
            std::getline(input, message);

            response = json::parse(message);
            spdlog::debug("Received data from server: {}", response.dump());
            return true;
        }
        else
        {
            spdlog::warn("No data received from server. Error message: {}", error.message());
            return false;
        }
    }
    catch (std::exception &e)
    {
        spdlog::error("Failed to receive data from server. Error message: {}", e.what());
        return false;
    }
}

void TcpClient::Disconnect()
{
    if (socket_.is_open())
    {
        boost::system::error_code ec;
        socket_.shutdown(tcp::socket::shutdown_both, ec);
        socket_.close();
        spdlog::info("Disconnected from server.");
    }
}

bool TcpClient::CheckServerExistence()
{
    tcp::resolver resolver(io_context_);
    auto endpoints = resolver.resolve(host_, port_);

    tcp::socket temp_socket(io_context_);
    boost::asio::connect(temp_socket, endpoints);

    if (temp_socket.is_open())
    {
        temp_socket.close();
        spdlog::info("Server {}:{} exists.", host_, port_);
        return true;
    }
    else
    {
        spdlog::error("Server {}:{} does not exist.", host_, port_);
        return false;
    }
}

bool TcpClient::IsConnected()
{
    return socket_.is_open();
}

void TcpClient::SetTimeout(int timeout)
{
    
}

void TcpClient::ClearSocket()
{
    boost::system::error_code error;
    size_t available_bytes = socket_.available(error);

    if (!error && available_bytes > 0)
    {
        std::vector<char> buffer(available_bytes);
        socket_.receive(boost::asio::buffer(buffer));
    }
}

void TcpClient::SetRecvBufferSize(int size)
{
    socket_.set_option(boost::asio::socket_base::receive_buffer_size(size));
}

void TcpClient::SetSendBufferSize(int size)
{
    socket_.set_option(boost::asio::socket_base::send_buffer_size(size));
}

void TcpClient::SetKeepAlive(bool enable)
{
    socket_.set_option(boost::asio::socket_base::keep_alive(enable));
}

void TcpClient::SetLinger(bool enable, int linger_time)
{
    socket_.set_option(boost::asio::socket_base::linger(enable, linger_time));
}

void TcpClient::SetNoDelay(bool enable)
{
    socket_.set_option(boost::asio::ip::tcp::no_delay(enable));
}

tcp::endpoint TcpClient::GetLocalEndpoint()
{
    return socket_.local_endpoint();
}
