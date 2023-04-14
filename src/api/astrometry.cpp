/*
 * astrometry.cpp
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

Date: 2023-4-9

Description: Astrometry Command Line

**************************************************/

#include "astrometry.hpp"

namespace OpenAPT::API::Astrometry
{
    /**
     * @brief 利用 Astrometry.net 的 solve-field 工具对图像进行求解
     * 
     * @param image 图像文件路径
     * @param ra 目标区域的赤经信息，格式为 HH:MM:SS（时：分：秒）
     * @param dec 目标区域的赤纬信息，格式为 DD:MM:SS（度：分：秒）
     * @param radius 搜索半径
     * @param downsample 下采样倍率
     * @param depth 图像搜索深度，由两个整数构成一个 vector
     * @param scale_low 亮度值下限
     * @param scale_high 亮度值上限
     * @param width 图像宽度
     * @param height 图像高度
     * @param scale_units 图像尺寸单位，可选值有：degwidth（度）、arcminwidth（弧分）和 arcsecwidth（弧秒）
     * @param overwrite 是否覆盖已有文件
     * @param no_plot 是否生成星表图像
     * @param verify 是否验证解决方案
     * @param debug 是否开启调试模式
     * @param timeout 解析超时时间（秒）
     * @param resort 是否按照匹配等级重新排序
     * @param _continue 是否从上次停止的地方继续
     * @param no_tweak 是否关闭微调选项
     * @return json类型的结果，包含解决方案的相关信息如赤经、赤纬、视场大小等，若解决方案失败则返回对应错误信息
     */
    json solve(const std::string& image, const std::string& ra, const std::string& dec, const double& radius, const int& downsample,
           const std::vector<int>& depth, const double& scale_low, const double& scale_high, const int& width, const int& height,
           const std::string& scale_units, const bool& overwrite, const bool& no_plot, const bool& verify,
           const bool& debug, const int& timeout, const bool& resort, const bool& _continue, const bool& no_tweak)
    {
        // 初始化返回值
        json ret_json;
        ret_json["message"] = "unknown error";

        try {
            // 参数校验
            assert(!image.empty() && "wrong image file type");

            // 生成命令行指令
            std::string command = "solve-field " + image;

            if (!ra.empty())
                command += " --ra " + ra;
            if (!dec.empty())
                command += " --dec " + dec;
            if (radius > 0)
                command += " --radius " + std::to_string(radius);
            if (downsample != 1)
                command += " --downsample " + std::to_string(downsample);
            if (!depth.empty())
                command += " --depth " + std::to_string(depth[0]) + "," + std::to_string(depth[1]);
            if (scale_low > 0)
                command += " --scale-low " + std::to_string(scale_low);
            if (scale_high > 0)
                command += " --scale-high " + std::to_string(scale_high);
            if (width > 0)
                command += " --width " + std::to_string(width);
            if (height > 0)
                command += " --height " + std::to_string(height);
            if (!scale_units.empty())
                command += " --scale-units " + scale_units;
            if (overwrite)
                command += " --overwrite";
            if (no_plot)
                command += " --no-plot";
            if (verify)
                command += " --verify";
            if (resort)
                command += " --resort";
            if (_continue)
                command += " --continue";
            if (no_tweak)
                command += " --no-tweak";

            // 执行命令行指令
            FILE* pipe = popen(command.c_str(), "r");
            if (!pipe) {
                ret_json["message"] = "failed to open pipe";
                return ret_json;
            }

            char buffer[256];
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                std::string item(buffer);

                // 解析输出结果
                size_t pos;
                if ((pos = item.find("Field center: (RA H:M:S, Dec D:M:S) = ")) != std::string::npos) {
                    std::string ra_dec = item.substr(pos + 41, 19);
                    pos = ra_dec.find(",");
                    if (pos == std::string::npos) {
                        continue;
                    }
                    ret_json["ra"] = ra_dec.substr(0, pos);
                    ret_json["dec"] = ra_dec.substr(pos + 2);
                }
                else if ((pos = item.find("Field size: ")) != std::string::npos) {
                    std::string fov = item.substr(pos + 12);
                    pos = fov.find("x");
                    if (pos == std::string::npos) {
                        continue;
                    }
                    ret_json["fov_x"] = fov.substr(0, pos);
                    ret_json["fov_y"] = fov.substr(pos + 1);
                }
                else if ((pos = item.find("Field rotation angle: up is ")) != std::string::npos) {
                    auto end_pos = item.rfind(" degrees");
                    if (end_pos == std::string::npos) {
                        continue;
                    }
                    ret_json["rotation"] = item.substr(pos + 29, end_pos - pos - 29);
                }
            }

            pclose(pipe);

            // 判断解析结果是否可用
            if (ret_json.find("ra") == ret_json.end() || ret_json.find("dec") == ret_json.end()) {
                ret_json["message"] = "Solve failed";
            } else {
                ret_json.erase("message");
            }

        } catch (const std::exception& e) {
            ret_json["message"] = e.what();
        } catch (...) {
            ret_json["message"] = "unpredictable error";
        }

        return ret_json;
    }

} // namespace OpenAPT::API:Astrometry


