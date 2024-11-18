#include "convolve.hpp"
#include <opencv2/imgproc.hpp>
#include <stdexcept>
#include "atom/log/loguru.hpp"

// Constants
constexpr double EPSILON = 1e-10;
constexpr int MIN_KERNEL_SIZE = 3;

namespace {
// Helper function to validate inputs
void validateInputs(const cv::Mat& input, const cv::Mat& kernel) {
    LOG_F(INFO, "Validating inputs - Image: {}x{}, Kernel: {}x{}", input.cols,
          input.rows, kernel.cols, kernel.rows);

    if (input.empty()) {
        LOG_F(ERROR, "Input image is empty");
        throw std::invalid_argument("Input image is empty");
    }

    if (kernel.empty()) {
        LOG_F(ERROR, "Kernel is empty");
        throw std::invalid_argument("Kernel is empty");
    }

    if (kernel.cols < MIN_KERNEL_SIZE || kernel.rows < MIN_KERNEL_SIZE) {
        LOG_F(ERROR, "Kernel size too small: {}x{}", kernel.cols, kernel.rows);
        throw std::invalid_argument("Kernel size too small");
    }
}
}  // namespace

void convolve(const cv::Mat& input, const cv::Mat& kernel, cv::Mat& output,
              int borderType) {
    LOG_F(INFO, "Starting spatial domain convolution");
    try {
        validateInputs(input, kernel);

        auto start = std::chrono::high_resolution_clock::now();
        cv::filter2D(input, output, -1, kernel, cv::Point(-1, -1), 0,
                     borderType);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration =
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        LOG_F(INFO, "Convolution completed in {}ms", duration.count());
    } catch (const cv::Exception& e) {
        LOG_F(ERROR, "OpenCV error during convolution: {}", e.what());
        throw;
    }
}

void dftConvolve(const cv::Mat& input, const cv::Mat& kernel, cv::Mat& output) {
    LOG_F(INFO, "Starting frequency domain convolution");
    try {
        validateInputs(input, kernel);

        auto start = std::chrono::high_resolution_clock::now();

        cv::Mat padded, kernelPadded;
        int dftM = cv::getOptimalDFTSize(input.rows + kernel.rows - 1);
        int dftN = cv::getOptimalDFTSize(input.cols + kernel.cols - 1);

        LOG_F(INFO, "Optimal DFT size: {}x{}", dftM, dftN);

        cv::copyMakeBorder(input, padded, 0, dftM - input.rows, 0,
                           dftN - input.cols, cv::BORDER_CONSTANT,
                           cv::Scalar::all(0));
        cv::copyMakeBorder(kernel, kernelPadded, 0, dftM - kernel.rows, 0,
                           dftN - kernel.cols, cv::BORDER_CONSTANT,
                           cv::Scalar::all(0));

        padded.convertTo(padded, CV_32F);
        kernelPadded.convertTo(kernelPadded, CV_32F);

        cv::dft(padded, padded, cv::DFT_COMPLEX_OUTPUT);
        cv::dft(kernelPadded, kernelPadded, cv::DFT_COMPLEX_OUTPUT);

        cv::mulSpectrums(padded, kernelPadded, padded, 0);
        cv::dft(padded, padded,
                cv::DFT_INVERSE + cv::DFT_SCALE + cv::DFT_REAL_OUTPUT);

        output = padded(cv::Rect(0, 0, input.cols, input.rows));

        auto end = std::chrono::high_resolution_clock::now();
        auto duration =
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        LOG_F(INFO, "DFT convolution completed in {}ms", duration.count());
    } catch (const cv::Exception& e) {
        LOG_F(ERROR, "OpenCV error during DFT convolution: {}", e.what());
        throw;
    }
}

void deconvolve(const cv::Mat& input, const cv::Mat& kernel, cv::Mat& output,
                double regularization = EPSILON) {
    LOG_F(INFO, "Starting deconvolution with regularization = {}",
          regularization);
    try {
        validateInputs(input, kernel);

        auto start = std::chrono::high_resolution_clock::now();

        cv::Mat inputDFT, kernelDFT;
        int dftM = cv::getOptimalDFTSize(input.rows + kernel.rows - 1);
        int dftN = cv::getOptimalDFTSize(input.cols + kernel.cols - 1);

        LOG_F(INFO, "Optimal DFT size: {}x{}", dftM, dftN);

        cv::Mat tempA, tempB;
        cv::copyMakeBorder(input, tempA, 0, dftM - input.rows, 0,
                           dftN - input.cols, cv::BORDER_CONSTANT,
                           cv::Scalar::all(0));
        cv::copyMakeBorder(kernel, tempB, 0, dftM - kernel.rows, 0,
                           dftN - kernel.cols, cv::BORDER_CONSTANT,
                           cv::Scalar::all(0));

        tempA.convertTo(tempA, CV_32F);
        tempB.convertTo(tempB, CV_32F);

        cv::dft(tempA, inputDFT, cv::DFT_COMPLEX_OUTPUT);
        cv::dft(tempB, kernelDFT, cv::DFT_COMPLEX_OUTPUT);

        // Regularized division in frequency domain
        cv::Mat complexI(inputDFT.size(), inputDFT.type());
        for (int i = 0; i < inputDFT.rows; i++) {
            for (int j = 0; j < inputDFT.cols / 2; j++) {
                float re = kernelDFT.at<cv::Vec2f>(i, j)[0];
                float im = kernelDFT.at<cv::Vec2f>(i, j)[1];
                float denom = re * re + im * im + regularization;

                complexI.at<cv::Vec2f>(i, j)[0] =
                    (inputDFT.at<cv::Vec2f>(i, j)[0] * re +
                     inputDFT.at<cv::Vec2f>(i, j)[1] * im) /
                    denom;
                complexI.at<cv::Vec2f>(i, j)[1] =
                    (inputDFT.at<cv::Vec2f>(i, j)[1] * re -
                     inputDFT.at<cv::Vec2f>(i, j)[0] * im) /
                    denom;
            }
        }

        cv::dft(complexI, output,
                cv::DFT_INVERSE | cv::DFT_SCALE | cv::DFT_REAL_OUTPUT);
        output = output(cv::Rect(0, 0, input.cols, input.rows));

        auto end = std::chrono::high_resolution_clock::now();
        auto duration =
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        LOG_F(INFO, "Deconvolution completed in {}ms", duration.count());
    } catch (const cv::Exception& e) {
        LOG_F(ERROR, "OpenCV error during deconvolution: {}", e.what());
        throw;
    }
}

void separableConvolve(const cv::Mat& input, const cv::Mat& kernelX,
                       const cv::Mat& kernelY, cv::Mat& output) {
    LOG_F(INFO, "Starting separable convolution");
    try {
        if (input.empty() || kernelX.empty() || kernelY.empty()) {
            LOG_F(ERROR, "Invalid inputs for separable convolution");
            throw std::invalid_argument(
                "Invalid inputs for separable convolution");
        }

        auto start = std::chrono::high_resolution_clock::now();

        cv::Mat temp;
        cv::sepFilter2D(input, output, -1, kernelX, kernelY);

        auto end = std::chrono::high_resolution_clock::now();
        auto duration =
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        LOG_F(INFO, "Separable convolution completed in {}ms",
              duration.count());
    } catch (const cv::Exception& e) {
        LOG_F(ERROR, "OpenCV error during separable convolution: {}", e.what());
        throw;
    }
}