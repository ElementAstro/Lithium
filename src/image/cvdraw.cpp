#include <iostream>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <spdlog/spdlog.h>

class IndiAllskyDetectLines
{
private:
    int canny_low_threshold = 15;
    int canny_high_threshold = 50;
    int blur_kernel_size = 5;
    float rho = 1;
    float theta = CV_PI / 180;
    int threshold = 125;
    int min_line_length = 40;
    int max_line_gap = 20;
    int mask_blur_kernel_size = 75;
    cv::Mat _sqm_mask;
    cv::Mat _sqm_gradient_mask;

public:
    IndiAllskyDetectLines(const int bin_v, const cv::Mat &mask = nullptr)
    {
        /* 构造函数 */
        this->bin_v = bin_v;
        this->_sqm_mask = mask;
    }

    std::vector<cv::Vec4i> detectLines(cv::Mat original_img, const std::vector<int> &sqm_roi)
    {
        this->_generateSqmMask(original_img, sqm_roi);

        if (this->_sqm_gradient_mask.empty())
        {
            this->_generateSqmGradientMask(original_img);
        }

        cv::Mat masked_img;
        cv::multiply(original_img, this->_sqm_gradient_mask, masked_img);

        cv::Mat img_gray;
        if (original_img.channels() == 1)
        {
            img_gray = masked_img;
        }
        else
        {
            cv::cvtColor(masked_img, img_gray, cv::COLOR_BGR2GRAY);
        }

        double lines_start = cv::getTickCount();

        cv::Mat blur_gray;
        cv::GaussianBlur(img_gray, blur_gray, cv::Size(this->blur_kernel_size, this->blur_kernel_size), cv::BORDER_DEFAULT);

        cv::Mat edges;
        cv::Canny(blur_gray, edges, this->canny_low_threshold, this->canny_high_threshold);

        std::vector<cv::Vec4i> lines;
        cv::HoughLinesP(edges, lines, this->rho, this->theta, this->threshold, this->min_line_length, this->max_line_gap);

        double lines_elapsed_s = (cv::getTickCount() - lines_start) / cv::getTickFrequency();

        spdlog::info("Line detection in {} s", lines_elapsed_s);

        if (lines.empty())
        {
            spdlog::info("Detected 0 lines");
            return lines;
        }

        spdlog::info("Detected {} lines", lines.size());

        this->_drawLines(original_img, lines);

        return lines;
    }

private:
    void _generateSqmMask(const cv::Mat &img, const std::vector<int> &sqm_roi)
    {
        spdlog::info("Generating mask based on SQM_ROI");

        int image_height = img.rows;
        int image_width = img.cols;

        cv::Mat mask = cv::Mat::zeros(image_height, image_width, CV_8UC1);

        cv::Point pt1, pt2;
        try
        {
            pt1.x = sqm_roi[0] / this->bin_v;
            pt1.y = sqm_roi[1] / this->bin_v;
            pt2.x = sqm_roi[2] / this->bin_v;
            pt2.y = sqm_roi[3] / this->bin_v;
        }
        catch (std::out_of_range &e)
        {
            spdlog::warn("Using central ROI for blob calculations");
            pt1.x = (image_width / 2) - (image_width / 3);
            pt1.y = (image_height / 2) - (image_height / 3);
            pt2.x = (image_width / 2) + (image_width / 3);
            pt2.y = (image_height / 2) + (image_height / 3);
        }

        cv::rectangle(mask, pt1, pt2, cv::Scalar(255), cv::FILLED);

        cv::Mat blur_mask;
        cv::blur(mask, blur_mask, cv::Size(this->mask_blur_kernel_size, this->mask_blur_kernel_size), cv::BORDER_DEFAULT);
        this->_sqm_mask = blur_mask;
    }

    void _generateSqmGradientMask(const cv::Mat &img)
    {
        int image_height = img.rows;
        int image_width = img.cols;

        if (!this->_sqm_mask.empty() && config.type() == CV_32FC1 && config.at<int>("IMAGE_STACK_COUNT", 1) > 1 && config.at<bool>("IMAGE_STACK_SPLIT"))
        {
            int half_width = image_width / 2;
            cv::line(this->_sqm_mask, cv::Point(half_width, 0), cv::Point(half_width, image_height), cv::Scalar(0), 71);
        }

        cv::Mat blur_mask;
        cv::blur(this->_sqm_mask, blur_mask, cv::Size(this->mask_blur_kernel_size, this->mask_blur_kernel_size), cv::BORDER_DEFAULT);

        cv::Mat mask;
        if (img.channels() == 1)
        {
            mask = blur_mask;
        }
        else
        {
            cv::cvtColor(blur_mask, mask, cv::COLOR_GRAY2BGR);
        }

        mask.convertTo(this->_sqm_gradient_mask, CV_32FC3, 1.0 / 255.0);
    }

    void _drawLines(cv::Mat img, const std::vector<cv::Vec4i> &lines)
    {
        cv::Scalar color_bgr(0, 255, 0);

        for (auto &line : lines)
        {
            int x1 = line[0];
            int y1 = line[1];
            int x2 = line[2];
            int y2 = line[3];

            cv::line(img, cv::Point(x1, y1), cv::Point(x2, y2), color_bgr, 3);
        }
    }

    int bin_v;
};

