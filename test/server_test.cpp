#include <iostream>
#include <string>
#include <uv.h>
#include <functional>

class TcpServer
{
public:
    TcpServer(const std::string &address, int port)
        : loop_(uv_default_loop()),
          server_addr_(),
          server_handle_(),
          on_receive_data(nullptr)
    {

        uv_ip4_addr(address.c_str(), port, &server_addr_);
        server_handle_.data = this;
    }

    ~TcpServer()
    {
        uv_stop(loop_);
        uv_loop_close(loop_);
    }

    bool Start()
    {
        if (uv_tcp_init(loop_, &server_handle_) != 0)
        {
            std::cerr << "Failed to initialize server." << std::endl;
            return false;
        }

        if (uv_tcp_bind(&server_handle_, (const struct sockaddr *)&server_addr_, 0) != 0)
        {
            std::cerr << "Failed to bind server." << std::endl;
            return false;
        }

        if (uv_listen((uv_stream_t *)&server_handle_, SOMAXCONN, OnNewConnection) != 0)
        {
            std::cerr << "Failed to listen on server." << std::endl;
            return false;
        }

        std::cout << "Server started listening on " << GetAddress() << ":" << GetPort() << std::endl;
        uv_run(loop_, UV_RUN_DEFAULT);
        return true;
    }

    bool Send(uv_stream_t *client, const void *data, size_t size)
    {
        uv_buf_t buffer = uv_buf_init(const_cast<char *>(reinterpret_cast<const char *>(data)), size);
        uv_write_t *write_req = new uv_write_t();
        write_req->data = nullptr;

        if (uv_write(write_req, client, &buffer, 1, OnWrite) != 0)
        {
            std::cerr << "Failed to send data." << std::endl;
            delete write_req;
            return false;
        }

        return true;
    }

    std::string GetAddress() const
    {
        char ip[17] = {'\0'};
        uv_ip4_name(&server_addr_, ip, sizeof(ip));
        return std::string(ip);
    }

    int GetPort() const
    {
        return ntohs(server_addr_.sin_port);
    }

    void SetOnReceiveData(const std::function<void(uv_stream_t *, ssize_t, const uv_buf_t *)> &callback)
    {
        on_receive_data = callback;
    }

private:
    static void OnNewConnection(uv_stream_t *server, int status)
    {
        if (status < 0)
        {
            std::cerr << "New connection error: " << uv_strerror(status) << std::endl;
            return;
        }

        TcpServer *tcp_server = static_cast<TcpServer *>(server->data);

        uv_tcp_t *client_handle = new uv_tcp_t();
        uv_tcp_init(tcp_server->loop_, client_handle);

        if (uv_accept(server, (uv_stream_t *)client_handle) == 0)
        {
            uv_read_start((uv_stream_t *)client_handle, AllocBuffer, OnRead);
        }
        else
        {
            uv_close((uv_handle_t *)client_handle, OnClientClose);
        }
    }

    static void AllocBuffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
    {
        buf->base = new char[suggested_size];
        buf->len = suggested_size;
    }

    static void OnRead(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
    {
        if (nread > 0)
        {
            TcpServer *tcp_server = static_cast<TcpServer *>(client->data);
            if (tcp_server->on_receive_data)
            {
                tcp_server->on_receive_data(client, nread, buf);
            }
        }
        else if (nread < 0)
        {
            delete[] buf->base;
            uv_close((uv_handle_t *)client, OnClientClose);
        }
    }

    static void OnWrite(uv_write_t *req, int status)
    {
        delete[] req->data;
        delete req;
    }

    static void OnClientClose(uv_handle_t *handle)
    {
        delete handle;
    }

    uv_loop_t *loop_;
    sockaddr_in server_addr_;
    uv_tcp_t server_handle_;
    std::function<void(uv_stream_t *, ssize_t, const uv_buf_t *)> on_receive_data;
};

int main()
{
    TcpServer server("0.0.0.0", 12345);

    server.SetOnReceiveData([&server](uv_stream_t *client, ssize_t size, const uv_buf_t *buf)
                            {
        std::cout << "Received data: " << std::string(buf->base, size) << std::endl;

        std::string response = "Response";
        server.Send(client, response.data(), response.size());
        
        delete[] buf->base; });

    if (!server.Start())
    {
        return 1;
    }

    return 0;
}
