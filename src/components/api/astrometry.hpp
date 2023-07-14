
#pragma once

#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace Lithium::API::Astrometry
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
    json solve(const std::string &image, const std::string &ra = "", const std::string &dec = "", const double &radius = -1.0, const int &downsample = 1,
               const std::vector<int> &depth = {}, const double &scale_low = -1.0, const double &scale_high = -1.0, const int &width = -1, const int &height = -1,
               const std::string &scale_units = "degwidth", const bool &overwrite = true, const bool &no_plot = true, const bool &verify = false,
               const bool &debug = false, const int &timeout = 30, const bool &resort = false, const bool &_continue = false, const bool &no_tweak = false);
} // namespace Lithium::API:Astrometry
