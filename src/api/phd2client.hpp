/*
 * phd2client.hpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-5-25

Description: PHD2 CLient Interface

**************************************************/

#ifndef PHD2_CLIENT_HPP
#define PHD2_CLIENT_HPP

#include <boost/asio.hpp>
#include <memory>
#include "json.hpp"

using boost::asio::ip::tcp;
using nlohmann::json;

class PHD2Client : public std::enable_shared_from_this<PHD2Client>
{
public:
    PHD2Client(boost::asio::io_context &io_context, const std::string &host, const unsigned short port);

    ~PHD2Client();

    void run();

    void send(const json &data);

private:
    void read();

    void handle_data(const json &data);

private:
    tcp::resolver resolver_;
    tcp::socket socket_;
    std::string host_;
    unsigned short port_;
    enum
    {
        max_length = 1024
    };
    std::array<char, max_length> buffer_;
};

#endif // PHD2_CLIENT_HPP
/*
#include <iostream>
#include <boost/asio.hpp>
#include <memory>
#include "json.hpp"
#include "phd2_client.hpp"

using namespace std;
using boost::asio::io_context;
using nlohmann::json;

int main()
{
    io_context io_ctx;

    auto client = make_shared<PHD2Client>(io_ctx, "localhost", 4400);
    client->run();

    // 发送指令
    json command = {
        {"method", "get_status"},
        {"params", {}},
        {"id", 1}
    };
    client->send(command);

    io_ctx.run();

    return 0;
}

*/
