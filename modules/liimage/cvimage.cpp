#include "cvimage.hpp"

#include <iostream>
#include <cmath>
#include <numeric>

#include <loguru/loguru.hpp>

using namespace cv;

void CalHist(Mat img, std::vector<Mat> &bgr_planes, std::vector<Mat> &hist)
{
    int histSize = 65535;
    float range[] = {0, 65535};
    const float *histRange = {range};
    bool accumulate = false;

    split(img, bgr_planes);

    for (int i = 0; i < 3; i++)
    {
        calcHist(&bgr_planes[i], 1, 0, Mat(), hist[i], 1, &histSize, &histRange, accumulate);
    }
}

Mat CalGrayHist(Mat img)
{
    int histSize = 65535;
    float range[] = {0, 65535};
    const float *histRange = {range};
    bool accumulate = false;

    Mat gray_hist;
    calcHist(&img, 1, 0, Mat(), gray_hist, 1, &histSize, &histRange, accumulate);

    return gray_hist;
}

Mat Stretch_WhiteBalance(std::vector<Mat> hists, std::vector<Mat> bgr_planes)
{
    std::vector<Mat> planes;
    std::vector<double> highs(3);
    double max_para = 0.0001;
    double min_para = 0.0001;

    for (int i = 0; i < 3; i++)
    {
        Mat hist = hists[i];
        Mat plane = bgr_planes[i];

        Mat nonzero_indices;
        findNonZero(hist, nonzero_indices);

        int nonezero_len = nonzero_indices.rows;
        int min_index = nonezero_len * min_para;
        int max_index = nonezero_len * (1 - max_para) - 1;
        double min = nonzero_indices.at<Point>(min_index).x;
        double max = nonzero_indices.at<Point>(max_index).x;

        plane.convertTo(plane, CV_32S);
        plane = (plane - min) / (max - min) * 65535;
        threshold(plane, plane, 65535, 65535, THRESH_TRUNC);
        plane.convertTo(plane, CV_16U);

        planes.push_back(plane);
        highs[i] = (hist.rows - min) / (max - min) * 65535;
    }

    double high_mean = std::accumulate(highs.begin(), highs.end(), 0.0) / highs.size();

    Mat planes_b = planes[0] * (high_mean / highs[0]);
    Mat planes_g = planes[1] * (high_mean / highs[1]);
    Mat planes_r = planes[2] * (high_mean / highs[2]);

    threshold(planes_b, planes_b, 65535, 65535, THRESH_TRUNC);
    threshold(planes_g, planes_g, 65535, 65535, THRESH_TRUNC);
    threshold(planes_r, planes_r, 65535, 65535, THRESH_TRUNC);
    planes_b.convertTo(planes_b, CV_16U);
    planes_g.convertTo(planes_g, CV_16U);
    planes_r.convertTo(planes_r, CV_16U);

    Mat dst;
    merge(std::vector<Mat>{planes_b, planes_g, planes_r}, dst);

    return dst;
}
Mat StretchGray(Mat hist, Mat plane)
{
    double max_para = 0.01;
    double min_para = 0.01;

    Mat nonzero_indices;
    findNonZero(hist, nonzero_indices);

    int nonezero_len = nonzero_indices.rows;
    int min_index = nonezero_len * min_para;
    int max_index = nonezero_len * (1 - max_para) - 1;
    double min = nonzero_indices.at<Point>(min_index).x;
    double max = nonzero_indices.at<Point>(max_index).x;

    threshold(plane, plane, max, 65535, THRESH_TRUNC);
    threshold(plane, plane, min, 0, THRESH_TOZERO);

    plane.convertTo(plane, CV_16U);

    Mat stretch_plane;
    plane.convertTo(stretch_plane, CV_32F);

    // 计算中值（使用排序方式）
    Mat sorted_plane;
    sort(stretch_plane.reshape(1), sorted_plane, SORT_ASCENDING);
    double gradMed = sorted_plane.at<float>(sorted_plane.rows / 2);

    double Mt = gradMed / 30000;
    pow(stretch_plane / 65535, 1 / Mt, stretch_plane);
    threshold(stretch_plane, stretch_plane, 65535, 65535, THRESH_TRUNC);
    stretch_plane.convertTo(stretch_plane, CV_16U);

    return stretch_plane;
}

