/*
 * basic_device.hpp
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

Date: 2023-3-29

Description: Basic Device Definitions

**************************************************/

#pragma once

#include <string>

#include "task/camera_task.hpp"

namespace OpenAPT
{

    enum class DeviceType
    {
        Camera,
        Telescope,
        Focuser,
        FilterWheel,
        Solver,
        Guider,
        NumDeviceTypes
    };

    static constexpr int DeviceTypeCount = 6;

    enum class DeviceStatus
    {
        Unconnected,
        Connected,
        Disconnected
    };

    struct CameraFrame
    {
        int X = 0;
        int Y = 0;
        double PixelX = 0;
        double PixelY = 0;
    };

    /**
     * @brief 设备基类
     */
    class Device
    {
    public:
        /**
         * @brief 构造函数
         *
         * @param name 设备名称
         */
        explicit Device(const std::string &name);

        /**
         * @brief 析构函数
         */
        virtual ~Device();

        /**
         * @brief 连接设备
         *
         * @param name 设备名称
         * @return 连接设备是否成功
         */
        virtual bool connect(std::string name) {}

        /**
         * @brief 断开设备连接
         *
         * @return 断开设备连接是否成功
         */
        virtual bool disconnect() {}

        /**
         * @brief 重新连接设备
         *
         * @return 重新连接设备是否成功
         */
        virtual bool reconnect() {}

        /**
         * @brief 扫描可用设备
         *
         * @return 扫描可用设备是否成功
         */
        virtual bool scanForAvailableDevices() {}

        /**
         * @brief 获取设备设置
         *
         * @return 获取设备设置是否成功
         */
        virtual bool getSettings() {}

        /**
         * @brief 保存设备设置
         *
         * @return 保存设备设置是否成功
         */
        virtual bool saveSettings() {}

        /**
         * @brief 获取设备参数
         *
         * @param paramName 参数名称
         * @return 获取设备参数是否成功
         */
        virtual bool getParameter(const std::string &paramName) {}

        /**
         * @brief 设置设备参数
         *
         * @param paramName 参数名称
         * @param paramValue 参数值
         * @return 设置设备参数是否成功
         */
        virtual bool setParameter(const std::string &paramName, const std::string &paramValue) {}

        /**
         * @brief 获取设备名称
         *
         * @return 设备名称
         */
        virtual std::string getName() const
        {
            return _name;
        }

        /**
         * @brief 设置设备名称
         *
         * @param name 设备名称
         * @return 设置设备名称是否成功
         */
        virtual bool setName(const std::string &name)
        {
            _name = name;
        }

        std::string getId() const
        {
            return device_name;
        }

        /**
         * @brief 设置设备ID
         *
         * @param id 设备ID
         * @return 设置设备ID是否成功
         */
        virtual bool setId(int id)
        {
            id = id;
        }

        /**
         * @brief 获取SimpleTask
         *
         * @param task_name 任务名
         * @param params 参数
         * @return SimpleTask指针
         */
        virtual std::shared_ptr<OpenAPT::SimpleTask> getSimpleTask(const std::string &task_name, const nlohmann::json &params) {}

        /**
         * @brief 获取ConditionalTask
         *
         * @param task_name 任务名
         * @param params 参数
         * @return ConditionalTask指针
         */
        virtual std::shared_ptr<OpenAPT::ConditionalTask> getCondtionalTask(const std::string &task_name, const nlohmann::json &params) {}

        /**
         * @brief 获取LoopTask
         *
         * @param task_name 任务名
         * @param params 参数
         * @return LoopTask指针
         */
        virtual std::shared_ptr<OpenAPT::LoopTask> getLoopTask(const std::string &task_name, const nlohmann::json &params) {}

    public:
        std::string _name;                  ///< 设备名称
        int _id;                            ///< 设备ID
        std::string device_name;            ///< 设备名称
        std::string description;            ///< 设备描述信息
        std::string configPath;             ///< 配置文件路径
        std::string hostname = "127.0.0.1"; ///< 主机名
        int port = 7624;                    ///< 端口号
        bool is_connected;                  ///< 是否已连接
        bool is_debug;                      ///< 调试模式
    };

