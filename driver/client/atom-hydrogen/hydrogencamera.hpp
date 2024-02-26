/*
 * hydrogencamera.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-9

Description: Hydrogen Camera

**************************************************/

#ifndef ATOM_HYDROGEN_CAMERA_HPP
#define ATOM_HYDROGEN_CAMERA_HPP

#include "atom/driver/camera.hpp"
#include "hydrogenbasic.hpp"
#include "atom/utils/switch.hpp"

class CapturedFrame
{
public:
    void *m_data;
    size_t m_size;
    char m_format[MAXHYDROGENBLOBFMT];

    CapturedFrame()
    {
        m_data = nullptr;
        m_size = 0;
        m_format[0] = 0;
    }

    ~CapturedFrame()
    {
#ifdef HYDROGEN_SHARED_BLOB_SUPPORT
        IDSharedBlobFree(m_data);
#else
        free(m_data);
#endif
    }

    // Take ownership of this blob's data, so HYDROGEN won't overwrite/free the memory
    void steal(IBLOB *bp)
    {
        m_data = bp->blob;
        m_size = bp->size;
        strncpy(m_format, bp->format, MAXHYDROGENBLOBFMT);

        bp->blob = nullptr;
        bp->size = 0;
    }
};

class HydrogenCamera : public Camera, public HYDROGEN::BaseClient
{
public:
    // 构造函数
    explicit HydrogenCamera(const std::string &name);
    // 析构函数
    ~HydrogenCamera();

    virtual bool connect(const json &params) override;

    virtual bool disconnect(const json &params) override;

    virtual bool reconnect(const json &params) override;

