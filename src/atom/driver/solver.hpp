/*
 * solver.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-1

Description: Basic Solver Defination

*************************************************/

#pragma once

#include "device.hpp"

enum class SolveStatus
{
    Success = 0,
    Error = 1,
    Timeout = 2,
    Invalid = 3,
    Unknown = 4,
};

struct SolveResult
{
    std::string ra;
    std::string dec;
    double fov_x = 0;
    double fov_y = 0;
    double fov_avg = 0;
    double rotation = 0;

    std::string error;
};

class Solver : public AtomDriver
{
public:
    Solver(const std::string &name);
    ~Solver();

    virtual bool connect(const json &params) override;

    virtual bool disconnect(const json &params) override;

    virtual bool reconnect(const json &params) override;

    virtual bool isConnected() override;

public:
    virtual bool _solveImage(const json &params);

    virtual bool solveImage(const std::string &image, const int &timeout, const bool &debug);

    virtual bool _getSolveResult(const json &params);

    virtual bool getSolveResult(const int &timeout, const bool &debug);

    virtual bool _getSolveStatus(const json &params);

    virtual bool getSolveStatus(const int &timeout, const bool &debug);

    virtual bool _setSolveParams(const json &params);

    virtual bool setSolveParams(const json &params);

    virtual json _getSolveParams(const json &params);

    virtual json getSolveParams();

private:
    std::atomic_bool m_debug;
    std::string m_imagePath;
    std::string m_solverPath;

    int m_timeout;

    std::string m_ra;
    std::string m_dec;
    std::string m_az;
    std::string m_alt;
    double m_radius;
    int m_downsample;
    std::vector<int> m_depth;
    double m_scale_low;
    double m_scale_high;
    int m_width;
    int m_height;
    std::string m_scale_units;
    bool m_overwrite;
    bool m_no_plot;
    bool m_verify;
    bool m_resort;
    bool m_continue;
    bool m_no_tweak;

    SolveStatus m_status;
};