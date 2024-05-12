#include "convolve.hpp"

#include <opencv2/imgproc.hpp>

void convolve(const cv::Mat& input, const cv::Mat& kernel, cv::Mat& output) {
    // Use filter2D to apply convolution
    cv::filter2D(input, output, -1, kernel, cv::Point(-1, -1), 0,
                 cv::BORDER_DEFAULT);
}

void dftConvolve(const cv::Mat& input, const cv::Mat& kernel, cv::Mat& output) {
    cv::Mat padded;  // add zero padding for convolution
    int dftM = cv::getOptimalDFTSize(input.rows + kernel.rows - 1);
    int dftN = cv::getOptimalDFTSize(input.cols + kernel.cols - 1);

    cv::copyMakeBorder(input, padded, 0, dftM - input.rows, 0,
                       dftN - input.cols, cv::BORDER_CONSTANT,
                       cv::Scalar::all(0));

    cv::Mat kernelPadded;
    cv::copyMakeBorder(kernel, kernelPadded, 0, dftM - kernel.rows, 0,
                       dftN - kernel.cols, cv::BORDER_CONSTANT,
                       cv::Scalar::all(0));

    // Convert to 32F for DFT
    padded.convertTo(padded, CV_32F);
    kernelPadded.convertTo(kernelPadded, CV_32F);

    // DFT
    cv::dft(padded, padded, cv::DFT_COMPLEX_OUTPUT);
    cv::dft(kernelPadded, kernelPadded, cv::DFT_COMPLEX_OUTPUT);

    // Multiply in frequency domain
    cv::mulSpectrums(padded, kernelPadded, padded, 0);

    // Inverse DFT
    cv::dft(padded, padded,
            cv::DFT_INVERSE + cv::DFT_SCALE + cv::DFT_REAL_OUTPUT);

    // Crop the image to undo padding
    output = padded(cv::Rect(0, 0, input.cols, input.rows));
}

void deconvolve(const cv::Mat& input, const cv::Mat& kernel, cv::Mat& output) {
    // First, compute the DFT of the input image and kernel
    cv::Mat inputDFT, kernelDFT;
    int dftM = cv::getOptimalDFTSize(input.rows + kernel.rows - 1);
    int dftN = cv::getOptimalDFTSize(input.cols + kernel.cols - 1);

    cv::Mat tempA, tempB;
    cv::copyMakeBorder(input, tempA, 0, dftM - input.rows, 0, dftN - input.cols,
                       cv::BORDER_CONSTANT, cv::Scalar::all(0));
    cv::copyMakeBorder(kernel, tempB, 0, dftM - kernel.rows, 0,
                       dftN - kernel.cols, cv::BORDER_CONSTANT,
                       cv::Scalar::all(0));

    cv::dft(tempA, inputDFT, cv::DFT_COMPLEX_OUTPUT);
    cv::dft(tempB, kernelDFT, cv::DFT_COMPLEX_OUTPUT);

    // Divide in frequency domain
    cv::Mat divResult;
    cv::mulSpectrums(inputDFT, kernelDFT, divResult, 0,
                     true);  // true for conjugate multiplication

    // Inverse DFT to get the result
    cv::Mat inverseDFT;
    cv::dft(divResult, inverseDFT,
            cv::DFT_INVERSE | cv::DFT_SCALE | cv::DFT_REAL_OUTPUT);

    output = inverseDFT(cv::Rect(0, 0, input.cols, input.rows));
}