Mat DebayerStarCount(Mat img, int Thres)
{
    Mat gray;
    if (img.channels() == 3)
    {
        cvtColor(img, gray, COLOR_BGR2GRAY);
    }
    else
    {
        gray = img;
    }

    Mat blur;
    GaussianBlur(gray, blur, Size(5, 5), 0);

    Mat thresh;
    threshold(blur, thresh, Thres, 255, THRESH_BINARY);

    std::vector<std::vector<Point>> contours;
    std::vector<Vec4i> hierarchy;
    findContours(thresh, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    for (size_t i = 0; i < contours.size(); i++)
    {
        double area = contourArea(contours[i]);
        if (area >= 10)
        {
            Point2f center;
            float radius;
            minEnclosingCircle(contours[i], center, radius);
            int x = static_cast<int>(center.x);
            int y = static_cast<int>(center.y);
            circle(img, center, static_cast<int>(radius), Scalar(0, 0, 255), 2);
        }
    }

    return img;
}

bool insideCircle(int x, int y, int centerX, int centerY, float radius)
{
    return sqrt(pow(x - centerX, 2) + pow(y - centerY, 2)) < radius;
}

double calcHfd(Mat inImage, int inOuterDiameter)
{
    Mat Img = inImage - mean(inImage)[0];
    Img = max(Img, 0);

    int width = Img.cols;
    int height = Img.rows;
    double outerRadius = inOuterDiameter / 2.0;

    double sum = 0;
    double sumDist = 0;
    int centerX = ceil(width / 2.0);
    int centerY = ceil(height / 2.0);

    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            if (insideCircle(x, y, centerX, centerY, outerRadius))
            {
                sum += Img.at<uchar>(y, x);
                sumDist += Img.at<uchar>(y, x) * sqrt(pow(x - centerX, 2) + pow(y - centerY, 2.0));
            }
        }
    }

    sum = sum;
    double hfd;
    if (sum > 0)
    {
        hfd = 2.0 * sumDist / sum;
    }
    else
    {
        hfd = sqrt(2.0) * outerRadius;
    }

    return hfd;
}

