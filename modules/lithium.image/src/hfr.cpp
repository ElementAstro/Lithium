#include "hfr.hpp"
#include "imgutils.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <opencv2/imgproc.hpp>
#include <vector>

#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"

using json = nlohmann::json;
using namespace std;
using namespace cv;

auto calcHfr(const cv::Mat& inImage, float radius) -> double {
    try {
        LOG_F(INFO, "Calculating HFR with radius: {}", radius);
        cv::Mat img;
        inImage.convertTo(img, CV_32F);
        img -= cv::mean(img)[0];
        img = cv::max(img, 0.0F);

        int centerX = std::ceil(img.cols / 2.0);
        int centerY = std::ceil(img.rows / 2.0);

        double sum = 0.0;
        double sumDist = 0.0;
        for (int i = 0; i < img.rows; ++i) {
            for (int j = 0; j < img.cols; ++j) {
                double dist = std::sqrt((i - centerX) * (i - centerX) +
                                        (j - centerY) * (j - centerY));
                constexpr double K_MAGIC_NUMBER = 1.2;
                if (dist <= radius * K_MAGIC_NUMBER) {
                    sum += img.at<float>(i, j);
                    sumDist += img.at<float>(i, j) * dist;
                }
            }
        }

        if (sum <= 0) {
            LOG_F(WARNING, "Sum is non-positive, returning default HFR value.");
            return std::sqrt(2.0) * radius * 1.2;
        }

        double hfr = sumDist / sum;
        LOG_F(INFO, "Calculated HFR: {}", hfr);
        return hfr;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception in calcHfr: {}", e.what());
        throw;
    }
}

auto caldim(const cv::Mat& img) -> bool {
    try {
        LOG_F(INFO, "Performing caldim check.");
        cv::Mat gray;
        if (img.channels() == 3) {
            cvtColor(img, gray, COLOR_BGR2GRAY);
        } else {
            gray = img;
        }

        double minVal;
        double maxVal;
        minMaxLoc(gray, &minVal, &maxVal);

        double threshold = minVal + (maxVal - minVal) * 0.5;
        cv::Mat binary;
        cv::threshold(gray, binary, threshold, 255, THRESH_BINARY);

        int nonZeroCount = countNonZero(binary);
        double nonZeroRatio =
            static_cast<double>(nonZeroCount) / (binary.rows * binary.cols);

        LOG_F(INFO, "caldim check: non-zero ratio = %f", nonZeroRatio);

        constexpr double K_NON_ZERO_RATIO_THRESHOLD = 0.1;
        return nonZeroRatio < K_NON_ZERO_RATIO_THRESHOLD;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception in caldim: %s", e.what());
        throw;
    }
}

auto preprocessImage(const Mat& img, Mat& grayimg, Mat& rgbImg,
                     Mat& mark_img) -> void {
    if (img.channels() == 3) {
        cvtColor(img, grayimg, COLOR_BGR2GRAY);
        rgbImg = img.clone();
        LOG_F(INFO, "Converted BGR to grayscale.");
    } else {
        grayimg = img;
        cvtColor(grayimg, rgbImg, COLOR_GRAY2BGR);
        LOG_F(INFO, "Converted grayscale to RGB.");
    }

    if (mark_img.data == nullptr) {
        mark_img = rgbImg.clone();
        LOG_F(INFO, "Initialized mark_img with cloned RGB image.");
    } else if (mark_img.channels() == 1) {
        cvtColor(mark_img, mark_img, COLOR_GRAY2BGR);
        LOG_F(INFO, "Converted single-channel mark_img to BGR.");
    }
}

auto removeNoise(Mat& map, bool if_removehotpixel,
                 bool if_noiseremoval) -> void {
    if (if_removehotpixel) {
        LOG_F(INFO, "Removing hot pixels using median blur.");
        medianBlur(map, map, 3);
    }

    if (if_noiseremoval) {
        LOG_F(INFO, "Removing noise using Gaussian blur.");
        GaussianBlur(map, map, Size(3, 3), 1.0);
    }
}