    /**
     * @brief 相机设备类，继承自Device类
     */
    class Camera : public Device
    {
    public:
        /**
         * @brief 构造函数
         *
         * @param name 设备名称
         */
        Camera(const std::string &name);

        /**
         * @brief 析构函数
         */
        ~Camera();

        /**
         * @brief 启动曝光
         *
         * @param duration_ms 曝光时间（毫秒）
         * @return 启动曝光是否成功
         */
        virtual bool startExposure(int duration_ms) {}

        /**
         * @brief 停止曝光
         *
         * @return 停止曝光是否成功
         */
        virtual bool stopExposure() {}

        /**
         * @brief 等待曝光完成
         *
         * @return 曝光完成是否成功
         */
        virtual bool waitForExposureComplete() {}

        // virtual bool readImage(Image& image);

        /**
         * @brief 获取当前曝光时间
         *
         * @return 当前曝光时间
         */
        double getExposureTime() const { return current_exposure_time; }

        /**
         * @brief 设置曝光时间
         *
         * @param time 曝光时间
         * @return 设置曝光时间是否成功
         */
        virtual bool setExposureTime(double time) {}

        /**
         * @brief 启动实时预览
         *
         * @return 启动实时预览是否成功
         */
        virtual bool startLiveView() {}

        /**
         * @brief 停止实时预览
         *
         * @return 停止实时预览是否成功
         */
        virtual bool stopLiveView() {}

        // virtual bool readLiveView(Image& image);

        /**
         * @brief 检查是否有视频输出
         *
         * @return 是否有视频输出
         */
        virtual bool isVideoAvailable() const { return is_video_available; }

        /**
         * @brief 检查是否可冷却
         *
         * @return 是否可冷却
         */
        bool isCoolingAvailable() { return can_cooling; }

        /**
         * @brief 检查是否正在冷却
         *
         * @return 是否正在冷却
         */
        bool isCoolingOn() { return is_cooling; }

        /**
         * @brief 控制冷却开关
         *
         * @param on 是否开启冷却
         * @return 控制冷却开关是否成功
         */
        virtual bool setCoolingOn(bool on) {}

        /**
         * @brief 设置目标温度
         *
         * @param temperature 目标温度
         * @return 设置目标温度是否成功
         */
        virtual bool setTemperature(double temperature) {}

        /**
         * @brief 获取当前温度
         *
         * @return 当前温度
         */
        virtual double getTemperature() {}

        /**
         * @brief 检查是否有快门
         *
         * @return 是否有快门
         */
        bool isShutterAvailable() { return has_shutter; }

        /**
         * @brief 检查快门状态
         *
         * @return 快门是否关闭
         */
        bool isShutterOpen() { return is_shutter_closed; }

        /**
         * @brief 控制快门开关
         *
         * @param open 是否打开快门
         * @return 控制快门开关是否成功
         */
        virtual bool setShutterOpen(bool open) {}

        /**
         * @brief 检查是否支持子帧
         *
         * @return 是否支持子帧
         */
        bool isSubframeEnabled() { return is_subframe; }

        /**
         * @brief 设置子帧是否启用
         *
         * @param enabled 是否启用子帧
         * @return 设置子帧是否成功
         */
        bool setSubframeEnabled(bool enabled) {}

        // virtual bool setSubframe(const ImageRect& rect);

        /**
         * @brief 检查是否支持某种采样方式
         *
         * @param binning 采样率
         * @return 是否支持该采样率
         */
        bool isBinningSupported(int binning) { return can_binning; }

        /**
         * @brief 获取最大采样率
         *
         * @return 最大采样率
         */
        int getMaxBinning() { return max_binning; }

        /**
         * @brief 获取横向采样率
         *
         * @return 横向采样率
         */
        int getBinningX() { return binning_x; }

        /**
         * @brief 设置采样率
         *
         * @param binning 采样率
         * @return 设置采样率是否成功
         */
        virtual bool setBinning(int binning) {}

        /**
         * @brief 检查是否支持某种增益值
         *
         * @param gain 增益值
         * @return 是否支持该增益值
         */
        bool isGainSupported(int gain) { return can_gain; }

