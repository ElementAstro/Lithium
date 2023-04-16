#include "cvimage.hpp"
#include "plugins/base64.hpp"

#include <fitsio.h>

void LoadImage(const std::string &filename, cv::Mat &img, ImageType type)
{
    try
    {
        std::string fileExt = filename.substr(filename.find_last_of(".") + 1);

        if (fileExt == "fits")
        {
            fitsfile *fitsPtr;
            int status = 0;
            fits_open_file(&fitsPtr, filename.c_str(), READONLY, &status);
            if (status != 0)
            {
                throw std::runtime_error("Failed to open fits file: " + filename);
            }

            int naxis;
            fits_get_img_dim(fitsPtr, &naxis, &status);
            if (status != 0)
            {
                throw std::runtime_error("Failed to get image dimensions from fits file: " + filename);
            }

            if (naxis != 2)
            {
                throw std::runtime_error("Fits file should have 2 dimensions but has " + std::to_string(naxis));
            }

            long naxes[2];
            fits_get_img_size(fitsPtr, 2, naxes, &status);
            if (status != 0)
            {
                throw std::runtime_error("Failed to get image size from fits file: " + filename);
            }

            int bitpix;
            fits_get_img_type(fitsPtr, &bitpix, &status);
            if (status != 0)
            {
                throw std::runtime_error("Failed to get image type from fits file: " + filename);
            }

            auto cvType = CV_8U;
            if (bitpix == BYTE_IMG)
            {
                cvType = CV_8U;
            }
            else if (bitpix == SHORT_IMG)
            {
                cvType = CV_16U;
            }
            else if (bitpix == LONG_IMG)
            {
                cvType = CV_32S;
            }
            else if (bitpix == FLOAT_IMG)
            {
                cvType = CV_32F;
            }
            else if (bitpix == DOUBLE_IMG)
            {
                cvType = CV_64F;
            }
            else
            {
                throw std::runtime_error("Unsupported bitpix value in fits file: " + std::to_string(bitpix));
            }

            img = cv::Mat(naxes[1], naxes[0], cvType);

            // fits_read_img(fitsPtr, reinterpret_cast<void*>(img.data), 1, naxes[0] * naxes[1], NULL, nulval, NULL, &status);
            if (status != 0)
            {
                throw std::runtime_error("Failed to read image from fits file: " + filename);
            }

            fits_close_file(fitsPtr, &status);
            if (status != 0)
            {
                throw std::runtime_error("Failed to close fits file: " + filename);
            }
        }
        else
        {
            int cvLoadType = cv::IMREAD_UNCHANGED;

            if (type == ImageType::GRAY)
            {
                cvLoadType = cv::IMREAD_GRAYSCALE;
            }

            img = cv::imread(filename, cvLoadType);

            if (img.empty())
            {
                throw std::runtime_error("Failed to load image from file: " + filename);
            }
        }

        spdlog::info("LoadImage: successfully loaded image from {}", filename);
    }
    catch (std::exception &e)
    {
        spdlog::error("LoadImage: {}", e.what());
        throw;
    }
}

void RotateImage(const cv::Mat &src, cv::Mat &dst, double angle, double scale /*= 1.0*/)
{
    try
    {
        cv::Point2f center(src.cols / 2.0, src.rows / 2.0);
        cv::Mat rot = cv::getRotationMatrix2D(center, angle, scale);
        cv::warpAffine(src, dst, rot, src.size());

        spdlog::info("RotateImage: successfully rotated image by {} degrees", angle);
    }
    catch (std::exception &e)
    {
        spdlog::error("RotateImage: {}", e.what());
        throw;
    }
}

void CropImage(const cv::Mat &src, cv::Mat &dst, const cv::Rect &roi)
{
    try
    {
        dst = src(roi).clone();

        spdlog::info("CropImage: successfully cropped image");
    }
    catch (std::exception &e)
    {
        spdlog::error("CropImage: {}", e.what());
        throw;
    }
}

