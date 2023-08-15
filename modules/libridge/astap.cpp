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
#include <cstring>
#include <filesystem>
#include <map>
#include <fstream>

#include "astap.hpp"

#if defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <loguru/loguru.hpp>
#include <fitsio.h>

using namespace std;

namespace fs = std::filesystem;

namespace Lithium::API::ASTAP
{
    /**
     * @brief 检查指定的可执行文件是否存在且可以执行。
     *
     * @param file_name 文件名（不包含扩展名）。
     * @param file_ext 扩展名（包括点号）。
     * @return true 如果文件存在且可以执行。
     * @return false 如果文件不存在或不可执行。
     */
    bool check_executable_file(const std::string &file_name, const std::string &file_ext)
    {
#if defined(_WIN32)
        fs::path file_path = file_name + file_ext;
#else
        fs::path file_path = file_name;
#endif

        LOG_F(INFO, "Checking file '%s'.", file_path.string().c_str());

        if (!fs::exists(file_path))
        {
            LOG_F(WARNING, "The file '%s' does not exist.", file_path.string().c_str());
            return false;
        }

#if defined(_WIN32)
        if (!fs::is_regular_file(file_path) || !(GetFileAttributesA(file_path.generic_string().c_str()) & FILE_ATTRIBUTE_DIRECTORY))
        {
            LOG_F(WARNING, "The file '%s' is not a regular file or is not executable.", file_path.string().c_str());
            return false;
        }
#else
        if (!fs::is_regular_file(file_path) || access(file_path.c_str(), X_OK) != 0)
        {
            LOG_F(WARNING, "The file '%s' is not a regular file or is not executable.", file_path.string().c_str());
            return false;
        }
#endif

        LOG_F(INFO, "The file '%s' exists and is executable.", file_path.string().c_str());
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
    std::string execute_command(const std::string &command)
    {
        std::array<char, 4096> buffer{};
        FILE *pipe = popen(command.c_str(), "r");
        if (!pipe)
        {
            LOG_F(ERROR, "Error: failed to run command '%s'.", command.c_str());
            return "";
        }
        std::string output = "";
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
        {
            output += buffer.data();
        }
        pclose(pipe);
        return output;
    }

    template <typename Func, typename... Args>
    std::future<typename std::result_of<Func(Args...)>::type> async_retry(Func &&func, int attempts_left, std::chrono::seconds delay, Args &&...args)
    {
        typedef typename std::result_of<Func(Args...)>::type result_type;

        if (attempts_left < 1)
        {
            LOG_F(ERROR, "Exceeded maximum attempts");
            throw std::runtime_error("Exceeded maximum attempts");
        }

        try
        {
            return std::async(std::launch::async, std::forward<Func>(func), std::forward<Args>(args)...);
        }
        catch (...)
        {
            if (attempts_left == 1)
            {
                LOG_F(ERROR, "Failed to execute function after multiple attempts");
                throw;
            }
            else
            {
                --attempts_left;
                std::this_thread::sleep_for(delay);
                return async_retry(std::forward<Func>(func), attempts_left, delay, std::forward<Args>(args)...);
            }
        }
    }

    std::string execute_astap_command(const std::string &command, const double &ra = 0.0, const double &dec = 0.0, const double &fov = 0.0,
                                      const int &timeout = 30, const bool &update = true, const std::string &image = "")
    {
        // 输入参数合法性检查
        if (ra < 0.0 || ra > 360.0)
        {
            LOG_F(ERROR, "RA should be within [0, 360]");
            return "";
        }
        if (dec < -90.0 || dec > 90.0)
        {
            LOG_F(ERROR, "DEC should be within [-90, 90]");
            return "";
        }
        if (fov <= 0.0 || fov > 180.0)
        {
            LOG_F(ERROR, "FOV should be within (0, 180]");
            return "";
        }
        if (!image.empty())
        {
            std::fstream file;
            file.open(image, std::ios::in | std::ios::out);
            if (!file.good())
            {
                LOG_F(ERROR, "Error: image file '%s' is not accessible.", image.c_str());
                return "";
            }
            file.close();
        }

        // 构造命令行字符串
        std::string cmd = command;
        if (ra != 0.0)
        {
            cmd += " -ra " + std::to_string(ra);
        }
        if (dec != 0.0)
        {
            cmd += " -spd " + std::to_string(dec + 90);
        }
        if (fov != 0.0)
        {
            cmd += " -fov " + std::to_string(fov);
        }
        if (!image.empty())
        {
            cmd += " -f " + image;
        }
        if (update)
        {
            cmd += " -update ";
        }

        // 执行命令行指令
        auto result = async_retry([](const std::string &cmd)
                                  { return execute_command(cmd); },
                                  3, std::chrono::seconds(5), cmd);

        // 等待命令执行完成，或者超时
        auto start_time = std::chrono::system_clock::now();
        while (result.wait_for(std::chrono::seconds(1)) != std::future_status::ready)
        {
            auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start_time).count();
            if (elapsed_time > timeout)
            {
                LOG_F(ERROR, "Error: command timed out after %s seconds.", std::to_string(timeout).c_str());
                return "";
            }
        }

        // 返回命令执行结果，并输出调试信息
        auto output = result.get();
        LOG_F(INFO, "Command '%s' returned: %s", cmd.c_str(), output.c_str());
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
    std::map<std::string, std::string> read_astap_result(const std::string &image)
    {
        std::map<std::string, std::string> ret_struct;

        // 打开 FITS 文件并读取头信息
        fitsfile *fptr;
        int status = 0;
        fits_open_file(&fptr, image.c_str(), READONLY, &status);
        if (status != 0)
        {
            ret_struct["message"] = "Error: cannot open FITS file '" + image + "'.";
            LOG_F(ERROR, "{}", ret_struct["message"].c_str());
            return ret_struct;
        }

        double solved_ra, solved_dec, x_pixel_arcsec, y_pixel_arcsec, rotation, x_pixel_size, y_pixel_size;
        bool data_get_flag = false;
        char comment[FLEN_COMMENT];

        // 读取头信息中的关键字
        status = 0;
        fits_read_key(fptr, TDOUBLE, "CRVAL1", &solved_ra, comment, &status);

        status = 0;
        fits_read_key(fptr, TDOUBLE, "CRVAL2", &solved_dec, comment, &status);

        status = 0;
        fits_read_key(fptr, TDOUBLE, "CDELT1", &x_pixel_arcsec, comment, &status);

        status = 0;
        fits_read_key(fptr, TDOUBLE, "CDELT2", &y_pixel_arcsec, comment, &status);

        status = 0;
        fits_read_key(fptr, TDOUBLE, "CROTA1", &rotation, comment, &status);

        status = 0;
        fits_read_key(fptr, TDOUBLE, "XPIXSZ", &x_pixel_size, comment, &status);

        status = 0;
        fits_read_key(fptr, TDOUBLE, "YPIXSZ", &y_pixel_size, comment, &status);

        // 关闭 FITS 文件
        fits_close_file(fptr, &status);
        if (status != 0)
        {
            ret_struct["message"] = "Error: failed to close FITS file '" + image + "'.";
            LOG_F(ERROR, "{}", ret_struct["message"].c_str());
            return ret_struct;
        }

        // 构造返回结果
        if (data_get_flag)
        {
            ret_struct["message"] = "Solve success";
            ret_struct["ra"] = std::to_string(solved_ra);
            ret_struct["dec"] = std::to_string(solved_dec);
            ret_struct["rotation"] = std::to_string(rotation);
            if (x_pixel_size > 0.0 && y_pixel_size > 0.0)
            {
                double x_focal_length = x_pixel_size / x_pixel_arcsec * 206.625;
                double y_focal_length = y_pixel_size / y_pixel_arcsec * 206.625;
                double avg_focal_length = (x_focal_length + y_focal_length) / 2.0;
                ret_struct["focal_length"] = std::to_string(avg_focal_length);
                // 调试输出
                LOG_F(INFO, "avg_focal_length: {}", avg_focal_length);
            }
        }
        else
        {
            ret_struct["message"] = "Solve failed";
        }

        // 最终输出
        LOG_F(INFO, "Function solve_fits_header result: {}", ret_struct["message"].c_str());

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
    std::map<std::string, std::string> run_astap(double ra, double dec, double fov, int timeout, bool update, std::string image)
    {
        std::map<std::string, std::string> ret_struct;

        if (!check_executable_file("/usr/bin/astap", "") && !check_executable_file("/usr/local/bin/astap", ""))
        {
            LOG_F(INFO, "No Astap solver engine found, please install before trying to solve an image");
            ret_struct["message"] = "No solver found!";
            return ret_struct;
        }

        std::string result = execute_astap_command("astap", ra, dec, fov, timeout, update, image);
        if (result.find("Solution found:") != std::string::npos)
        {
            LOG_F(INFO, "Solved successfully");
            ret_struct = read_astap_result(image);
        }
        else
        {
            LOG_F(ERROR, "Failed to solve the image");
            ret_struct["message"] = "Failed to solve the image";
        }

        return ret_struct;
    }

}