        /**
         * @brief 获取最大增益值
         *
         * @return 最大增益值
         */
        int getMaxGain() { return max_gain; }

        /**
         * @brief 获取当前增益值
         *
         * @return 当前增益值
         */
        int getGain() { return gain; }

        /**
         * @brief 设置增益值
         *
         * @param gain 增益值
         * @return 设置增益值是否成功
         */
        virtual bool setGain(int gain) {}

        /**
         * @brief 检查是否支持某种偏置值
         *
         * @param offset 偏置值
         * @return 是否支持该偏置值
         */
        bool isOffsetSupported(int offset) { return can_offset; }

        /**
         * @brief 获取最大偏置值
         *
         * @return 最大偏置值
         */
        int getMaxOffset() { return max_offset; }

        /**
         * @brief 获取当前偏置值
         *
         * @return 当前偏置值
         */
        int getOffset() { return offset; }

        /**
         * @brief 设置偏置值
         *
         * @param offset 偏置值
         * @return 设置偏置值是否成功
         */
        virtual bool setOffset(int offset) {}

        // virtual bool getROIFrame() {}

        /**
         * @brief 设置ROI帧
         *
         * @param start_x X轴起始坐标
         * @param start_y Y轴起始坐标
         * @param frame_x 帧宽度
         * @param frame_y 帧高度
         * @return 设置ROI帧是否成功
         */
        virtual bool setROIFrame(int start_x, int start_y, int frame_x, int frame_y) {}

    public:
        static const double UnknownPixelSize;

        bool is_connected;  ///< 是否已连接
        bool is_exposuring; ///< 是否正在曝光
        bool is_video;      ///< 是否有视频输出
        bool is_color;      ///< 是否为彩色相机

        double current_exposure_time; ///< 当前曝光时间
        double max_exposure_time;     ///< 最大曝光时间
        double min_exposure_time;     ///< 最小曝光时间

        bool is_video_available; ///< 视频输出是否可用

        bool can_gain; ///< 是否可调增益
        int gain;      ///< 当前增益值
        int max_gain;  ///< 最大增益值

        bool can_offset; ///< 是否可调偏置值
        int offset;      ///< 当前偏置值
        int max_offset;  ///< 最大偏置值

        bool has_shutter;       ///< 是否有快门
        bool is_shutter_closed; ///< 快门是否关闭

        bool has_subframe; ///< 是否支持子帧
        bool is_subframe;  ///< 子帧是否启用

        bool can_binning; ///< 是否支持采样率控制
        int binning_x;    ///< 横向采样率
        int binning_y;    ///< 纵向采样率
        int max_binning;  ///< 最大采样率
        int min_binning;  ///< 最小采样率

        int read_delay; ///< 读取延迟

        bool can_cooling;           ///< 是否可冷却
        bool is_cooling;            ///< 是否正在冷却
        double current_temperature; ///< 当前温度
        double current_power;       ///< 当前功率

        double pixel;    ///< 像素大小
        double pixel_x;  ///< X轴像素大小
        double pixel_y;  ///< Y轴像素大小
        int pixel_depth; ///< 像素深度
        int frame_x;     ///< 帧宽度
        int frame_y;     ///< 帧高度
        int max_frame_x; ///< 最大帧宽度
        int max_frame_y; ///< 最大帧高度
        int start_x;     ///< X轴起始坐标
        int start_y;     ///< Y轴起始坐标
    };

    class Telescope : public Device
    {
    public:
        /**
         * @brief 构造函数
         * @param name 望远镜名称
         */
        Telescope(const std::string &name);

        /**
         * @brief 析构函数
         */
        ~Telescope();

        /**
         * @brief 指向新目标
         * @param ra 目标赤经
         * @param dec 目标赤纬
         * @param j2000 是否使用J2000坐标系，默认为false，表示使用本地坐标系
         * @return 是否成功指向新目标
         */
        virtual bool SlewTo(const std::string &ra, const std::string &dec, const bool j2000 = false) {}

