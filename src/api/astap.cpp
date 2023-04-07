/*
 * astap.cpp
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

Date: 2023-4-4

Description: Astap Solver Interface

**************************************************/

#include <iostream>
#include <array>
#include <string>
#include <future>
#include <chrono>
#include <stdexcept>
#include <spdlog/spdlog.h>
#include <fitsio.h>
#include <iostream>
#include <cstring>
#include <filesystem>
#include <map>

#include "astap.hpp"

#if defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace std;

namespace fs = std::filesystem;

namespace OpenAPT::API::ASTAP
{
    /**
     * @brief 检查指定的可执行文件是否存在且可以执行。
     *
     * @param file_name 文件名（不包含扩展名）。
     * @param file_ext 扩展名（包括点号）。
     * @return true 如果文件存在且可以执行。
     * @return false 如果文件不存在或不可执行。
     */
    bool check_executable_file(const string &file_name, const string &file_ext)
    {
// 构建文件路径
#if defined(_WIN32)
        fs::path file_path = file_name + file_ext;
#else
        fs::path file_path = file_name;
#endif

        // 输出调试信息
        spdlog::debug("Checking file '{}'.", file_path.string());

        // 检查文件是否存在
        if (!fs::exists(file_path))
        {
            spdlog::warn("The file '{}' does not exist.", file_path.string());
            return false;
        }

// 检查文件权限（仅限 Linux）
#if !defined(_WIN32)
        if (!fs::is_regular_file(file_path) || access(file_path.c_str(), X_OK) != 0)
        {
            spdlog::warn("The file '{}' is not a regular file or is not executable.", file_path.string());
            return false;
        }
#endif

// 检查文件是否为可执行文件（仅限 Windows）
#if defined(_WIN32)
        if (!fs::is_regular_file(file_path) || !(GetFileAttributesA(file_path.c_str()) & FILE_ATTRIBUTE_DIRECTORY))
        {
            spdlog::warn("The file '{}' is not a regular file or is not executable.", file_path.string());
            return false;
        }
#endif

        spdlog::info("The file '{}' exists and is executable.", file_path.string());
        return true;
    }

    bool IsSubstring(const std::string &str, const std::string &sub)
    {
        return str.find(sub) != std::string::npos;
    }

    /**
     * @brief 使用 ASTAP 命令进行图像匹配，并返回匹配结果的输出。如果执行过程中出现错误，返回一个空字符串。
     *
     * @param command ASTAP 命令的字符串。该参数必须提供。
     * @param ra 图像所在区域的赤经（度）。如果不需要指定
     * @param dec 图像所在区域的赤纬（度）。如果不需要指定
     * @param fov 图像视场（度）。如果不需要指定
     * @param timeout 执行命令的超时时间（秒数）。默认值为30秒。
     * @param update 是否在执行匹配前更新星表。默认为true。
     * @param image 需要匹配的图像文件路径。如果不需要指定，请传入一个空字符串。
     * @return 如果执行成功，返回匹配结果的输出；否则返回一个空字符串。
     */
    string execute_astap_command(string command, double ra = 0.0, double dec = 0.0, double fov = 0.0, int timeout = 30, bool update = true, string image = "")
    {
        // 输出调试信息
        spdlog::debug("Executing command '{}' with RA={}, DEC={}, FOV={}, timeout={}s, image='{}'.", command, ra, dec, fov, timeout, image);

        // 构造命令行字符串
        if (ra != 0.0)
        {
            command += " -ra " + to_string(ra);
        }
        if (dec != 0.0)
        {
            command += " -spd " + to_string(dec + 90);
        }
        if (fov != 0.0)
        {
            command += " -fov " + to_string(fov);
        }
        if (!image.empty())
        {
            command += " -f " + image;
        }
        if (update)
        {
            command += " -update ";
        }
        // 执行命令并等待结果
        array<char, 4096> buffer{};
        future<string> result = async(launch::async, [&]() -> string
                                      {
            // 创建子进程
            FILE *pipe = popen(command.c_str(), "r");
            if (!pipe) {
                throw runtime_error("Error: failed to run command '" + command + "'.");
            }
            // 读取子进程的输出
            string output = "";
            while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
                output += buffer.data();
            }
            // 关闭子进程并返回输出结果
            pclose(pipe);
            return output; });

