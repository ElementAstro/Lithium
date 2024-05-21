/*
 * _component.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-13

Description: Lithium Image Component for Atom Addon

**************************************************/

#include "_component.hpp"

#include "base64.hpp"
#include "convolve.hpp"
#include "debayer.hpp"
#include "fitsio.hpp"
#include "hfr.hpp"
#include "hist.hpp"
#include "imgutils.hpp"
#include "stack.hpp"
#include "stretch.hpp"

#include "atom/log/loguru.hpp"

ImageComponent::ImageComponent(const std::string& name) : Component(name) {
    LOG_F(INFO, "Lithium Image Component Constructed");

    def("base64_encode", &base64_encode, "utils", "Encode a string to base64");
    def("base64_decode", &base64_decode, "utils",
        "Decode a string from base64");

    def("cv_convolve", &convolve, "utils", "Convolve a cv::Mat with a kernel");
    // def("cv_dft_convolve", &dftConvolve, "utils",
    //     "Convolve a cv::Mat with a kernel using DFT");
    // def("cv_deconvolve", &deconvolve, "utils",
    //     "Deconvolve a cv::Mat with a kernel");

    def("cv_debayer", &Debayer, "utils", "Debayer a cv::Mat");

    def("check_fits_status", &checkFitsStatus, "utils", "Check FITS status");
    def("read_fits_to_mat", &readFitsToMat, "utils",
        "Read a FITS file to a cv::Mat");
    def("write_mat_to_fits", &writeMatToFits, "utils",
        "Write a cv::Mat to a FITS file");
    def("fits_to_base64", &fitsToBase64, "utils",
        "Convert a FITS file to base64");
    def("mat_to_base64", &matToBase64, "utils", "Convert a cv::Mat to base64");

    def("calc_hfr", &calcHfr, "utils", "Calculate HFR of a cv::Mat");
    def("detact_hfr", &StarDetectAndHfr, "utils",
        "Detect stars and calculate HFR of a cv::Mat");

    def("calc_hist", &CalHist, "utils", "Calculate histogram of a cv::Mat");
    def("calc_gray_hist", &CalGrayHist, "utils",
        "Calculate gray histogram of a cv::Mat");

    def("load_images", &loadImages, "utils", "Load images from a folder");

    def("stack_image", &stackImages, "utils", "Stack images from a folder");

    def("stretch_wb", &Stretch_WhiteBalance, "utils",
        "Stretch white balance of a cv::Mat");
    def("stretch_gray", &StretchGray, "utils", "Stretch gray of a cv::Mat");
}

ImageComponent::~ImageComponent() {
    LOG_F(INFO, "Lithium Image Component Destructed");
}

bool ImageComponent::initialize() {
    LOG_F(INFO, "Lithium Image Component Initialized");
    return true;
}

bool ImageComponent::destroy() {
    LOG_F(INFO, "Lithium Image Component Destroyed");
    return true;
}