class IndiAllSkyStars
{
private:
    int _distanceThreshold = 10;
    double _detectionThreshold;
    cv::Mat _sqm_mask;
    cv::Mat _star_template;
    std::string _image_dir;
    std::shared_ptr<spdlog::logger> _logger;

public:
    IndiAllSkyStars(const double detect_threshold, const cv::Mat &mask) : _detectionThreshold(detect_threshold),
                                                                          _sqm_mask(mask)
    {
        // start with a black image
        _star_template = cv::Mat::zeros(15, 15, CV_8UC1);

        // draw a white circle
        cv::circle(
            _star_template,
            cv::Point(7, 7),
            3,
            cv::Scalar(255),
            cv::FILLED);

        // blur circle to simulate a star
        cv::blur(_star_template, _star_template, cv::Size(2, 2));

        cv::flip(_star_template, _star_template, 0);

        spdlog::set_level(spdlog::level::info);
    }

    std::vector<cv::Point> detectObjects(cv::Mat &original_data, const int bin_v)
    {
        if (_sqm_mask.empty())
        {
            // This only needs to be done once if a mask is not provided
            _generateSqmMask(original_data, bin_v);
        }

        cv::Mat masked_img;
        cv::bitwise_and(original_data, original_data, masked_img, _sqm_mask);

        cv::Mat grey_img;
        cv::cvtColor(masked_img, grey_img, cv::COLOR_BGR2GRAY);

        auto sep_start = std::chrono::steady_clock::now();

        cv::Mat result;
        cv::matchTemplate(grey_img, _star_template, result, cv::TM_CCOEFF_NORMED);

        std::vector<cv::Point> blobs;
        cv::threshold(result, result, _detectionThreshold, 1.0, cv::THRESH_BINARY);
        cv::findNonZero(result, blobs);

        auto sep_elapsed_s = std::chrono::duration_cast<std::chrono::milliseconds>(
                                 std::chrono::steady_clock::now() - sep_start)
                                 .count();
        spdlog::info("Star detection in {} ms", sep_elapsed_s);

        spdlog::info("Found {} objects", blobs.size());

        _drawCircles(original_data, blobs);

        return blobs;
    }

private:
    void _generateSqmMask(const cv::Mat &img, const int bin_v)
    {
        spdlog::info("Generating mask based on SQM_ROI");

        int image_height = img.rows;
        int image_width = img.cols;

        // create a black background
        cv::Mat mask(image_height, image_width, CV_8UC1, cv::Scalar(0));

        std::vector<int> sqm_roi;
        // 从config中读取SQM_ROI的部分改为手动输入参数
        int sqm_roi_array[4] = {0, 0, image_width, image_height};
        sqm_roi = std::vector<int>(sqm_roi_array, sqm_roi_array + 4);

        int x1, y1, x2, y2;
        if (sqm_roi.size() == 4)
        {
            x1 = static_cast<int>(sqm_roi[0] / bin_v);
            y1 = static_cast<int>(sqm_roi[1] / bin_v);
            x2 = static_cast<int>(sqm_roi[2] / bin_v);
            y2 = static_cast<int>(sqm_roi[3] / bin_v);
        }
        else
        {
            spdlog::warn("Using central ROI for star detection");
            x1 = static_cast<int>(image_width / 2 - image_width / 3);
            y1 = static_cast<int>(image_height / 2 - image_height / 3);
            x2 = static_cast<int>(image_width / 2 + image_width / 3);
            y2 = static_cast<int>(image_height / 2 + image_height / 3);
        }

        // The white area is what we keep
        cv::rectangle(
            mask,
            cv::Point(x1, y1),
            cv::Point(x2, y2),
            cv::Scalar(255),
            cv::FILLED);

        _sqm_mask = mask;
    }

    void _drawCircles(cv::Mat &sep_data, const std::vector<cv::Point> &blob_list)
    {
        int image_height = sep_data.rows;
        int image_width = sep_data.cols;

        cv::Scalar color_bgr(255, 255, 255);

        // 使用 set 来去重
        std::set<std::pair<int, int>> pos_set;

        spdlog::info("Draw circles around objects");
        for (const auto &blob : blob_list)
        {
            int x = blob.x;
            int y = blob.y;

            auto pos_pair = std::make_pair(x, y);
            if (pos_set.find(pos_pair) != pos_set.end())
            {
                continue; // 如果在 set 中已经存在，则跳过这个点
            }
            else
            {
                pos_set.insert(pos_pair); // 否则插入到 set 中
            }

            cv::Point center(x + (_star_template.cols / 2) + 1,
                             y + (_star_template.rows / 2) + 1);

            cv::circle(
                sep_data,
                center,
                6,
                color_bgr,
                1);
        }
        cv::imwrite("testt.jpg", sep_data);
    }
};

int main()
{
    double detect_threshold = 0.7;
    cv::Mat mask;  // 假设已经有对应的mask
    int bin_v = 1; // 假设binning为2
    IndiAllSkyStars star_detector(detect_threshold, mask);
    cv::Mat img = cv::imread("test.jpg");
    auto star_positions = star_detector.detectObjects(img, bin_v);

    IndiAllskyDetectLines allsky_detect_lines(1, nullptr);

    cv::Mat allsky_img = cv::imread("test.jpg");
    std::vector<int> sqm_roi = {100, 100, 400, 400};

    std::vector<cv::Vec4i> lines = allsky_detect_lines.detectLines(allsky_img, sqm_roi);
    return 0;
}