void CompressImage(const cv::Mat &src, const std::string &filename, int quality)
{
    try
    {
        std::vector<int> params;
        params.push_back(cv::IMWRITE_JPEG_QUALITY);
        params.push_back(quality);
        cv::imwrite(filename, src, params);

        spdlog::info("CompressImage: successfully compressed image to {}", filename);
    }
    catch (std::exception &e)
    {
        spdlog::error("CompressImage: {}", e.what());
        throw;
    }
}

void SharpenImage(const cv::Mat &src, cv::Mat &dst)
{
    try
    {
        cv::Mat kernel = (cv::Mat_<double>(3, 3) << 0, -1, 0, -1, 5, -1, 0, -1, 0);
        cv::filter2D(src, dst, -1, kernel);

        spdlog::info("SharpenImage: successfully sharpened image");
    }
    catch (std::exception &e)
    {
        spdlog::error("SharpenImage: {}", e.what());
        throw;
    }
}

void FlipImage(const cv::Mat &src, cv::Mat &dst, int flipCode)
{
    try
    {
        cv::flip(src, dst, flipCode);

        spdlog::info("FlipImage: successfully flipped image");
    }
    catch (std::exception &e)
    {
        spdlog::error("FlipImage: {}", e.what());
        throw;
    }
}

void StretchImage(const cv::Mat &src, cv::Mat &dst, double fx, double fy)
{
    try
    {
        cv::resize(src, dst, cv::Size(), fx, fy);

        spdlog::info("StretchImage: successfully stretched image");
    }
    catch (std::exception &e)
    {
        spdlog::error("StretchImage: {}", e.what());
        throw;
    }
}

void BlurImage(const cv::Mat &src, cv::Mat &dst, int ksize, double sigmaX, double sigmaY)
{
    try
    {
        cv::GaussianBlur(src, dst, cv::Size(ksize, ksize), sigmaX, sigmaY);

        spdlog::info("BlurImage: successfully blurred image");
    }
    catch (std::exception &e)
    {
        spdlog::error("BlurImage: {}", e.what());
        throw;
    }
}

void CalculateHistogram(const cv::Mat &src, std::vector<cv::Mat> &hist, int bins /*= 256*/)
{
    try
    {
        std::vector<cv::Mat> bgr_planes;
        cv::split(src, bgr_planes);
        hist.clear();
        for (int i = 0; i < 3; i++)
        {
            cv::Mat plane_hist;
            cv::calcHist(&bgr_planes[i], 1, 0, cv::Mat(), plane_hist, 1, &bins, 0);
            hist.push_back(plane_hist);
        }

        spdlog::info("CalculateHistogram: successfully calculated histogram");
    }
    catch (std::exception &e)
    {
        spdlog::error("CalculateHistogram: {}", e.what());
        throw;
    }
}

int GetImageDepth(const cv::Mat &src)
{
    try
    {
        return src.depth();
    }
    catch (std::exception &e)
    {
        spdlog::error("GetImageDepth: {}", e.what());
        throw;
    }
}

void CalibrateImage(const cv::Mat &src, cv::Mat &dst, const cv::Mat &cameraMatrix, const cv::Mat &distCoeffs)
{
    try
    {
        cv::undistort(src, dst, cameraMatrix, distCoeffs);

        spdlog::info("CalibrateImage: successfully calibrated image");
    }
    catch (std::exception &e)
    {
        spdlog::error("CalibrateImage: {}", e.what());
        throw;
    }
}

void OverlayImage(const cv::Mat &background, const cv::Mat &foreground, cv::Mat &dst, const cv::Point &pos)
{
    try
    {
        dst = background.clone();
        cv::Rect roi(pos.x, pos.y, foreground.cols, foreground.rows);
        cv::Mat roi_dst = dst(roi);
        cv::addWeighted(foreground, 1.0, roi_dst, 0.7, 0.0, roi_dst);

        spdlog::info("OverlayImage: successfully overlaid image");
    }
    catch (std::exception &e)
    {
        spdlog::error("OverlayImage: {}", e.what());
        throw;
    }
}