    virtual bool isConnected() override;

public:
    /**
     * @brief 启动曝光
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool startExposure(const json &params);

    /**
     * @brief 中止曝光
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool abortExposure(const json &params);

    /**
     * @brief 获取曝光状态
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool getExposureStatus(const json &params);

    /**
     * @brief 获取曝光结果
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool getExposureResult(const json &params);

    /**
     * @brief 保存曝光结果
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool saveExposureResult(const json &params);

    /**
     * @brief 启动视频
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool startVideo(const json &params);

    /**
     * @brief 停止视频
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool stopVideo(const json &params);

    /**
     * @brief 获取视频状态
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool getVideoStatus(const json &params);

    /**
     * @brief 获取视频结果
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool getVideoResult(const json &params);

    /**
     * @brief 保存视频结果
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool saveVideoResult(const json &params);

    /**
     * @brief 启动冷却
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool startCooling(const json &params);

    /**
     * @brief 停止冷却
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool stopCooling(const json &params);

    bool isCoolingAvailable();

    /**
     * @brief 获取温度
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool getTemperature(const json &params);

    /**
     * @brief 获取冷却功率
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool getCoolingPower(const json &params);

    /**
     * @brief 设置温度
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool setTemperature(const json &params);

    /**
     * @brief 设置冷却功率
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool setCoolingPower(const json &params);

    /**
     * @brief 获取增益值
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool getGain(const json &params);

    /**
     * @brief 设置增益值
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool setGain(const json &params);

    bool isGainAvailable();

    /**
     * @brief 获取偏移量
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool getOffset(const json &params);

    /**
     * @brief 设置偏移量
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool setOffset(const json &params);

    bool isOffsetAvailable();

    /**
     * @brief 获取ISO值
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool getISO(const json &params);

    /**
     * @brief 设置ISO值
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool setISO(const json &params);

    bool isISOAvailable();

    /**
     * @brief 获取帧数
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool getFrame(const json &params);

    /**
     * @brief 设置帧数
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool setFrame(const json &params);

    bool isFrameSettingAvailable();

protected:
    // 清空状态
    void ClearStatus();

    // Hydrogen Client API
protected:
    void newDevice(HYDROGEN::BaseDevice dp) override;
    void removeDevice(HYDROGEN::BaseDevice dp) override;
    void newProperty(HYDROGEN::Property property) override;
    void updateProperty(HYDROGEN::Property property) override;
    void removeProperty(HYDROGEN::Property property) override {}
    void newMessage(HYDROGEN::BaseDevice dp, int messageID) override;
    void serverConnected() override;
    void serverDisconnected(int exit_code) override;

    void newSwitch(HYDROGEN::PropertyViewSwitch *svp);
    void newNumber(HYDROGEN::PropertyViewNumber *nvp);
    void newText(HYDROGEN::PropertyViewText *tvp);
    void newBLOB(HYDROGEN::PropertyViewBlob *bp);

    // Hydrogen Parameters
private:
    // 连接属性
    std::shared_ptr<HYDROGEN::PropertyViewSwitch> m_connection_prop;
    // 曝光属性
    std::shared_ptr<HYDROGEN::PropertyViewNumber> exposure_prop;
    // 停止曝光属性
    std::shared_ptr<HYDROGEN::PropertyViewSwitch> abort_exposure_prop;
    // 帧属性
    std::shared_ptr<HYDROGEN::PropertyViewNumber> frame_prop;
    // 温度属性
    std::shared_ptr<HYDROGEN::PropertyViewNumber> temperature_prop;
    // 增益属性
    std::shared_ptr<HYDROGEN::PropertyViewNumber> gain_prop;
    // 偏移属性
    std::shared_ptr<HYDROGEN::PropertyViewNumber> offset_prop;
    // 帧区域参数
    std::shared_ptr<INumber> hydrogen_frame_x;
    std::shared_ptr<INumber> hydrogen_frame_y;
    std::shared_ptr<INumber> hydrogen_frame_width;
    std::shared_ptr<INumber> hydrogen_frame_height;
    // 帧类型
    std::shared_ptr<HYDROGEN::PropertyViewSwitch> frame_type_prop;
    // 图像类型
    std::shared_ptr<HYDROGEN::PropertyViewSwitch> frame_format_prop;
    // CCD 设备信息
    std::shared_ptr<HYDROGEN::PropertyViewNumber> ccdinfo_prop;
    // 二次取样属性
    std::shared_ptr<HYDROGEN::PropertyViewNumber> binning_prop;
    // 二次取样 X 轴
    std::shared_ptr<INumber> hydrogen_binning_x;
    // 二次取样 Y 轴
    std::shared_ptr<INumber> hydrogen_binning_y;
    // 视频属性
    std::shared_ptr<HYDROGEN::PropertyViewSwitch> video_prop;
    // 视频延迟
    std::shared_ptr<HYDROGEN::PropertyViewNumber> video_delay_prop;
    // 视频曝光时间
    std::shared_ptr<HYDROGEN::PropertyViewNumber> video_exposure_prop;
    // 视频帧率
    std::shared_ptr<HYDROGEN::PropertyViewNumber> video_fps_prop;
    // 相机端口
    std::shared_ptr<HYDROGEN::PropertyViewText> camera_prop;
    // 相机设备
    HYDROGEN::BaseDevice camera_device;
    // 调试模式
    std::shared_ptr<HYDROGEN::PropertyViewSwitch> debug_prop;
    // 信息刷新间隔
    std::shared_ptr<HYDROGEN::PropertyViewNumber> polling_prop;
    // 已连接的辅助设备
    std::shared_ptr<HYDROGEN::PropertyViewText> active_device_prop;
    // 是否压缩
    std::shared_ptr<HYDROGEN::PropertyViewSwitch> compression_prop;
    // 图像上传模式
    std::shared_ptr<HYDROGEN::PropertyViewSwitch> image_upload_mode_prop;
    // 快速读出模式
    std::shared_ptr<HYDROGEN::PropertyViewSwitch> fast_read_out_prop;
    // 相机限制
    std::shared_ptr<HYDROGEN::PropertyViewNumber> camera_limit_prop;
    // 相机温度
    std::shared_ptr<HYDROGEN::PropertyViewNumber> camera_temperature_prop;

    std::shared_ptr<HYDROGEN::PropertyViewText> cfa_prop;

    std::shared_ptr<IText> cfa_type_prop;

    // 标志位
    std::atomic_bool is_ready; // 是否就绪
    std::atomic_bool has_blob; // 是否有 BLOB 数据
    std::atomic_bool is_debug;
    std::atomic_bool is_connected;
    std::atomic_bool is_exposure;
    std::atomic_bool is_video;
    bool is_color;

    std::atomic_int current_gain;
    std::atomic_int current_offset;
    std::atomic_int current_exposure;
    std::atomic<double> current_temperature;

    // Hydrogen 指令
    std::string hydrogen_camera_cmd = "CCD_"; // Hydrogen 控制命令前缀
    std::string hydrogen_blob_name;           // BLOB 文件名
    std::string hydrogen_camera_exec = "";    // Hydrogen 执行命令
    std::string hydrogen_camera_version;
    std::string hydrogen_camera_interface;
    std::string hydrogen_camera_port;

    CameraFrame frame;

    std::atomic<double> polling_period;

    std::unique_ptr<Atom::Utils::StringSwitch<HYDROGEN::PropertyViewNumber *>> m_number_switch;
    std::unique_ptr<Atom::Utils::StringSwitch<HYDROGEN::PropertyViewSwitch *>> m_switch_switch;
    std::unique_ptr<Atom::Utils::StringSwitch<HYDROGEN::PropertyViewText *>> m_text_switch;

private:
    // For Hydrogen Toupcamera

    std::shared_ptr<HYDROGEN::PropertyViewSwitch> toupcam_fan_control_prop;

    std::shared_ptr<HYDROGEN::PropertyViewSwitch> toupcam_heat_control_prop;

    std::shared_ptr<HYDROGEN::PropertyViewSwitch> toupcam_hcg_control_prop;

    std::shared_ptr<HYDROGEN::PropertyViewSwitch> toupcam_low_noise_control_prop;

    std::shared_ptr<HYDROGEN::PropertyViewSwitch> toupcam_simulation_prop;

    std::shared_ptr<HYDROGEN::PropertyViewSwitch> toupcam_binning_mode_prop;

    // For Hydrogen ZWOASI

    // 图像翻转
    std::shared_ptr<HYDROGEN::PropertyViewSwitch> asi_image_flip_prop;
    // 图像翻转
    std::shared_ptr<HYDROGEN::PropertyViewSwitch> asi_image_flip_hor_prop;
    std::shared_ptr<HYDROGEN::PropertyViewSwitch> asi_image_flip_ver_prop;
    // 控制模式
    std::shared_ptr<HYDROGEN::PropertyViewNumber> asi_controls_prop;
    // 控制模式
    std::shared_ptr<HYDROGEN::PropertyViewSwitch> asi_controls_mode_prop;

    // For Hydrogen QHYCCD
};

#endif