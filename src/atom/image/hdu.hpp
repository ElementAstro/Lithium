#pragma once

#include <fstream>
#include <memory>
#include <vector>
#include "fits_data.hpp"
#include "fits_header.hpp"

class HDU {
public:
    virtual ~HDU() = default;
    virtual void readHDU(std::ifstream& file) = 0;
    virtual void writeHDU(std::ofstream& file) const = 0;

    const FITSHeader& getHeader() const { return header; }
    FITSHeader& getHeader() { return header; }

    void setHeaderKeyword(const std::string& keyword, const std::string& value);
    std::string getHeaderKeyword(const std::string& keyword) const;

protected:
    FITSHeader header;
    std::unique_ptr<FITSData> data;
};

class ImageHDU : public HDU {
public:
    void readHDU(std::ifstream& file) override;
    void writeHDU(std::ofstream& file) const override;

    void setImageSize(int w, int h, int c = 1);
    std::tuple<int, int, int> getImageSize() const;

    template <typename T>
    void setPixel(int x, int y, T value, int channel = 0);

    template <typename T>
    T getPixel(int x, int y, int channel = 0) const;

    template <typename T>
    struct ImageStats {
        T min;
        T max;
        double mean;
        double stddev;
    };

    template <typename T>
    ImageStats<T> computeImageStats(int channel = 0) const;

    template <typename T>
    void applyFilter(const std::vector<std::vector<double>>& kernel,
                     int channel = -1);

    // New methods for color image support
    bool isColor() const { return channels > 1; }
    int getChannelCount() const { return channels; }

private:
    int width = 0;
    int height = 0;
    int channels = 1;

    template <typename T>
    void initializeData();
};