auto calculateMeanAndStd(const Mat& map, bool down_sample_mean_std,
                         double& medianVal, double& stdDev) -> void {
    if (!down_sample_mean_std) {
        medianVal = mean(map)[0];
        Scalar meanVal;
        Scalar stddev;
        meanStdDev(map, meanVal, stddev);
        stdDev = stddev[0];
        LOG_F(INFO, "Calculated mean and std without downsampling.");
    } else {
        LOG_F(INFO, "Calculating mean and std with downsampling.");
        vector<uchar> bufferValue;
        if (map.isContinuous()) {
            bufferValue.assign(map.datastart, map.dataend);
        } else {
            for (int i = 0; i < map.rows; ++i) {
                bufferValue.insert(bufferValue.end(), map.ptr<uchar>(i),
                                   map.ptr<uchar>(i) + map.cols);
            }
        }
        constexpr int K_MAX_SAMPLES = 500000;
        int sampleBy = 1;
        if (map.rows * map.cols > K_MAX_SAMPLES) {
            sampleBy = map.rows * map.cols / K_MAX_SAMPLES;
            LOG_F(INFO, "Downsampling with step: {}", sampleBy);
        }

        vector<uchar> sampleValue;
        for (size_t i = 0; i < bufferValue.size(); i += sampleBy) {
            sampleValue.push_back(bufferValue[i]);
        }
        medianVal =
            std::accumulate(sampleValue.begin(), sampleValue.end(), 0.0) /
            sampleValue.size();
        double sum = 0;
        for (uchar val : sampleValue) {
            sum += (val - medianVal) * (val - medianVal);
        }
        stdDev = sqrt(sum / sampleValue.size());
        LOG_F(INFO, "Calculated downsampled mean: {} and std: {}", medianVal,
              stdDev);
    }
}

auto processContours(const Mat& grayimg, const Mat& rgbImg, Mat& mark_img,
                     const vector<vector<Point>>& contours, double threshold,
                     bool do_star_mark)
    -> tuple<int, double, vector<double>, vector<double>> {
    constexpr double K_STAND_SIZE = 1552;
    Size imgShps = grayimg.size();
    double sclsize = max(imgShps.width, imgShps.height);
    double maximunArea = 1500 * (sclsize / K_STAND_SIZE);
    double minimunArea = max(1.0, ceil(sclsize / K_STAND_SIZE));
    double bshScale = sclsize / 2048;
    vector<double> hfrList;
    vector<double> arelist;
    int starnum = 0;

    for (size_t i = 0; i < contours.size(); i++) {
        double area = contourArea(contours[i]);
        if (area >= minimunArea && area < maximunArea) {
            Point2f center;
            float radius;
            minEnclosingCircle(contours[i], center, radius);

            Rect boundingBox = boundingRect(contours[i]);
            Point rectCenter(boundingBox.x + boundingBox.width / 2,
                             boundingBox.y + boundingBox.height / 2);

            if (checkElongated(boundingBox.width, boundingBox.height)) {
                LOG_F(INFO, "Contour {} is elongated. Skipping.", i);
                continue;
            }

            int bshNum;
            vector<int> bshList;
            vector<double> bshThresList;
            tie(bshNum, bshList, bshThresList) = defineNarrowRadius(
                static_cast<int>(minimunArea), maximunArea, area, bshScale);

            bool bshCheck = false;
            for (int bshIndex = 0; bshIndex < bshNum; bshIndex++) {
                int narrowRadius = bshList[bshIndex];
                double pixelthresh = bshThresList[bshIndex];
                Rect expandedRect(boundingBox.x - 5, boundingBox.y - 5,
                                  boundingBox.width + 10,
                                  boundingBox.height + 10);

                if (expandedRect.x < 0 || expandedRect.y < 0 ||
                    expandedRect.x + expandedRect.width >= grayimg.cols ||
                    expandedRect.y + expandedRect.height >= grayimg.rows) {
                    LOG_F(
                        WARNING,
                        "Expanded rectangle out of bounds. Skipping contour {}",
                        i);
                    continue;
                }

                Mat rectThresExpand = grayimg(expandedRect);

                if (checkBresenhamCircle(rectThresExpand, radius - narrowRadius,
                                         pixelthresh, false)) {
                    bshCheck = true;
                    break;
                }
            }
            if (!bshCheck) {
                LOG_F(INFO, "Contour {} failed BresenHam check. Skipping.", i);
                continue;
            }

            Rect starRegion(static_cast<int>(center.x - radius),
                            static_cast<int>(center.y - radius),
                            static_cast<int>(2 * radius),
                            static_cast<int>(2 * radius));
            if (starRegion.x < 0 || starRegion.y < 0 ||
                starRegion.x + starRegion.width >= grayimg.cols ||
                starRegion.y + starRegion.height >= grayimg.rows) {
                LOG_F(WARNING,
                      "Star region out of bounds for contour {}. Skipping.", i);
                continue;
            }

            Mat rectExpand = rgbImg(starRegion);

            if (caldim(rectExpand)) {
                LOG_F(INFO, "Contour {} failed caldim check. Skipping.", i);
                continue;
            }

            double hfr = calcHfr(grayimg(starRegion), radius);
            constexpr double K_HFR_THRESHOLD = 0.05;
            if (hfr < K_HFR_THRESHOLD) {
                LOG_F(INFO, "HFR below threshold for contour {}. Skipping.", i);
                continue;
            }
            hfrList.push_back(hfr);
            starnum++;
            arelist.push_back(area);

            if (do_star_mark) {
                circle(mark_img, rectCenter, static_cast<int>(radius) + 5,
                       Scalar(0, 255, 0), 1);
                putText(mark_img, to_string(hfr), rectCenter,
                        FONT_HERSHEY_SIMPLEX, 1.0, Scalar(0, 255, 0), 1,
                        LINE_AA);
                LOG_F(INFO, "Marked star at contour {} with HFR: {}", i, hfr);
            }
        }
    }

    return make_tuple(
        starnum,
        accumulate(hfrList.begin(), hfrList.end(), 0.0) / hfrList.size(),
        hfrList, arelist);
}