Mat DebayerStarCountHfr(Mat img, bool do_star_mark = false)
{
    int histSize = 255;

    Mat gray;
    if (img.channels() == 3)
    {
        cvtColor(img, gray, COLOR_BGR2GRAY);
    }
    else
    {
        gray = img;
        cvtColor(img, img, COLOR_GRAY2BGR);
    }

    Mat gray_hist;
    calcHist(&gray, 1, 0, Mat(), gray_hist, 1, &histSize, 0, true);

    Mat blur;
    medianBlur(gray, blur, 1);

    Mat nonzero_indices;
    findNonZero(gray_hist, nonzero_indices);

    int nonezero_len = nonzero_indices.rows;
    int thres = static_cast<int>(nonzero_indices.at<Point>(nonezero_len * 0.5).x);

    Mat thresh;
    threshold(blur, thresh, thres, 255, THRESH_BINARY);

    std::vector<std::vector<Point>> contours;
    std::vector<Vec4i> hierarchy;
    findContours(thresh, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    int StarCount = 0;
    std::vector<double> HfrList;
    for (size_t i = 0; i < contours.size(); i++)
    {
        double area = contourArea(contours[i]);
        if (area >= 3)
        {
            StarCount++;
            Point2f center;
            float radius;
            minEnclosingCircle(contours[i], center, radius);
            int x = static_cast<int>(center.x);
            int y = static_cast<int>(center.y);
            Mat roi = gray(Rect(x - static_cast<int>(radius) - 5, y - static_cast<int>(radius) - 5,
                                static_cast<int>(radius) * 2 + 10, static_cast<int>(radius) * 2 + 10));
            double hfr = calcHfd(roi, 60) / 2;
            HfrList.push_back(hfr);
            if (do_star_mark)
            {
                circle(img, center, static_cast<int>(radius) + 10, Scalar(0, 255, 0), 1);
                putText(img, std::to_string(round(hfr * 100) / 100), center, FONT_HERSHEY_SIMPLEX, 1.0, Scalar(0, 255, 0), 1, LINE_AA);
            }
        }
    }

    double AvgHfr = (HfrList.empty()) ? 0 : std::accumulate(HfrList.begin(), HfrList.end(), 0.0) / HfrList.size();

    return img;
}

int main()
{
    std::string filepath = "path_to_image";
    int Thres = 128;

    Mat img = imread(filepath);

    // Example usage of the functions
    std::vector<Mat> bgr_planes(3);
    std::vector<Mat> hist(3);
    CalHist(img, bgr_planes, hist);

    Mat gray_hist = CalGrayHist(img);

    Mat dst = Stretch_WhiteBalance(hist, bgr_planes);

    Mat result = DebayerStarCount(img, Thres);

    Mat result_hfr = DebayerStarCountHfr(img, true);

    return 0;
}

Mat Cal_MTF(int m, const Mat &img)
{
    Mat mtf_img = img.clone();
    Mat zeros = (img == 0);
    Mat halfs = (img == m);
    Mat ones = (img == 1);
    Mat others = ~(zeros | halfs | ones);
    mtf_img.setTo((m - 1) * mtf_img / ((((2 * m) - 1) * mtf_img) - m), others);
    return mtf_img;
}

Mat GrayStretch(const Mat &img)
{
    double black_clip = -1.25;
    double target_bkg = 0.1;
    Mat norm_img;
    normalize(img, norm_img, 0, 1, NORM_MINMAX);
    double median = median(norm_img)[0];
    double avgbias = 0.0;

    int size = norm_img.rows * norm_img.cols;
    for (int i = 0; i < norm_img.rows; i++)
    {
        for (int j = 0; j < norm_img.cols; j++)
        {
            avgbias += std::abs(norm_img.at<double>(i, j) - median);
        }
    }
    avgbias /= size;

    double c0 = std::min(std::max(median + (black_clip * avgbias), 0.0), 1.0);
    double m = (median - c0) * (target_bkg - 1) / ((((2 * target_bkg) - 1) * (median - c0)) - target_bkg);
    double c1 = 1.0;

    for (int i = 0; i < norm_img.rows; i++)
    {
        for (int j = 0; j < norm_img.cols; j++)
        {
            if (norm_img.at<double>(i, j) < c0)
            {
                norm_img.at<double>(i, j) = 0;
            }
            else
            {
                norm_img.at<double>(i, j) = (norm_img.at<double>(i, j) - c0) / (1 - c0);
                norm_img.at<double>(i, j) = MTF(m, norm_img.at<double>(i, j));
            }
        }
    }

    Mat dst_img;
    normalize(norm_img, dst_img, 0, 65535, NORM_MINMAX);
    dst_img.convertTo(dst_img, CV_16U);
    return dst_img;
}

double CalScale(const Mat &img, int resize_size = 1080)
{
    int width = img.cols;
    int height = img.rows;
    double scale;

    if (width > height)
    {
        scale = resize_size / static_cast<double>(width);
    }
    else
    {
        scale = resize_size / static_cast<double>(height);
    }

    return scale;
}

double Cal_Middev(double mid, const Mat &img)
{
    Mat mid_dev;
    absdiff(img, mid, mid_dev);
    double median_deviation = median(mid_dev)[0];
    return median_deviation;
}

Mat AutoParamCompute_OneChannel(const Mat &img)
{
    double maxNum = 255.0;
    Mat norm_img;
    normalize(img, norm_img, 0, 1, NORM_MINMAX);
    double median = median(norm_img)[0];

    double med_dev = Cal_Middev(median * maxNum, img);
    double MADN = 1.4826 * (med_dev / maxNum);

    double B = 0.25;
    double C = -2.8;
    bool upper_half = (median / maxNum) > 0.5;
    double shadows = 0.0;
    double highlights = 1.0;

    if (upper_half && MADN != 0)
    {
        shadows = std::min(1.0, std::max(0.0, (median / maxNum) + C * MADN));
    }
    else if (!upper_half && MADN != 0)
    {
        highlights = std::min(1.0, std::max(0.0, (median / maxNum) - C * MADN));
    }

    double X, M, midtones;
    if (!upper_half)
    {
        X = (median / maxNum) - shadows;
        M = B;
    }
    else
    {
        X = B;
        M = highlights - (median / maxNum);
    }

    if (X == 0)
    {
        midtones = 0.0;
    }
    else if (X == M)
    {
        midtones = 0.5;
    }
    else if (X == 1)
    {
        midtones = 1.0;
    }
    else
    {
        midtones = ((M - 1) * X) / (((2 * M - 1) * X) - M);
    }

    return norm_img, shadows, midtones, highlights;
}

Mat Stretch_OneChannel(const Mat &norm_img, double shadows, double midtones, double highlights)
{
    double hsRangeFactor = (highlights == shadows) ? 1.0 : 1.0 / (highlights - shadows);
    double k1 = (midtones - 1) * hsRangeFactor;
    double k2 = ((2 * midtones) - 1) * hsRangeFactor;

    Mat stretched_img = norm_img.clone();
    Mat downshadow = (norm_img < shadows);
    stretched_img.setTo(0, downshadow);

    Mat uphighlight = (norm_img > highlights);
    stretched_img.setTo(1, uphighlight);

    Mat others = ~(downshadow | uphighlight);
    stretched_img.setTo((stretched_img - shadows) * k1 / (((stretched_img - shadows) * k2) - midtones), others);

    return stretched_img;
}

Mat ComputeStretch_ThreeChannels(const Mat &img, bool do_jpg)
{
    std::vector<Mat> bgr_planes;
    split(img, bgr_planes);

    Mat dst_img(img.rows, img.cols, CV_8UC3);
    Mat norm_img, shadows, midtones, highlights;

    std::tie(norm_img, shadows, midtones, highlights) = AutoParamCompute_OneChannel(bgr_planes[0]);
    Mat plane0 = Stretch_OneChannel(norm_img, shadows, midtones, highlights);

    std::tie(norm_img, shadows, midtones, highlights) = AutoParamCompute_OneChannel(bgr_planes[1]);
    Mat plane1 = Stretch_OneChannel(norm_img, shadows, midtones, highlights);

    std::tie(norm_img, shadows, midtones, highlights) = AutoParamCompute_OneChannel(bgr_planes[2]);
    Mat plane2 = Stretch_OneChannel(norm_img, shadows, midtones, highlights);

    if (do_jpg)
    {
        plane0.convertTo(dst_img.col(0), CV_8U, 255);
        plane1.convertTo(dst_img.col(1), CV_8U, 255);
        plane2.convertTo(dst_img.col(2), CV_8U, 255);
    }
    else
    {
        plane0.convertTo(dst_img.col(0), CV_16U, 65535);
        plane1.convertTo(dst_img.col(1), CV_16U, 65535);
        plane2.convertTo(dst_img.col(2), CV_16U, 65535);
    }

    return dst_img;
}
