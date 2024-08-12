#include "fitsio.hpp"
#include "base64.hpp"

#include <fitsio.h>
#include <fitsio2.h>
#include <array>
#include <cstddef>
#include <filesystem>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <stdexcept>
#include <vector>

// Helper function to handle FITS errors
void checkFitsStatus(int status, const std::string& errorMessage) {
    if (status != 0) {
        fits_report_error(stderr, status);
        throw std::runtime_error(
            errorMessage + " CFITSIO error code: " + std::to_string(status));
    }
}

cv::Mat readFitsToMat(const std::filesystem::path& filepath) {
    fitsfile* fptr;  // FITS file pointer
    int status = 0;  // CFITSIO status value MUST be initialized to zero
    int bitpix;
    int naxis;
    long naxes[3] = {1, 1, 1};

    // Open the FITS file
    if (fits_open_file(&fptr, filepath.string().c_str(), READONLY, &status)) {
        checkFitsStatus(status, "Cannot open FITS file");
    }

    // Read the image dimensions and bit depth
    if (fits_get_img_param(fptr, 3, &bitpix, &naxis, naxes, &status)) {
        checkFitsStatus(status, "Cannot read FITS image parameters");
    }

    // Determine the type of the image
    int width = naxes[0];
    int height = naxes[1];
    int depth = (bitpix == USHORT_IMG || bitpix == SHORT_IMG) ? CV_16U : CV_8U;

    // Prepare to read the image data
    std::vector<long> fpixel(naxis, 1);  // First pixel to read
    long nelements =
        static_cast<long>(width * height);  // Number of pixels to read

    // Prepare the cv::Mat container
    cv::Mat image;

    // Check if it's a single channel (grayscale) or three channels (RGB) image
    // by naxis and bitpix
    if (naxis == 2 || (naxis == 3 && naxes[2] == 1)) {  // Grayscale image
        image = cv::Mat(height, width, CV_MAKETYPE(depth, 1));
        // Read the image data
        if (fits_read_pix(
                fptr, CV_MAKETYPE(depth, 1) == CV_16U ? TUSHORT : TBYTE,
                &fpixel[0], nelements, nullptr, image.data, nullptr, &status)) {
            checkFitsStatus(status, "Cannot read FITS image data");
        }
    } else if (naxis == 3 && naxes[2] == 3) {  // RGB image
        image = cv::Mat(height, width, CV_MAKETYPE(depth, 3));
        std::vector<cv::Mat> channels(3);
        for (int i = 0; i < 3; ++i) {
            channels[i] = cv::Mat(height, width, CV_MAKETYPE(depth, 1));
            fpixel[2] = i + 1;  // Set the correct channel (plane)
            if (fits_read_pix(fptr,
                              CV_MAKETYPE(depth, 1) == CV_16U ? TUSHORT : TBYTE,
                              &fpixel[0], nelements, nullptr, channels[i].data,
                              nullptr, &status)) {
                checkFitsStatus(status,
                                "Cannot read FITS image data for channel " +
                                    std::to_string(i));
            }
        }
        cv::merge(channels, image);
    } else {
        throw std::runtime_error("Unsupported FITS image format");
    }

    // Close the FITS file
    if (fits_close_file(fptr, &status)) {
        checkFitsStatus(status, "Cannot close FITS file");
    }

    return image;
}

// Convert cv::Mat to FITS file
void writeMatToFits(const cv::Mat& image,
                    const std::filesystem::path& filepath) {
    fitsfile* fptr;
    int status = 0;
    long naxes[3] = {image.cols, image.rows, image.channels()};
    int bitpix = (image.depth() == CV_16U) ? USHORT_IMG : BYTE_IMG;

    if (fits_create_file(&fptr, filepath.string().c_str(), &status)) {
        checkFitsStatus(status, "Cannot create FITS file");
    }

    if (image.channels() == 1) {
        fits_create_img(fptr, bitpix, 2, naxes, &status);
    } else if (image.channels() == 3) {
        fits_create_img(fptr, bitpix, 3, naxes, &status);
    } else {
        throw std::runtime_error("Unsupported number of channels in Mat");
    }

    if (image.channels() == 1) {
        if (fits_write_img(fptr, (image.depth() == CV_16U) ? TUSHORT : TBYTE, 1,
                           image.cols * image.rows, (void*)image.data,
                           &status)) {
            checkFitsStatus(status, "Cannot write FITS image data");
        }
    } else if (image.channels() == 3) {
        std::vector<cv::Mat> channels(3);
        cv::split(image, channels);
        for (int i = 0; i < 3; ++i) {
            long fpixel[3] = {1, 1, i + 1};
            if (fits_write_pix(fptr,
                               (image.depth() == CV_16U) ? TUSHORT : TBYTE,
                               fpixel, image.cols * image.rows,
                               (void*)channels[i].data, &status)) {
                checkFitsStatus(status,
                                "Cannot write FITS image data for channel " +
                                    std::to_string(i));
            }
        }
    }

    if (fits_close_file(fptr, &status)) {
        checkFitsStatus(status, "Cannot close FITS file");
    }
}