        /**
         * @brief 中止望远镜的指向
         * @return 是否成功中止指向
         */
        virtual bool Abort() {}

        /**
         * @brief 获取望远镜是否在指向新目标
         * @return 返回 true 表示正在指向新目标，否则返回 false
         */
        bool isSlewing() { return is_slewing; }

        /**
         * @brief 获取当前赤经位置
         * @return 当前赤经位置
         */
        std::string getCurrentRA() { return current_ra; }

        /**
         * @brief 获取当前赤纬位置
         * @return 当前赤纬位置
         */
        std::string getCurrentDec() { return current_dec; }

        /**
         * @brief 开始跟踪运动目标
         * @param model 跟踪模式，包括恒星跟踪、太阳跟踪和月球跟踪
         * @param speed 跟踪速度，默认为1
         * @return 是否成功开始跟踪运动目标
         */
        virtual bool StartTracking(const std::string &model, const std::string &speed) {}

        /**
         * @brief 停止跟踪运动目标
         * @return 是否成功停止跟踪运动目标
         */
        virtual bool StopTracking() {}

        /**
         * @brief 设置跟踪模式
         * @param mode 跟踪模式，包括恒星跟踪、太阳跟踪和月球跟踪
         * @return 是否成功设置跟踪模式
         */
        virtual bool setTrackingMode(const std::string &mode) {}

        /**
         * @brief 设置跟踪速度
         * @param speed 跟踪速度
         * @return 是否成功设置跟踪速度
         */
        virtual bool setTrackingSpeed(const std::string &speed) {}

        /**
         * @brief 获取当前跟踪模式
         * @return 当前跟踪模式，包括恒星跟踪、太阳跟踪和月球跟踪
         */
        std::string getTrackingMode() { return current_tracking_mode; }

        /**
         * @brief 获取当前跟踪速度
         * @return 当前跟踪速度
         */
        std::string getTrackingSpeed() { return current_tracking_speed; }

        /**
         * @brief 将望远镜回到家位置
         * @return 是否成功将望远镜回到家位置
         */
        virtual bool Home() {}

        /**
         * @brief 判断望远镜是否在家位置
         * @return 返回 true 表示望远镜在家位置，否则返回 false
         */
        virtual bool isAtHome() {}

        /**
         * @brief 设置家位置
         * @return 是否成功设置家位置
         */
        virtual bool setHomePosition() {}

        /**
         * @brief 获取望远镜是否可以回到家位置
         * @return 返回 true 表示望远镜可以回到家位置，否则返回 false
         */
        bool isHomeAvailable() { return can_home; }

        /**
         * @brief 停车
         * @return 是否成功停车
         */
        virtual bool Park() {}

        /**
         * @brief 解除停车状态
         * @return 是否成功解除停车状态
         */
        virtual bool Unpark() {}

        /**
         * @brief 判断望远镜是否在停车位置
         * @return 返回 true 表示位于停车位置，否则返回 false
         */
        virtual bool isAtPark() {}

        /**
         * @brief 设置停车位置
         * @return 是否成功设置停车位置
         */
        virtual bool setParkPosition() {}

        /**
         * @brief 获取望远镜是否可以停车
         * @return 返回 true 表示望远镜可以停车，否则返回 false
         */
        bool isParkAvailable() { return can_park; }

    public:
        std::string mount_type = ""; // 望远镜安装类型

        bool is_slewing = false;  // 是否在指向新目标
        bool is_tracking = false; // 是否正在跟踪运动目标

        std::string current_ra;  // 当前赤经位置
        std::string current_dec; // 当前赤纬位置
        std::string current_az;  // 当前方位角位置
        std::string current_alt; // 当前高度角位置

        std::string current_target_name; // 当前指向的目标名称

        std::string current_lat;       // 当前望远镜所在位置的纬度
        std::string current_lon;       // 当前望远镜所在位置的经度
        std::string current_elevation; // 当前望远镜所在位置的海拔高度

        std::string current_tracking_mode;  // 当前跟踪模式，包括恒星跟踪、太阳跟踪和月球跟踪
        std::string current_tracking_speed; // 当前跟踪速度，默认为1

