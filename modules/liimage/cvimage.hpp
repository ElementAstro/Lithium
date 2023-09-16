#pragma once

#include <opencv2/opencv.hpp>

using namespace cv;

void CalHist(Mat img, std::vector<Mat> &bgr_planes, std::vector<Mat> &hist);
Mat CalGrayHist(Mat img);
Mat Stretch_WhiteBalance(std::vector<Mat> hists, std::vector<Mat> bgr_planes);
Mat StretchGray(Mat hist, Mat plane);
Mat DebayerStarCount(Mat img, int Thres);
bool insideCircle(int x, int y, int centerX, int centerY, float radius);
double calcHfd(Mat inImage, int inOuterDiameter);
Mat DebayerStarCountHfr(Mat img, bool do_star_mark);
Mat Cal_MTF(int m, const Mat &img);
Mat GrayStretch(const Mat &img);
double CalScale(const Mat &img, int resize_size = 1080);
double Cal_Middev(double mid, const Mat &img);
Mat AutoParamCompute_OneChannel(const Mat &img);
Mat Stretch_OneChannel(const Mat &norm_img, double shadows, double midtones, double highlights);
Mat ComputeStretch_ThreeChannels(const Mat &img, bool do_jpg);