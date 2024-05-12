#include "hfr.hpp"
#include "utils.hpp"

#include <vector>
#include <cmath>
#include <algorithm>
#include <opencv2/imgproc.hpp>

double calcHfr(const cv::Mat& inImage, float radius) {
    cv::Mat img;
    inImage.convertTo(img, CV_32F);
    img -= cv::mean(img)[0];
    img = cv::max(img, 0.0f);

    int centerX = std::ceil(img.cols / 2.0);
    int centerY = std::ceil(img.rows / 2.0);

    double sum = 0.0, sumDist = 0.0;
    for (int i = 0; i < img.rows; ++i) {
        for (int j = 0; j < img.cols; ++j) {
            double dist = std::sqrt((i - centerX) * (i - centerX) + (j - centerY) * (j - centerY));
            if (dist <= radius * 1.2) {
                sum += img.at<float>(i, j);
                sumDist += img.at<float>(i, j) * dist;
            }
        }
    }

    return sum > 0 ? sumDist / sum : std::sqrt(2.0) * radius * 1.2;
}

using namespace std;
using namespace cv;

tuple<Mat, int, double, json> StarDetectAndHfr(const Mat& img, bool if_removehotpixel, bool if_noiseremoval, bool do_star_mark, bool down_sample_mean_std, Mat mark_img) {
    Mat grayimg, rgb_img;
    if (img.channels() == 3) {
        cvtColor(img, grayimg, COLOR_BGR2GRAY);
        rgb_img = img.clone();
    } else {
        grayimg = img;
        cvtColor(grayimg, rgb_img, COLOR_GRAY2BGR);
    }

    if (!mark_img.data) {
        mark_img = rgb_img.clone();
    } else if (mark_img.channels() == 1) {
        cvtColor(mark_img, mark_img, COLOR_GRAY2BGR);
    }

    Size img_shps = grayimg.size();
    Mat map = grayimg.clone();

    if (if_removehotpixel) {
        medianBlur(map, map, 3);
    }

    if (if_noiseremoval) {
        GaussianBlur(map, map, Size(3, 3), 1.0);
    }

    double median, std;
    if (!down_sample_mean_std) {
        median = mean(map)[0];
        Scalar mean, stddev;
        meanStdDev(map, mean, stddev);
        std = stddev[0];
    } else {
        vector<uchar> buffer_value;
        if (map.isContinuous()) {
            buffer_value.assign(map.datastart, map.dataend);
        } else {
            for (int i = 0; i < map.rows; ++i) {
                buffer_value.insert(buffer_value.end(), map.ptr<uchar>(i), map.ptr<uchar>(i) + map.cols);
            }
        }
        int maxSamples = 500000;
        int sampleBy = 1;
        if (map.rows * map.cols > maxSamples) {
            sampleBy = map.rows * map.cols / maxSamples;
        }

        vector<uchar> sample_value;
        for (size_t i = 0; i < buffer_value.size(); i += sampleBy) {
            sample_value.push_back(buffer_value[i]);
        }
        median = std::accumulate(sample_value.begin(), sample_value.end(), 0.0) / sample_value.size();
        double sum = 0;
        for (uchar val : sample_value) {
            sum += (val - median) * (val - median);
        }
        std = sqrt(sum / sample_value.size());
    }

    double threshold = median + 3 * std;
    Mat thres_map;
    cv::threshold(map, thres_map, threshold, 255, THRESH_BINARY);

    Mat closekernel = getStructuringElement(MORPH_RECT, Size(3, 3));
    morphologyEx(thres_map, thres_map, MORPH_OPEN, closekernel);

    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(thres_map, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE);

    double stand_size = 1552;
    double sclsize = max(img_shps.width, img_shps.height);
    double maximun_area = 1500 * (sclsize / stand_size);
    double minimun_area = max(1.0, ceil(sclsize / stand_size));
    double bsh_scale = sclsize / 2048;
    vector<double> HfrList;
    vector<double> arelist;
    int starnum = 0;

    for (size_t i = 0; i < contours.size(); i++) {
        double area = contourArea(contours[i]);
        if (area >= minimun_area && area < maximun_area) {
            Point2f center;
            float radius;
            minEnclosingCircle(contours[i], center, radius);

            Rect boundingBox = boundingRect(contours[i]);
            Point rect_center(boundingBox.x + boundingBox.width / 2, boundingBox.y + boundingBox.height / 2);

            if (checkElongated(boundingBox.width, boundingBox.height)) {
                continue;
            }

            int bsh_num;
            vector<int> bsh_list;
            vector<double> bsh_thres_list;
            tie(bsh_num, bsh_list, bsh_thres_list) = define_narrow_radius(static_cast<int>(minimun_area), maximun_area, area, bsh_scale);

            bool bsh_check = false;
            for (int bsh_index = 0; bsh_index < bsh_num; bsh_index++) {
                int narrow_radius = bsh_list[bsh_index];
                double pixelthresh = bsh_thres_list[bsh_index];
                Rect expandedRect(boundingBox.x - 5, boundingBox.y - 5, boundingBox.width + 10, boundingBox.height + 10);

                if (expandedRect.x < 0 || expandedRect.y < 0 || expandedRect.x + expandedRect.width >= img.cols || expandedRect.y + expandedRect.height >= img.rows) {
                    continue;
                }

                Mat rect_thres_expand = thres_map(expandedRect);

                if (BresenHamCheckCircle(rect_thres_expand, radius - narrow_radius, pixelthresh, false)) {
                    bsh_check = true;
                    break;
                }
            }
            if (!bsh_check) {
                continue;
            }

            Rect starRegion(static_cast<int>(center.x - radius), static_cast<int>(center.y - radius), static_cast<int>(2 * radius), static_cast<int>(2 * radius));
            if (starRegion.x < 0 || starRegion.y < 0 || starRegion.x + starRegion.width >= img.cols || starRegion.y + starRegion.height >= img.rows) {
                continue;
            }

            Mat rect_expand = rgb_img(starRegion);

            if (caldim(rect_expand)) {
                continue;
            }

            double hfr = calcHfr(grayimg(starRegion), radius);
            if (hfr < 0.05) {
                continue;
            }
            HfrList.push_back(hfr);
            starnum++;
            arelist.push_back(area);

            if (do_star_mark) {
                circle(mark_img, rect_center, static_cast<int>(radius) + 5, Scalar(0, 255, 0), 1);
                putText(mark_img, to_string(hfr), rect_center, FONT_HERSHEY_SIMPLEX, 1.0, Scalar(0, 255, 0), 1, LINE_AA);
            }
        }
    }

    double avghfr = HfrList.empty() ? 0 : accumulate(HfrList.begin(), HfrList.end(), 0.0) / HfrList.size();
    double maxarea = arelist.empty() ? -1 : *max_element(arelist.begin(), arelist.end());
    double minarea = arelist.empty() ? -1 : *min_element(arelist.begin(), arelist.end());
    double avgarea = arelist.empty() ? -1 : accumulate(arelist.begin(), arelist.end(), 0.0) / arelist.size();

   // Prepare the result as JSON
   json result = {
       {"max", maxarea},
       {"min", minarea},
       {"average", avgarea}
   };

   return make_tuple(mark_img, starnum, avghfr, result);
}