        bool is_home = false;   // 是否在家位置
        bool is_parked = false; // 是否处于停车状态

        bool can_home = false;          // 是否可以回到家位置
        bool can_park = false;          // 是否可以停车
        bool can_abort = true;          // 是否可以中止指向
        bool can_track_speed = false;   // 是否支持调整跟踪速度
        bool can_slew_speed = false;    // 是否支持调整指向速度
        bool can_guiding_speed = false; // 是否支持调整导星速度
    };

    class Focuser : public Device
    {
    public:
        /**
         * @brief 构造函数，创建一个名为 name 的电调对象
         *
         * @param name 电调名称
         */
        Focuser(const std::string &name);

        /**
         * @brief 析构函数，释放资源
         */
        ~Focuser();

        /**
         * @brief 将电调移动到 position 位置
         *
         * @param position 相对移动的步数
         * @return bool 移动是否成功
         */
        virtual bool moveTo(const int position) {}

        /**
         * @brief 将电调移动到绝对位置 position
         *
         * @param position 绝对位置步数
         * @return bool 移动是否成功
         */
        virtual bool moveToAbsolute(const int position) {}

        /**
         * @brief 移动电调 step 个步长
         *
         * @param step 移动步数
         * @return bool 移动是否成功
         */
        virtual bool moveStep(const int step) {}

        /**
         * @brief 移动电调至绝对步数位置
         *
         * @param step 绝对步数位置
         * @return bool 移动是否成功
         */
        virtual bool moveStepAbsolute(const int step) {}

        /**
         * @brief 中止电调移动
         *
         * @return bool 操作是否成功
         */
        virtual bool AbortMove() {}

        /**
         * @brief 获取电调最大位置
         *
         * @return int 电调最大位置
         */
        virtual int getMaxPosition() {}

        /**
         * @brief 设置电调最大位置
         *
         * @param max_position 电调最大位置
         * @return bool 操作是否成功
         */
        virtual bool setMaxPosition(int max_position) {}

        /**
         * @brief 判断是否支持获取温度功能
         *
         * @return bool 是否支持获取温度功能
         */
        bool isGetTemperatureAvailable() {}

        /**
         * @brief 获取电调当前温度
         *
         * @return double 当前温度
         */
        virtual double getTemperature() {}

        /**
         * @brief 判断是否支持绝对移动功能
         *
         * @return bool 是否支持绝对移动功能
         */
        bool isAbsoluteMoveAvailable() {}

        /**
         * @brief 判断是否支持手动移动功能
         *
         * @return bool 是否支持手动移动功能
         */
        bool isManualMoveAvailable() {}

        /**
         * @brief 获取电调当前位置
         *
         * @return int 当前位置
         */
        virtual int getCurrentPosition() {}

        /**
         * @brief 判断电调是否存在反向间隙
         *
         * @return bool 是否存在反向间隙
         */
        virtual bool haveBacklash() {}

        /**
         * @brief 设置电调反向间隙值
         *
         * @param value 反向间隙值
         * @return bool 操作是否成功
         */
        virtual bool setBacklash(int value) {}

    public:
        bool is_moving; // 电调是否正在移动

        int current_mode;     // 电调当前模式
        int current_motion;   // 电调当前运动状态
        double current_speed; // 电调当前速度

        int current_position; // 电调当前位置
        int max_position;     // 电调可移动的最大位置
        int min_position;     // 电调可移动的最小位置
        int max_step;         // 电调可移动的最大步长

        bool can_get_temperature;   // 是否支持获取温度功能
        double current_temperature; // 当前温度值

        bool can_absolute_move; // 是否支持绝对移动功能
        bool can_manual_move;   // 是否支持手动移动功能

        int delay; // 电调延迟时间

        bool has_backlash;
    };

    class Filterwheel : public Device
    {
    public:
        Filterwheel(const std::string &name);
        ~Filterwheel();

        virtual bool moveTo(const int position) {}

    public:
        int current_position;
        int max_position;
        int min_position;
    };

    class Solver : public Device
    {
    public:
        Solver(const std::string &name);
        ~Solver();

    public:
    };
} // namespace OpenAPT
