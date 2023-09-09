#include <uv.h>
#include <iostream>
#include <string>
#include <thread>
#include <functional>
#include <vector>

class TcpServer
{
public:
    TcpServer(const std::string &address, int port)
        : loop_(uv_default_loop()), server_handle_(), client_handles_(), loop_thread_(),
          on_receive_data(nullptr)
    {
        server_handle_.data = this;
        uv_tcp_init(loop_, &server_handle_);

        sockaddr_in addr;
        uv_ip4_addr(address.c_str(), port, &addr);

        if (uv_tcp_bind(&server_handle_, reinterpret_cast<const sockaddr *>(&addr), 0) != 0)
        {
            std::cerr << "Failed to bind server." << std::endl;
        }
    }

    ~TcpServer()
    {
        uv_stop(loop_);
        loop_thread_.join();
    }

    // 启动服务器
    bool Start()
    {
        if (uv_listen(reinterpret_cast<uv_stream_t *>(&server_handle_), SOMAXCONN, OnNewConnection) != 0)
        {
            std::cerr << "Failed to listen on server." << std::endl;
            return false;
        }

        loop_thread_ = std::thread([&]()
                                   { uv_run(loop_, UV_RUN_DEFAULT); });

        return true;
    }

    // 发送数据
    bool Send(uv_stream_t *client, const void *data, size_t size)
    {
        uv_buf_t buffer = uv_buf_init(const_cast<char *>(reinterpret_cast<const char *>(data)), size);
        uv_write_t *request = new uv_write_t;

        if (uv_write(request, client, &buffer, 1, OnWrite) != 0)
        {
            std::cerr << "Failed to send data." << std::endl;
            delete request;
            return false;
        }

        return true;
    }

    // 接收数据回调函数
    std::function<void(uv_stream_t *, ssize_t, const uv_buf_t *)> on_receive_data;

private:
    static void OnNewConnection(uv_stream_t *server, int status)
    {
        if (status < 0)
        {
            std::cerr << "New connection error: " << uv_strerror(status) << std::endl;
            return;
        }

        TcpServer *tcp_server = reinterpret_cast<TcpServer *>(server->data);

        uv_tcp_t client_handle;
        uv_tcp_init(tcp_server->loop_, &client_handle);

        if (uv_accept(server, reinterpret_cast<uv_stream_t *>(&client_handle)) == 0)
        {
            tcp_server->client_handles_.push_back(client_handle);
            uv_read_start(reinterpret_cast<uv_stream_t *>(&client_handle), OnAllocBuffer, OnRead);
        }
        else
        {
            uv_close(reinterpret_cast<uv_handle_t *>(&client_handle), nullptr);
        }
    }

    static void OnAllocBuffer(uv_handle_t *handle, size_t size, uv_buf_t *buffer)
    {
        *buffer = uv_buf_init(new char[size], size);
    }

    static void OnRead(uv_stream_t *client, ssize_t nread, const uv_buf_t *buffer)
    {
        if (nread > 0)
        {
            TcpServer *tcp_server = reinterpret_cast<TcpServer *>(client->data);
            if (tcp_server->on_receive_data)
            {
                tcp_server->on_receive_data(client, nread, buffer);
            }
        }
        else if (nread < 0)
        {

            uv_close(reinterpret_cast<uv_handle_t *>(client), OnClientClose);
        }

        delete[] buffer->base;
    }

    static void OnWrite(uv_write_t *request, int status)
    {
        delete request->bufs[0].base;
    delete request;
    }

    static void OnClientClose(uv_handle_t *handle)
    {
        delete handle;
    }

    uv_loop_t *loop_;
    uv_tcp_t server_handle_;
    std::vector<uv_tcp_t> client_handles_;

    std::thread loop_thread_;
};

int main()
{
    TcpServer server("0.0.0.0", 12345);

    // 设置接收数据的回调函数
    server.on_receive_data = [&server](uv_stream_t *client, ssize_t size, const uv_buf_t *buffer)
    {
        std::cout << "Received data: " << std::string(buffer->base, size) << std::endl;
        // 处理接收到的数据

        // 发送响应数据
        std::string response = "Response";
        server.Send(client, response.data(), response.size());
    };

    // 启动服务器
    if (!server.Start())
    {
        return 1;
    }

    char input;
    std::cout << "请输入一个字符：";

    std::cin >> input;

    // 主线程执行其他逻辑
    // ...

    return 0;
}
