#pragma once

#include <opencv2/opencv.hpp>

enum ConvolutionType
{
    CONVOLUTION,
    CORRELATION,
    SHARPENING
};

template <typename T>
class Kernel
{
public:
    static cv::Mat Identity()
    {
        return (cv::Mat_<T>(3, 3) << 0, 0, 0, 0, 1, 0, 0, 0, 0);
    }

    static cv::Mat BoxBlur(int ksize = 3)
    {
        cv::Mat kernel = cv::Mat::ones(ksize, ksize, CV_32F) / (float)(ksize * ksize);
        return kernel;
    }

    static cv::Mat GaussianBlur(int ksize = 3, double sigmaX = 0, double sigmaY = 0)
    {
        cv::Mat kernel = cv::getGaussianKernel(ksize, sigmaX, CV_32F);
        cv::Mat kernel2d = kernel * kernel.t();
        return kernel2d;
    }

    static cv::Mat EdgeEnhancement()
    {
        return (cv::Mat_<T>(3, 3) << 0, -1, 0, -1, 5, -1, 0, -1, 0);
    }

    static cv::Mat Sharpen()
    {
        return (cv::Mat_<T>(3, 3) << -1, -1, -1, -1, 9, -1, -1, -1, -1);
    }

    static cv::Mat Emboss()
    {
        return (cv::Mat_<T>(3, 3) << -2, -1, 0, -1, 1, 1, 0, 1, 2);
    }
};

/*
cv::Mat identity = Kernel<double>::Identity();
cv::Mat boxBlur = Kernel<float>::BoxBlur(5);
cv::Mat gaussBlur = Kernel<float>::GaussianBlur(7, 5.0, 5.0);
cv::Mat edgeEnhance = Kernel<double>::EdgeEnhancement();
cv::Mat sharpen = Kernel<double>::Sharpen();
cv::Mat emboss = Kernel<double>::Emboss();
cv::Mat result;
ConvolveImage(colorImg, result, gaussBlur, CONVOLUTION);
*/

enum class ImageType
{
    COLOR,
    GRAY
};

/**
 * @brief 从指定路径加载图像
 * @param filename 图像文件路径
 * @param img 加载后的图像
 * @param type 图像类型，可选值为 COLOR、GRAYSCALE 或 UNCHANGED，默认为 COLOR（彩色图像）
 */
void LoadImage(const std::string &filename, cv::Mat &img, ImageType type = ImageType::COLOR);

/**
 * @brief 旋转图像
 * @param src 原始图像
 * @param dst 旋转后的图像
 * @param angle 旋转角度（单位为度）
 * @param scale 缩放比例，默认为 1.0（不缩放）
 */
void RotateImage(const cv::Mat &src, cv::Mat &dst, double angle, double scale = 1.0);

/**
 * @brief 根据矩形框裁剪图像
 * @param src 原始图像
 * @param dst 裁剪后的图像
 * @param roi 矩形框
 */
void CropImage(const cv::Mat &src, cv::Mat &dst, const cv::Rect &roi);

/**
 * @brief 将图像进行压缩并保存到文件
 * @param src 原始图像
 * @param filename 保存的文件路径
 * @param quality 压缩质量，取值范围从 0 到 100，数值越大代表压缩质量越高，默认值为 95
 */
void CompressImage(const cv::Mat &src, const std::string &filename, int quality);

/**
 * @brief 对图像进行锐化
 * @param src 原始图像
 * @param dst 锐化后的图像
 */
void SharpenImage(const cv::Mat &src, cv::Mat &dst);

/**
 * @brief 按照指定方向翻转图像
 * @param src 原始图像
 * @param dst 翻转后的图像
 * @param flipCode 翻转方向，可选值为 0（沿 X 轴翻转）、1（沿 Y 轴翻转）或 -1（沿 X 轴和 Y 轴同时翻转）
 */
void FlipImage(const cv::Mat &src, cv::Mat &dst, int flipCode);

/**
 * @brief 对图像进行拉伸
 * @param src 原始图像
 * @param dst 拉伸后的图像
 * @param fx 在 X 轴方向上的缩放比例
 * @param fy 在 Y 轴方向上的缩放比例
 */
void StretchImage(const cv::Mat &src, cv::Mat &dst, double fx, double fy);

/**
 * @brief 对图像进行高斯模糊
 * @param src 原始图像
 * @param dst 模糊后的图像
 * @param ksize 卷积核的大小
 * @param sigmaX X 方向上的标准差
 * @param sigmaY Y 方向上的标准差，如果为 0，则默认使用 sigmaX 的值
 */
void BlurImage(const cv::Mat &src, cv::Mat &dst, int ksize, double sigmaX, double sigmaY = 0);

/**
 * @brief 计算图像的直方图
 * @param src 原始图像
 * @param hist 直方图
 * @param bins 直方图的 bin 数量，默认为 256
 */
void CalculateHistogram(const cv::Mat &src, std::vector<cv::Mat> &hist, int bins = 256);

/**
 * @brief 获取图像的位深度（每个像素所占用的比特数）
 * @param src 原始图像
 * @return 图像的位深度
 */
int GetImageDepth(const cv::Mat &src);

/**
 * @brief 校准图像的畸变和仿射变换
 * @param src 原始图像
 * @param dst 校准后的图像
 * @param cameraMatrix 相机内参矩阵
 * @param distCoeffs 相机的畸变参数
 */
void CalibrateImage(const cv::Mat &src, cv::Mat &dst, const cv::Mat &cameraMatrix, const cv::Mat &distCoeffs);

/**
 * @brief 在背景图像上叠加前景图像
 * @param background 背景图像
 * @param foreground 前景图像
 * @param dst 叠加后的图像
 * @param pos 在背景图像中前景图像的位置
 */
void OverlayImage(const cv::Mat &background, const cv::Mat &foreground, cv::Mat &dst, const cv::Point &pos);

/**
 * @brief 对图像进行卷积操作
 * @param src 原始图像
 * @param dst 卷积后的图像
 * @param kernel 卷积核
 * @param type 卷积类型（FULL、SAME 或 VALID）
 */
void ConvolveImage(const cv::Mat &src, cv::Mat &dst, const cv::Mat &kernel, ConvolutionType type);

/**
 * @brief 对图像进行反卷积操作
 * @param src 原始图像
 * @param dst 反卷积后的图像
 * @param kernel 卷积核
 * @param eps 求解过程中的收敛精度，默认为 10e-8
 * @param maxIter 最大迭代次数，默认为 100
 */
void DeconvolveImage(const cv::Mat &src, cv::Mat &dst, const cv::Mat &kernel, double eps = 10e-8, int maxIter = 100);

/**
 * @brief 检测图像中的关键点
 * @param src 原始图像
 * @param keypoints 检测出的关键点
 * @param threshold 关键点的阈值，值越大检测出的关键点越少
 */
void DetectStarPoints(const cv::Mat &src, std::vector<cv::KeyPoint> &keypoints, double threshold = 50.0);

/**
 * @brief 计算曝光时间
 * @param src 原始图像
 * @param aperture 光圈大小
 * @param shutterSpeed 快门速度
 * @param iso ISO 值
 * @return 曝光时间
 */
double CalculateExposureTime(const cv::Mat &src, double aperture, double shutterSpeed, double iso);

/**
 * @brief 调整图像的亮度
 * @param src 原始图像
 * @param dst 亮度调整后的图像
 * @param delta 亮度调整量，可以为正数或负数
 */
void AdjustBrightness(const cv::Mat &src, cv::Mat &dst, int delta);