// Convert cv::Mat to Base64 string
auto matToBase64(const cv::Mat& image,
                 const std::string& imgFormat) -> std::string {
    std::vector<uchar> buf;
    cv::imencode(imgFormat, image, buf);
    auto base64Encoded = base64_encode(buf.data(), buf.size());
    return base64Encoded;
}

// Convert FITS file to Base64 string
auto fitsToBase64(const std::filesystem::path& filepath) -> std::string {
    cv::Mat image = readFitsToMat(filepath);
    return matToBase64(image, ".png");
}

auto readFitsHeadForDevName(const std::string& filename)
    -> std::optional<std::string> {
    fitsfile* fptr;
    int status = 0;
    char card[FLEN_CARD];

    fits_open_file(&fptr, filename.c_str(), READONLY, &status);
    if (status != 0) {
        fits_report_error(stderr, status);
        return std::nullopt;
    }

    int nkeys;
    fits_get_hdrspace(fptr, &nkeys, nullptr, &status);
    std::optional<std::string> devname;

    for (int i = 1; i <= nkeys; i++) {
        fits_read_record(fptr, i, card, &status);
        std::string s = card;

        if (s.find("INSTRUME") != std::string::npos) {
            size_t a = s.find('\'');
            size_t b = s.rfind('\'');
            if (a != std::string::npos && b != std::string::npos && a != b) {
                devname = s.substr(a + 1, b - a - 1);
            }
        }
    }

    fits_close_file(fptr, &status);
    if (status != 0) {
        fits_report_error(stderr, status);
        return std::nullopt;
    }

    return devname;
}

auto readFits(const std::string& fileName, cv::Mat& image) -> int {
    fitsfile* fptr;
    int status = 0;
    int bitpix;
    int naxis;
    long naxes[2];
    std::unique_ptr<unsigned short[]> array;

    if (fits_open_file(&fptr, fileName.c_str(), READONLY, &status)) {
        return status;
    }

    if (fits_get_img_param(fptr, 2, &bitpix, &naxis, naxes, &status)) {
        return status;
    }

    if (naxis != 2) {
        return -1;
    }

    long nelements = naxes[0] * naxes[1];
    array = std::make_unique<unsigned short[]>(nelements);
    if (fits_read_img(fptr, TUSHORT, 1, nelements, nullptr, array.get(),
                      nullptr, &status)) {
        return status;
    }

    if (bitpix == 16) {
        image = cv::Mat(naxes[1], naxes[0], CV_16U, array.get()).clone();
    } else if (bitpix == 8) {
        image = cv::Mat(naxes[1], naxes[0], CV_8U, array.get()).clone();
    }

    fits_close_file(fptr, &status);
    return status;
}

auto readFits_(const std::string& fileName, cv::Mat& image) -> int {
    fitsfile* fptr;
    int status = 0;

    fits_open_file(&fptr, fileName.c_str(), READONLY, &status);
    if (status != 0) {
        fits_report_error(stderr, status);
        return status;
    }

    int bitpix;
    int naxis;
    long naxes[2] = {1, 1};
    fits_get_img_param(fptr, 2, &bitpix, &naxis, naxes, &status);
    if (status != 0) {
        fits_report_error(stderr, status);
        return status;
    }

    long fpixel[2] = {1, 1};
    image.create(naxes[1], naxes[0], CV_32F);
    fits_read_pix(fptr, TFLOAT, fpixel, naxes[0] * naxes[1], nullptr,
                  image.data, nullptr, &status);
    if (status != 0) {
        fits_report_error(stderr, status);
        return status;
    }

    fits_close_file(fptr, &status);
    return status;
}
