/*
 * ongc.hpp
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

Date: 2023-7-13

Description: C++ Version of PyOngc

**************************************************/

#pragma once

#include <vector>
#include <string>
#include <tuple>

/**
 * @class Dso
 * @brief 表示一个天体对象 (Distributed Star Object)
 */
class Dso
{
public:
    /**
     * @brief 构造函数
     * @param name 天体对象的名称
     * @param returndup 是否返回复制对象
     */
    Dso(const std::string &name, bool returndup = false);

    /**
     * @brief 将天体对象转换为字符串表示形式
     * @return 天体对象的字符串表示形式
     */
    std::string to_string() const;

    /**
     * @brief 获取天体对象所属的星座
     * @return 星座的名称
     */
    std::string get_constellation() const;

    /**
     * @brief 获取天体对象的坐标
     * @return 包含赤经和赤纬的坐标数组
     */
    std::vector<double> get_coords() const;

    /**
     * @brief 获取天体对象的赤纬
     * @return 赤纬的字符串表示形式
     */
    std::string get_dec() const;

    /**
     * @brief 获取天体对象的尺寸
     * @return 包含主轴长度、次轴长度和位置角的尺寸数组
     */
    std::vector<double> get_dimensions() const;

    /**
     * @brief 获取天体对象的哈勃分类
     * @return 哈勃分类的字符串表示形式
     */
    std::string get_hubble() const;

    /**
     * @brief 获取天体对象的ID
     * @return 天体对象的ID
     */
    int get_id() const;

    /**
     * @brief 获取天体对象的所有标识符
     * @return 返回一个包含多个标识符的元组，依次是目录、Messier编号、NGC编号、IC编号和常用名称
     */
    std::tuple<std::string, std::vector<std::string>, std::vector<std::string>, std::vector<std::string>, std::vector<std::string>> get_identifiers() const;

private:
    /**
     * @brief 识别天体对象的名称
     * @param name 天体对象的名称
     * @param catalog 识别出的目录名称
     * @param objectname 识别出的目录中的对象名称
     */
    void recognize_name(const std::string &name, std::string &catalog, std::string &objectname) const;

    /**
     * @brief 查询数据库并返回单行结果
     * @param cols 查询的列名
     * @param tables 查询的表名
     * @param params 查询参数
     * @return 返回查询结果的字符串数组
     */
    std::vector<std::string> _queryFetchOne(const std::string &cols, const std::string &tables, const std::string &params);

    /**
     * @brief 将字符串按指定分隔符拆分为字符串数组
     * @param input 输入字符串
     * @param delimiter 分隔符
     * @return 拆分后的字符串数组
     */
    std::vector<std::string> split(const std::string &input, char delimiter) const;

    /**
     * @brief 将字符串数组按指定分隔符连接成一个字符串
     * @param strings 字符串数组
     * @param delimiter 分隔符
     * @return 连接后的字符串
     */
    std::string join(const std::vector<std::string> &strings, const std::string &delimiter) const;

    int _id;                    /**< 天体对象的ID */
    std::string _name;          /**< 天体对象的名称 */
    std::string _type;          /**< 天体对象的类型 */
    double _ra;                 /**< 天体对象的赤经 */
    double _dec;                /**< 天体对象的赤纬 */
    std::string _const;         /**< 天体对象所属的星座 */
    std::string _notngc;        /**< 是否不属于NGC目录 */
    double _majax;              /**< 天体对象的主轴长度 */
    double _minax;              /**< 天体对象的次轴长度 */
    int _pa;                    /**< 天体对象的位置角 */
    double _bmag;               /**< 天体对象的B星等 */
    double _vmag;               /**< 天体对象的V星等 */
    double _jmag;               /**< 天体对象的J星等 */
    double _hmag;               /**< 天体对象的H星等 */
    double _kmag;               /**< 天体对象的K星等 */
    double _sbrightn;           /**< 天体对象的表面亮度 */
    std::string _hubble;        /**< 天体对象的哈勃分类 */
    double _parallax;           /**< 天体对象的视差 */
    double _pmra;               /**< 天体对象的赤经年周运动 */
    double _pmdec;              /**< 天体对象的赤纬年周运动 */
    double _radvel;             /**< 天体对象的径向速度 */
    double _redshift;           /**< 天体对象的红移 */
    double _cstarumag;          /**< 中心恒星的U星等 */
    double _cstarbmag;          /**< 中心恒星的B星等 */
    double _cstarvmag;          /**< 中心恒星的V星等 */
    std::string _messier;       /**< 天体对象的Messier编号 */
    std::string _ngc;           /**< 天体对象的NGC编号 */
    std::string _ic;            /**< 天体对象的IC编号 */
    std::string _cstarnames;    /**< 中心恒星的名称 */
    std::string _identifiers;   /**< 天体对象的标识符 */
    std::string _commonnames;   /**< 天体对象的常用名称 */
    std::string _nednotes;      /**< 天体对象的NED注释 */
    std::string _ongcnotes;     /**< 天体对象的OpenNGC注释 */
};