void ConvolveImage(const cv::Mat &src, cv::Mat &dst, const cv::Mat &kernel, ConvolutionType type)
{
    switch (type)
    {
    case CONVOLUTION:
        cv::filter2D(src, dst, -1, kernel);
        break;
    case CORRELATION:
        cv::filter2D(src, dst, -1, kernel, cv::Point(-1, -1), 0, cv::BORDER_REPLICATE);
        break;
    case SHARPENING:
        cv::Mat temp;
        cv::filter2D(src, temp, -1, kernel);
        cv::addWeighted(src, 1.0, temp, -1.0, 0, dst);
        break;
    }
}

// 对图像进行反卷积
void DeconvolveImage(const cv::Mat &src, cv::Mat &dst, const cv::Mat &kernel, double eps /*= 10e-8*/, int maxIter /*= 100*/)
{
    cv::Mat kernelFFT;
    cv::dft(kernel, kernelFFT, cv::DFT_SCALE | cv::DFT_COMPLEX_OUTPUT);

    cv::Mat srcFFT;
    cv::dft(src, srcFFT, cv::DFT_SCALE | cv::DFT_COMPLEX_OUTPUT);

    cv::Mat H1, H2;
    cv::polarToCart(cv::Mat_<double>(srcFFT.size()), cv::Mat_<double>(srcFFT.size()) * CV_PI / 180, H1, H2);
    cv::Mat weight = H1.mul(H1) + H2.mul(H2);
    cv::Mat tmp;

    for (int i = 0; i < maxIter; ++i)
    {
        cv::mulSpectrums(srcFFT, kernelFFT, tmp, 0, true);
        cv::dft(tmp, tmp, cv::DFT_INVERSE | cv::DFT_SCALE | cv::DFT_REAL_OUTPUT);
        tmp.setTo(eps, tmp <= eps);
        srcFFT = srcFFT.mul(weight) / tmp;
        cv::mulSpectrums(srcFFT, kernelFFT, srcFFT, 0, false);
    }

    cv::dft(srcFFT, dst, cv::DFT_INVERSE | cv::DFT_SCALE | cv::DFT_REAL_OUTPUT);
}

void DetectStarPoints(const cv::Mat &src, std::vector<cv::KeyPoint> &keypoints, double threshold /*= 50.0*/)
{
    cv::Ptr<cv::FeatureDetector> detector = cv::FastFeatureDetector::create(threshold);
    detector->detect(src, keypoints);
}

double CalculateExposureTime(const cv::Mat &src, double aperture, double shutterSpeed, double iso)
{
    double exposureTime = ((double)aperture * aperture) / (iso * shutterSpeed * src.at<uchar>(src.rows / 2, src.cols / 2));
    return exposureTime;
}

void AdjustBrightness(const cv::Mat &src, cv::Mat &dst, int delta)
{
    cv::Mat temp = cv::Mat::zeros(src.size(), src.type());
    if (delta >= 0)
    {
        temp.setTo(cv::Scalar(delta));
        cv::add(src, temp, dst);
    }
    else
    {
        temp.setTo(cv::Scalar(-delta));
        cv::subtract(src, temp, dst);
    }
}

std::pair<float, int> clacStarInfo(cv::Mat iMat, int outDiameter)
{
    /* 计算 HFD */
    double out = outDiameter / 2;
    float sum = 0, sumDist = 0;
    int centerX = iMat.rows / 2;
    int centerY = iMat.cols / 2;

    cv::Scalar mean;
    cv::Scalar stdDev;
    cv::meanStdDev(iMat, mean, stdDev);

    for (int x = 0; x < iMat.rows; x++)
    {
        float *pSrc = iMat.ptr<float>(x);
        for (int y = 0; y < iMat.cols; y++)
        {
            if (pow(x - centerX, 2.0) + pow(y - centerY, 2.0) <= pow(out, 2.0))
            {
                sum += pSrc[y];
                sumDist += pSrc[y] * sqrt(pow((float)x - (float)centerX, 2.0f) + pow((float)y - (float)centerY, 2.0f));
            }
        }
    }

    float HFD = ((int)((sum ? 2.0 * sumDist / sum : sqrt(2.0) * out) * 100)) / 100.0f;

    /* 计算星点数量 */
    cv::Mat gray;
    if (iMat.channels() == 3)
    {
        cv::cvtColor(iMat, gray, cv::COLOR_BGR2GRAY);
    }
    else if (iMat.channels() == 1)
    {
        gray = iMat;
    }

    cv::Mat blurMat;
    cv::GaussianBlur(gray, blurMat, cv::Size(5, 5), 2, 2);

    cv::Mat binMat;
    cv::threshold(blurMat, binMat, mean[0] + stdDev[0], 255, cv::THRESH_BINARY);

    std::vector<cv::Vec3f> circles;
    double dp = 1.8;
    double minDist = 10;
    double param1 = 80;
    double param2 = 30;
    int min_radius = 3;
    int max_radius = 7;
    cv::HoughCircles(binMat, circles, cv::HOUGH_GRADIENT, dp, minDist, param1, param2, min_radius, max_radius);

    int starIndex = circles.size();

    return std::make_pair(HFD, starIndex);
}

