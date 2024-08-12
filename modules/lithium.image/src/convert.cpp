#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <uuid/uuid.h>

#include "fitsio.hpp"

#include <libstellarsolver/stellarsolver.h>

void saveFitsAsJPG(const std::string& filename) {
    cv::Mat image;
    cv::Mat image16;
    cv::Mat SendImage;
    readFits(filename.c_str(), image);

    std::vector<FITSImage::Star> stars = Tools::FindStarsByStellarSolver(true, true);

    if (stars.size() != 0) {
        FWHM = stars[0].HFR;
    } else {
        FWHM = -1;
    }

    if (image16.depth() == 8)
        image.convertTo(image16, CV_16UC1, 256, 0);  // x256  MSB alignment
    else
        image.convertTo(image16, CV_16UC1, 1, 0);

    if (FWHM != -1) {
        // Draw detection results on the original image
        cv::Point center(stars[0].x, stars[0].y);
        cv::circle(image16, center, static_cast<int>(FWHM), cv::Scalar(0, 0, 255), 1);  // Draw HFR circle
        cv::circle(image16, center, 1, cv::Scalar(0, 255, 0), -1);  // Draw center point
        // Display HFR value on the image
        std::string hfrText = cv::format("%.2f", stars[0].HFR);
        cv::putText(image16, hfrText, cv::Point(stars[0].x - FWHM, stars[0].y - FWHM - 5), cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(0, 0, 255), 1);
    }

    cv::Mat NewImage = image16;

    FWHMCalOver = true;

    // Scale the image to the range of 0-255
    cv::normalize(NewImage, SendImage, 0, 255, cv::NORM_MINMAX, CV_8U);

    // Generate a unique ID
    uuid_t uuid;
    uuid_generate(uuid);
    char uniqueId[37];
    uuid_unparse(uuid, uniqueId);

    // List all files with "CaptureImage" prefix
    std::filesystem::path directory(vueDirectoryPath);
    std::vector<std::filesystem::path> fileList;
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        if (entry.path().extension() == ".jpg" && entry.path().stem().string().find("CaptureImage") == 0) {
            fileList.push_back(entry.path());
        }
    }

    // Remove all matching files
    for (const auto& filePath : fileList) {
        std::filesystem::remove(filePath);
    }

    // Remove the previous image file
    if (PriorROIImage != "NULL") {
        std::filesystem::remove(PriorROIImage);
    }

    // Save the new image with a unique ID in the filename
    std::string fileName = "CaptureImage_" + std::string(uniqueId) + ".jpg";
    std::string filePath = vueDirectoryPath + fileName;

    bool saved = cv::imwrite(filePath, SendImage);

    std::string Command = "ln -sf " + filePath + " " + vueImagePath + fileName;
    system(Command.c_str());

    PriorROIImage = vueImagePath + fileName;

    if (saved) {
        // emit wsThread->sendMessageToClient("SaveJpgSuccess:" + QString::fromStdString(fileName));

        if (FWHM != -1) {
            dataPoints.emplace_back(CurrentPosition, FWHM);

            qDebug() << "dataPoints:" << CurrentPosition << "," << FWHM;

            float a, b, c;
            Tools::fitQuadraticCurve(dataPoints, a, b, c);

            if (dataPoints.size() >= 5) {
                std::vector<QPointF> LineData;

                for (float x = CurrentPosition - 3000; x <= CurrentPosition + 3000; x += 10) {
                    float y = a * x * x + b * x + c;
                    LineData.emplace_back(x, y);
                }

                // Calculate the x-coordinate where the derivative is zero
                float x_min = -b / (2 * a);
                minPoint_X = x_min;
                // Calculate the y-coordinate of the minimum point
                float y_min = a * x_min * x_min + b * x_min + c;

                std::ostringstream dataString;
                for (const auto& point : LineData) {
                    dataString << point.x() << "|" << point.y() << ":";
                }

                R2 = Tools::calculateRSquared(dataPoints, a, b, c);
                qDebug() << "RSquared: " << R2;

                // emit wsThread->sendMessageToClient("fitQuadraticCurve:" + QString::fromStdString(dataString.str()));
                // emit wsThread->sendMessageToClient("fitQuadraticCurve_minPoint:" + QString::number(x_min) + ":" + QString::number(y_min));
            }
        }
    } else {
        qDebug() << "Save Image Failed...";
    }
}

int saveFitsAsPNG(const std::string& fitsFileName) {
    CaptureTestTimer.start();
    qDebug() << "\033[32m" << "Save image data start." << "\033[0m";

    cv::Mat image;
    int status = Tools::readFits(fitsFileName.c_str(), image);

    // std::vector<FITSImage::Star> stars = Tools::FindStarsByStellarSolver(false, true);

    if (status != 0) {
        qDebug() << "Failed to read FITS file: " << fitsFileName;
        return status;
    }

    int width = image.cols;
    int height = image.rows;

    qDebug() << "image size:" << width << "," << height;
    qDebug() << "image depth:" << image.depth();
    qDebug() << "image channels:" << image.channels();

    std::vector<unsigned char> imageData;
    imageData.assign(image.data, image.data + image.total() * image.channels() * 2);
    qDebug() << "imageData Size:" << imageData.size() << "," << image.data + image.total() * image.channels();

    // Generate a unique ID
    uuid_t uuid;
    uuid_generate(uuid);
    char uniqueId[37];
    uuid_unparse(uuid, uniqueId);

    // List all files with "CaptureImage" prefix
    std::filesystem::path directory(vueDirectoryPath);
    std::vector<std::filesystem::path> fileList;
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        if (entry.path().extension() == ".bin" && entry.path().stem().string().find("CaptureImage") == 0) {
            fileList.push_back(entry.path());
        }
    }

    // Remove all matching files
    for (const auto& filePath : fileList) {
        std::filesystem::remove(filePath);
    }

    // Remove the previous image file
    if (PriorCaptureImage != "NULL") {
        std::filesystem::remove(PriorCaptureImage);
    }

    std::string fileName_ = "CaptureImage_" + std::string(uniqueId) + ".bin";
    std::string filePath_ = vueDirectoryPath + fileName_;

    std::ofstream outFile(filePath_, std::ios::binary);
    if (!outFile) {
        throw std::runtime_error("Failed to open file for writing.");
    }

    outFile.write(reinterpret_cast<const char*>(imageData.data()), imageData.size());
    if (!outFile) {
        throw std::runtime_error("Failed to write data to file.");
    }

    outFile.close();
    if (!outFile) {
        throw std::runtime_error("Failed to close the file properly.");
    }

    CaptureTestTime = CaptureTestTimer.elapsed();
    qDebug() << "\033[32m" << "Save image Data completed:" << CaptureTestTime << "milliseconds" << "\033[0m";
    CaptureTestTimer.invalidate();

    std::string Command = "ln -sf " + filePath_ + " " + vueImagePath + fileName_;
    system(Command.c_str());

    PriorCaptureImage = vueImagePath + fileName_;

    // emit wsThread->sendMessageToClient("SaveBinSuccess:" + QString::fromStdString(fileName_));
    isStagingImage = true;
    SavedImage = QString::fromStdString(fileName_);

    std::vector<FITSImage::Star> stars = Tools::FindStarsByStellarSolver(false, true);

    std::ostringstream dataString;
    for (const auto& star : stars) {
                dataString << star.x << "|" << star.y << "|" << star.HFR << ":";
    }
    // emit wsThread->sendMessageToClient("DetectedStars:" + QString::fromStdString(dataString.str()));

    return 0;
}
