#include "debayer.hpp"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

std::tuple<cv::Mat, bool, std::map<std::string, std::string>> Debayer(const std::filesystem::path& filepath) {
    cv::Mat img, fits_img;
    bool continue_process = true;
    std::map<std::string, std::string> header;

    std::string fileExtension = filepath.extension().string();
    if (fileExtension == ".fits" || fileExtension == ".fit") {
        // Suppose we have a readFits function to convert FITS file into cv::Mat
        // As a placeholder, let's simulate loading an image for demonstration
        img = cv::imread("path_to_a_simulated_fits_image_in_png_or_other_format", cv::IMREAD_UNCHANGED);

        // Simulate reading the bayer pattern from the FITS header
        std::string bayerPat = "RGGB"; // This should be read from the header, simulated here

        if (bayerPat == "RGGB") {
            cv::cvtColor(img, fits_img, cv::COLOR_BayerBG2BGR);
        } else if (bayerPat == "GBRG") {
            cv::cvtColor(img, fits_img, cv::COLOR_BayerGB2BGR);
        } else if (bayerPat == "BGGR") {
            cv::cvtColor(img, fits_img, cv::COLOR_BayerRG2BGR);
        } else if (bayerPat == "GRBG") {
            cv::cvtColor(img, fits_img, cv::COLOR_BayerGR2BGR);
        } else {
            cv::cvtColor(img, fits_img, cv::COLOR_BayerBG2BGR);
            continue_process = false;
        }
    } else {
        // Load non-FITS image using OpenCV
        img = cv::imread(filepath.string(), cv::IMREAD_UNCHANGED);

        if (img.channels() == 1) {
            fits_img = img;
        } else {
            continue_process = false;
        }
    }

    return {fits_img, continue_process, header};
}