/**
 * @brief 将unsigned char类型的图像数据转换为base64编码的字符串
 *
 * @param imgBuf 图像数据指针
 * @param isColor 是否为彩色图像
 * @param ImageHeight 图像高度
 * @param ImageWidth 图像宽度
 * @return std::string base64编码的字符串
 */
std::string ConvertUCto64(unsigned char *imgBuf, bool isColor, int ImageHeight, int ImageWidth)
{
    // 图像质量设置
    static std::vector<int> compression_params = {cv::IMWRITE_JPEG_QUALITY, 100};
    // 存储编码后的图像数据
    static std::vector<uchar> vecImg;

    // 转换为cv::Mat格式并进行图像编码
    cv::Mat img;
    if (isColor)
    {
        img = cv::Mat(ImageHeight, ImageWidth, CV_8UC3, imgBuf); // 3通道图像信息
    }
    else
    {
        img = cv::Mat(ImageHeight, ImageWidth, CV_8UC1, imgBuf); // 单通道图像信息
    }
    clacStarInfo(img, 21); // 计算图像中的星星信息
    cv::imencode(".jpg", img, vecImg, compression_params);

    // 返回base64编码后的字符串
    std::vector<unsigned char> imgData;
    cv::imencode(".jpg", img, imgData, compression_params);
    clacStarInfo(img, 21); // 计算图像中的星星信息
    return base64Encode(imgData);
}

/*
int main() {
    cv::Mat grayImg, colorImg;

    cv::Mat rotatedImg;
    RotateImage(colorImg, rotatedImg, 45.0);

    cv::Mat croppedImg;
    CropImage(colorImg, croppedImg, cv::Rect(50, 50, 300, 300));

    CompressImage(colorImg, "compressed.jpg", 50);

    cv::Mat sharpenedImg;
    SharpenImage(colorImg, sharpenedImg);

    cv::Mat flippedImg;
    FlipImage(colorImg, flippedImg, 1);

    cv::Mat stretchedImg;
    StretchImage(colorImg, stretchedImg, 2.0, 2.0);

    cv::Mat blurredImg;
    BlurImage(colorImg, blurredImg, 5, 5.0, 5.0);

    std::vector<cv::Mat> histo;
    CalculateHistogram(colorImg, histo, 256);

    int imageDepth = GetImageDepth(colorImg);

    cv::Mat calibratedImg;
    cv::Mat cameraMatrix, distCoeffs;
    CalibrateImage(colorImg, calibratedImg, cameraMatrix, distCoeffs);

    cv::Mat overlayedImg;
    OverlayImage(colorImg, rotatedImg, overlayedImg, cv::Point(50, 50));

    cv::Mat kernel = (cv::Mat_<double>(3, 3) << -1, -1, -1, -1, 9, -1, -1, -1, -1);
    cv::Mat convolvedImg;
    ConvolveImage(colorImg, convolvedImg, kernel);

    cv::Mat deconvolvedImg;
    DeconvolveImage(convolvedImg, deconvolvedImg, kernel);

    std::vector<cv::KeyPoint> keypoints;
    DetectStarPoints(grayImg, keypoints, 50.0);

    double exposureTime = CalculateExposureTime(colorImg, 1.8, 1 / 1000.0, 200);

    cv::Mat brightenedImg;
    AdjustBrightness(colorImg, brightenedImg, 50);

    return 0;
}
*/
