/*
 * astrometry.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
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

Date: 2023-4-9

Description: Astrometry Command Line

**************************************************/

#include "astrometry.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif
#include <stdexcept>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <chrono>
#include <algorithm>
#include <ctime>

#ifdef _WIN32
#include <windows.h>
#endif

namespace Lithium::API::Astrometry
{
    using json = nlohmann::json;

    // 命令行工具路径
    constexpr char kSolveFieldPath[] = "/usr/local/astrometry/bin/solve-field";
    // 最大命令行缓冲区长度，防止缓冲区溢出
    constexpr int kMaxBufferSize = 4096;
    // 命令行执行超时时间
    constexpr int kDefaultTimeoutSeconds = 30;
    // 命令行执行状态码
    enum class CommandStatus
    {
        SUCCESS,
        FAILED,
        TIMEOUT
    };

    // 解析出的参数结构体
    struct SolveResult
    {
        std::string ra;
        std::string dec;
        double fov_x = 0;
        double fov_y = 0;
        double rotation = 0;
    };

    // 程序内部异常类型
    class AstrometryException : public std::runtime_error
    {
    public:
        explicit AstrometryException(const std::string &message)
            : std::runtime_error(message)
        {
        }
    };

    // UTC时间转换为字符串
    std::string to_string(const std::tm &tm, const std::string &format)
    {
        std::stringstream ss;
        char buffer[256];
        if (strftime(buffer, sizeof(buffer), format.c_str(), &tm) == 0)
        {
            throw AstrometryException("Date format error");
        }
        ss << buffer;
        return ss.str();
    }

    // 获取当前UTC时间
    std::string get_utc_time()
    {
        const auto now = std::chrono::system_clock::now();
        const std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
        struct std::tm tm;
#ifdef _WIN32
        gmtime_s(&tm, &now_time_t);
#else
        gmtime_r(&now_time_t, &tm);
#endif
        return to_string(tm, "%FT%TZ");
    }

