/*
 * _component.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-13

Description: Lithium Image Component for Atom Addon

**************************************************/

#include "atom/components/component.hpp"
#include "atom/components/registry.hpp"

#include <cstdint>

#include "base64.hpp"
#include "binning.hpp"
#include "bmp.hpp"
#include "convolve.hpp"
#include "debayer.hpp"
#include "fitsio.hpp"
#include "hfr.hpp"
#include "hist.hpp"
#include "imgio.hpp"
#include "imgutils.hpp"
#include "stack.hpp"
#include "stretch.hpp"

#include "atom/function/overload.hpp"
#include "atom/log/loguru.hpp"

ATOM_MODULE(lithium_image, [](Component& com) {
    LOG_F(INFO, "Lithium Image Component Constructed");

    com.def("base64_encode", &base64Encode, "utils",
            "Encode a string to base64");
    com.def("base64_decode", &base64Decode, "utils",
            "Decode a string from base64");

    com.def("merge_image_based_on_size", &mergeImageBasedOnSize, "utils",
            "Merge image based on size");
    com.def("process_mat_with_bin_avg", &processMatWithBinAvg, "utils",
            "Process a cv::Mat with bin average");
    com.def("process_with_average", &processWithAverage, "utils",
            "Process with average");
    com.def("process_with_binning", &processWithBinning, "utils",
            "Process with binning");
    com.def("calculate_average", &calculateAverage<int>, "utils",
            "Calculate average");

    com.def("little_to_native",
            atom::meta::overload_cast<uint32_t>(&littleToNative), "utils",
            "Little endian to native");
    com.def("little_to_native",
            atom::meta::overload_cast<uint16_t>(&littleToNative), "utils",
            "Little endian to native");

    com.def("read_endian_int", &readEndianInt, "utils", "Read endian int");
    com.def("read_endian_short", &readEndianShort, "utils",
            "Read endian short");

    com.def("load_bmp_image", &loadBMPImage, "utils", "Load BMP image");
    com.def("save_gray_image", &saveGrayImage, "utils", "Save gray image");

    com.def("cv_convolve", &convolve, "utils",
            "Convolve a cv::Mat with a kernel");
    com.def("cv_dft_convolve", &dftConvolve, "utils",
            "Convolve a cv::Mat with a kernel using DFT");
    com.def("cv_deconvolve", &deconvolve, "utils",
            "Deconvolve a cv::Mat with a kernel");

    com.def("cv_debayer", &debayer, "utils", "Debayer a cv::Mat");

    com.def("write_mat_to_fits", &writeMatToFits, "utils",
            "Write a cv::Mat to a FITS file");
    com.def("fits_to_base64", &fitsToBase64, "utils",
            "Convert a FITS file to base64");
    com.def("mat_to_base64", &matToBase64, "utils",
            "Convert a cv::Mat to base64");

    com.def("calc_hfr", &calcHfr, "utils", "Calculate HFR of a cv::Mat");
    com.def("detact_hfr", &starDetectAndHfr, "utils",
            "Detect stars and calculate HFR of a cv::Mat");

    com.def("calc_hist", &calculateHist, "utils",
            "Calculate histogram of a cv::Mat");
    com.def("calc_gray_hist", &calculateGrayHist, "utils",
            "Calculate gray histogram of a cv::Mat");
    com.def("calc_cdf", &calculateCDF, "utils", "Calculate CDF of a histogram");
    com.def("equalize_hist", &equalizeHistogram, "utils",
            "Equalize histogram of a cv::Mat");
    com.def("draw_hist", &drawHistogram, "utils",
            "Draw histogram of a cv::Mat");

    com.def("load_images", &loadImages, "utils", "Load images from a folder");

    com.def("stack_image", &stackImages, "utils", "Stack images from a folder");

    com.def("stretch_wb", &stretchWhiteBalance, "utils",
            "Stretch white balance of a cv::Mat");
    // TODO: How th handle reference argument?
    // com.def("stretch_gray", &StretchGray, "utils", "Stretch gray of a
    // cv::Mat");
});