        // 等待命令执行完成，或者超时
        auto start_time = chrono::system_clock::now();
        while (result.wait_for(chrono::seconds(1)) != future_status::ready)
        {
            auto elapsed_time = chrono::duration_cast<chrono::seconds>(chrono::system_clock::now() - start_time).count();
            if (elapsed_time > timeout)
            {
                spdlog::error("Error: command '{}' timed out after {} seconds.", command, timeout);
                return "";
            }
        }
        // 返回命令执行结果，并输出调试信息
        auto output = result.get();
        spdlog::debug("Command '{}' returned: {}", command, output);
        return output;
    }

    /**
     * @brief Solve FITS header and return solved RA/DEC, rotation, and focal length.
     *
     * @param image The name of the input FITS image file.
     *
     * @return A map structure containing the following keys:
     * - message: A string describing the solving result.
     * - ra: The right ascension in degrees (string).
     * - dec: The declination in degrees (string).
     * - rotation: The rotation angle (string).
     * - focal_length: The average focal length in millimeters (string).
     */
    map<string, string> read_astap_result(const string &image)
    {
        map<string, string> ret_struct;

        // 打开 FITS 文件并读取头信息
        fitsfile *fptr;
        int status = 0;
        fits_open_file(&fptr, image.c_str(), READONLY, &status);
        if (status != 0)
        {
            ret_struct["message"] = "Error: cannot open FITS file '" + image + "'.";
            spdlog::error(ret_struct["message"]);
            return ret_struct;
        }

        double solved_ra, solved_dec, x_pixel_arcsec, y_pixel_arcsec, rotation, x_pixel_size, y_pixel_size;
        bool data_get_flag = false;
        char comment[FLEN_COMMENT];

        // 读取头信息中的关键字
        fits_read_key(fptr, TDOUBLE, "CRVAL1", &solved_ra, comment, &status);
        fits_read_key(fptr, TDOUBLE, "CRVAL2", &solved_dec, comment, &status);
        fits_read_key(fptr, TDOUBLE, "CDELT1", &x_pixel_arcsec, comment, &status);
        fits_read_key(fptr, TDOUBLE, "CDELT2", &y_pixel_arcsec, comment, &status);
        fits_read_key(fptr, TDOUBLE, "CROTA1", &rotation, comment, &status);
        fits_read_key(fptr, TDOUBLE, "XPIXSZ", &x_pixel_size, comment, &status);
        fits_read_key(fptr, TDOUBLE, "YPIXSZ", &y_pixel_size, comment, &status);

        // 检查是否成功读取关键字
        if (status == 0)
        {
            data_get_flag = true;
        }
        else
        {
            spdlog::warn("Warning: cannot read some FITS header keywords.");
            status = 0;
        }

        // 关闭 FITS 文件
        fits_close_file(fptr, &status);

        // 构造返回结果
        if (data_get_flag)
        {
            ret_struct["message"] = "Solve success";
            ret_struct["ra"] = to_string(solved_ra);
            ret_struct["dec"] = to_string(solved_dec);
            ret_struct["rotation"] = to_string(rotation);
            if (x_pixel_size > 0.0 && y_pixel_size > 0.0)
            {
                double x_focal_length = x_pixel_size / x_pixel_arcsec * 206.625;
                double y_focal_length = y_pixel_size / y_pixel_arcsec * 206.625;
                double avg_focal_length = (x_focal_length + y_focal_length) / 2.0;
                ret_struct["focal_length"] = to_string(avg_focal_length);
                // 调试输出
                spdlog::debug("avg_focal_length: {}", avg_focal_length);
            }
        }
        else
        {
            ret_struct["message"] = "Solve failed";
        }

        // 最终输出
        spdlog::info("Function solve_fits_header result: {}", ret_struct["message"]);

        return ret_struct;
    }

    /**
     * @brief 使用 ASTAP 进行图像匹配，并返回匹配结果。如果匹配成功，返回包含星表数据的映射；否则返回一个错误消息。
     *
     * @param ra 图像所在区域的赤经（度）。如果不需要指定
     * @param dec 图像所在区域的赤纬（度）。如果不需要指定
     * @param fov 图像视场（度）。如果不需要指定
     * @param timeout 执行命令的超时时间（秒数）。默认值为30秒。
     * @param update 是否在执行匹配后更新图片。默认为true。
     * @param image 需要匹配的图像文件路径。该参数必须提供。
     * @return 如果匹配成功，返回包含星表数据的映射；否则返回一个错误消息。
     * */
    map<string, string> run_astap(double ra, double dec, double fov, int timeout, bool update, string image)
    {
        map<string, string> ret_struct;
        if (!check_executable_file("/usr/bin/astap", "") && !check_executable_file("/usr/local/bin/astap", ""))
        {
            spdlog::debug("No Astap solver engine found , please install before trying to solve an image");
            ret_struct["message"] = "No solver found!";
            return ret_struct;
        }

        string result = execute_astap_command("astap", ra, dec, fov, timeout, update, image);
        if (IsSubstring(result, "Solution found:"))
        {
            spdlog::info("Solved successfully");
            ret_struct = read_astap_result(image);
        }
        else
        {
            spdlog::error("Failed to solve the image");
            ret_struct["message"] = "Failed to solve the image";
        }
        return ret_struct;
    }
}