    CommandStatus execute_command(const std::string &command, int timeout_seconds, std::string &output)
    {
        std::array<char, kMaxBufferSize> buffer{};
        std::unique_ptr<FILE, decltype(&pclose)> pipe(nullptr, nullptr);

#ifdef _WIN32
        pipe.reset(_popen(command.c_str(), "r"));
#else
        pipe.reset(popen(command.c_str(), "r"));
#endif

        if (!pipe)
        {
            throw AstrometryException("Failed to open pipe");
        }

        auto start_time = std::chrono::system_clock::now();
        while (std::chrono::system_clock::now() - start_time < std::chrono::seconds(timeout_seconds))
        {
            std::memset(buffer.data(), 0, buffer.size());
            if (std::fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
            {
                output += buffer.data();
            }
            else
            {
                break;
            }
        }

        auto status = pclose(pipe.get());

#ifdef _WIN32
        if (status == 0)
        {
            return CommandStatus::SUCCESS;
        }
        else if (status != -1)
        {
            return CommandStatus::FAILED;
        }
#else
        if (status == 0)
        {
            return CommandStatus::SUCCESS;
        }
        else if (WIFEXITED(status))
        {
            return CommandStatus::FAILED;
        }
#endif
        return CommandStatus::TIMEOUT;
    }

    // 解析命令行输出结果
    SolveResult parse_output(const std::string &output)
    {
        SolveResult result;
        std::unordered_map<std::string, std::string> tokens;
        std::istringstream ss(output);
        std::string line;
        while (std::getline(ss, line))
        {
            // 解析命令行输出结果中的每一行
            std::string key, value;
            auto pos = line.find(": ");
            if (pos != std::string::npos)
            {
                key = line.substr(0, pos);
                value = line.substr(pos + 2);
            }
            else
            {
                key = line;
                value = "";
            }
            key.erase(std::remove_if(key.begin(), key.end(), [](unsigned char c)
                                     { return !std::isalnum(c); }),
                      key.end());
            tokens[key] = value;
        }

        // 提取解析出的参数
        auto iter = tokens.find("FieldcenterRAHMSDecDMS");
        if (iter != tokens.end())
        {
            auto pos = iter->second.find(",");
            if (pos != std::string::npos)
            {
                result.ra = iter->second.substr(0, pos);
                result.dec = iter->second.substr(pos + 1);
            }
        }
        iter = tokens.find("Fieldsize");
        if (iter != tokens.end())
        {
            auto pos = iter->second.find("x");
            if (pos != std::string::npos)
            {
                result.fov_x = std::stod(iter->second.substr(0, pos));
                result.fov_y = std::stod(iter->second.substr(pos + 1));
            }
        }
        iter = tokens.find("Fieldrotationangleupisdegrees");
        if (iter != tokens.end())
        {
            result.rotation = std::stod(iter->second);
        }

        return result;
    }

    // 给定输入参数，生成命令行指令
    std::string make_command(const std::string &image, const std::string &ra, const std::string &dec, const double &radius, const int &downsample,
                             const std::vector<int> &depth, const double &scale_low, const double &scale_high, const int &width, const int &height,
                             const std::string &scale_units, const bool &overwrite, const bool &no_plot, const bool &verify,
                             const bool &debug, const bool &resort, const bool &_continue, const bool &no_tweak)
    {
        // 参数校验
        if (image.empty())
        {
            throw AstrometryException("Image file is empty");
        }

        // 生成命令行指令
        std::stringstream ss;
        ss << kSolveFieldPath << " \"" << image << "\"";
        if (!ra.empty())
        {
            ss << " --ra \"" << ra << "\"";
        }
        if (!dec.empty())
        {
            ss << " --dec \"" << dec << "\"";
        }
        if (radius > 0)
        {
            ss << " --radius " << radius;
        }
        if (downsample != 1)
        {
            ss << " --downsample " << downsample;
        }
        if (!depth.empty())
        {
            ss << " --depth " << depth[0] << "," << depth[1];
        }
        if (scale_low > 0)
        {
            ss << " --scale-low " << scale_low;
        }
        if (scale_high > 0)
        {
            ss << " --scale-high " << scale_high;
        }
        if (width > 0)
        {
            ss << " --width " << width;
        }
        if (height > 0)
        {
            ss << " --height " << height;
        }
        if (!scale_units.empty())
        {
            ss << " --scale-units \"" << scale_units << "\"";
        }
        if (overwrite)
        {
            ss << " --overwrite";
        }
        if (no_plot)
        {
            ss << " --no-plot";
        }
        if (verify)
        {
            ss << " --verify";
        }
        if (debug)
        {
            ss << " --debug";
        }
        if (resort)
        {
            ss << " --resort";
        }
        if (_continue)
        {
            ss << " --continue";
        }
        if (no_tweak)
        {
            ss << " --no-tweak";
        }

        return ss.str();
    }

    json solve(const std::string &image, const std::string &ra, const std::string &dec, const double &radius, const int &downsample,
               const std::vector<int> &depth, const double &scale_low, const double &scale_high, const int &width, const int &height,
               const std::string &scale_units, const bool &overwrite, const bool &no_plot, const bool &verify,
               const bool &debug, const int &timeout, const bool &resort, const bool &_continue, const bool &no_tweak)
    {
        SolveResult result;
        json ret_json;

        try
        {
            auto command = make_command(image, ra, dec, radius, downsample, depth, scale_low, scale_high,
                                        width, height, scale_units, overwrite, no_plot, verify, debug, resort, _continue, no_tweak);
            std::string output;
            auto status = execute_command(command, timeout, output);
            switch (status)
            {
            case CommandStatus::SUCCESS:
                result = parse_output(output);
                break;
            case CommandStatus::FAILED:
                throw AstrometryException("Command execution failed");
            case CommandStatus::TIMEOUT:
                throw AstrometryException("Command execution timed out");
            }
        }
        catch (const std::exception &e)
        {
            ret_json["error_message"] = e.what();
            return ret_json;
        }

        // 将解析结果写入JSON对象
        if (!result.ra.empty())
        {
            ret_json["ra"] = result.ra;
        }
        if (!result.dec.empty())
        {
            ret_json["dec"] = result.dec;
        }
        if (result.fov_x > 0)
        {
            ret_json["fov_x"] = result.fov_x;
        }
        if (result.fov_y > 0)
        {
            ret_json["fov_y"] = result.fov_y;
        }
        if (result.rotation != 0)
        {
            ret_json["rotation"] = result.rotation;
        }
        if (ret_json.empty())
        {
            ret_json["error_message"] = "Solve failed";
        }

        return ret_json;
    }

} // namespace Lithium::API:Astrometry