auto starDetectAndHfr(const Mat& img, bool if_removehotpixel,
                      bool if_noiseremoval, bool do_star_mark,
                      bool down_sample_mean_std,
                      Mat mark_img) -> tuple<Mat, int, double, json> {
    try {
        LOG_F(INFO, "Starting StarDetectAndHfr processing.");
        Mat grayimg, rgbImg;
        preprocessImage(img, grayimg, rgbImg, mark_img);

        Mat map = grayimg.clone();
        removeNoise(map, if_removehotpixel, if_noiseremoval);

        double medianVal, stdDev;
        calculateMeanAndStd(map, down_sample_mean_std, medianVal, stdDev);

        double threshold = medianVal + 3 * stdDev;
        LOG_F(INFO, "Applying threshold: {}", threshold);
        Mat thresMap;
        cv::threshold(map, thresMap, threshold, 255, THRESH_BINARY);

        Mat closekernel = getStructuringElement(MORPH_RECT, Size(3, 3));
        morphologyEx(thresMap, thresMap, MORPH_OPEN, closekernel);
        LOG_F(INFO, "Performed morphological opening.");

        vector<vector<Point>> contours;
        vector<Vec4i> hierarchy;
        findContours(thresMap, contours, hierarchy, RETR_EXTERNAL,
                     CHAIN_APPROX_NONE);
        LOG_F(INFO, "Found {} contours.", contours.size());

        int starnum;
        double avghfr;
        vector<double> hfrList;
        vector<double> arelist;
        tie(starnum, avghfr, hfrList, arelist) = processContours(
            grayimg, rgbImg, mark_img, contours, threshold, do_star_mark);

        double maxarea =
            arelist.empty() ? -1 : *max_element(arelist.begin(), arelist.end());
        double minarea =
            arelist.empty() ? -1 : *min_element(arelist.begin(), arelist.end());
        double avgarea = arelist.empty()
                             ? -1
                             : accumulate(arelist.begin(), arelist.end(), 0.0) /
                                   arelist.size();

        LOG_F(INFO, "Processed {} stars.", starnum);
        LOG_F(INFO, "Average HFR: {}, Max Area: {}, Min Area: {}, Avg Area: {}",
              avghfr, maxarea, minarea, avgarea);

        // Prepare the result as JSON
        json result = {
            {"max", maxarea}, {"min", minarea}, {"average", avgarea}};

        return make_tuple(mark_img, starnum, avghfr, result);
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception in StarDetectAndHfr: {}", e.what());
        throw;
    }
}
