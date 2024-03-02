/*
 * solver.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-1

Description: Basic Solver Defination

*************************************************/

#include "solver.hpp"

#include "macro.hpp"

Solver::Solver(const std::string &name)
    : AtomDriver(name), m_debug(debug), m_imagePath("") {
    SetVariable("debug", m_debug);
    SetVariable("imagePath", m_imagePath);
    SetVariable("solverPath", m_solverPath);
    SetVariable("timeout", m_timeout);
    SetVariable("target_ra", m_ra);
    SetVariable("target_dec", m_dec);
    SetVariable("target_az", m_az);
    SetVariable("target_alt", m_alt);
    SetVariable("radius", m_radius);
    SetVariable("downsample", m_downsample);
    SetVariable("depth", m_depth);
    SetVariable("scale_low", m_scale_low);
    SetVariable("scale_high", m_scale_high);
    SetVariable("width", m_width);
    SetVariable("height", m_height);
    SetVariable("scale_units", m_scale_units);
    SetVariable("overwrite", m_overwrite);
    SetVariable("no_plot", m_no_plot);
    SetVariable("verify", m_verify);
    SetVariable("resort", m_resort);
    SetVariable("continue", m_continue);
    SetVariable("no_tweak", m_no_tweak);
}

Solver::~Solver() {}

bool Solver::connect(const json &params) { return true; }

bool Solver::disconnect(const json &params) { return true; }

bool Solver::reconnect(const json &params) { return true; }

bool Solver::isConnected() { return true; }

bool Solver::_solveImage(const json &params) {
    GET_PARAM(std::string, image)
    GET_PARAM(int, timeout)
    GET_PARAM(bool, debug)

    // Check image path
    // Max: image must be a fits file. Whether it is a little bit a problem?
    if (image.empty() || image.find(".fits") == std::string::npos) {
        LOG_F(ERROR, "Failed to execute solveImage: Invalid Parameters");
        return false;
    } else {
        SetVariable("imagePath", image);
    }

    TOGGLE_DEBUG(debug)
    TOGGLE_TIMEOUT(timeout)

    if (!solveImage(GetVariable("imagePath").value(),
                    GetVariable("timeout").value(),
                    GetVariable("debug").value())) {
        LOG_F(ERROR, "Failed to execute solveImage: Solve Failed");
        return false;
    }

    return true;
}

bool Solver::solveImage(const std::string &image, const int &timeout = 30,
                        const bool &debug = false) {}

bool Solver::_getSolveParams(const json &params) { return true; }

bool Solver::getSolveParams() {}

bool Solver::_getSolveResult(const json &params) { return true; }

bool Solver::getSolveResult(const int &timeout = 30,
                            const bool &debug = false) {}

bool Solver::_getSolveStatus(const json &params) { return true; }

bool Solver::getSolveStatus(const int &timeout = 30,
                            const bool &debug = false) {}

bool Solver::_setSolveParams(const json &params) { return true; }

bool Solver::setSolveParams(const json &params) {}