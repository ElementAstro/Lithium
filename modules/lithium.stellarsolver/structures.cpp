#include "structures.h"

FitsImageStatistic::FitsImageStatistic() {
    stat_.dataType = 20;
    stat_.bytesPerPixel = sizeof(int16_t);
}

py::list FitsImageStatistic::getMin() const {
    py::list res;
    for (unsigned int i = 0; i <= 2; i++) {
        res.append(stat_.min[i]);
    }
    return res;
}

void FitsImageStatistic::setMin(const py::object& value) {
    setDoubleArray(stat_.min, 3, value);
}

py::list FitsImageStatistic::getMax() const {
    py::list res;
    for (unsigned int i = 0; i <= 2; i++) {
        res.append(stat_.max[i]);
    }
    return res;
}

void FitsImageStatistic::setMax(const py::object& value) {
    setDoubleArray(stat_.max, 3, value);
}

py::list FitsImageStatistic::getMean() const {
    py::list res;
    for (unsigned int i = 0; i <= 2; i++) {
        res.append(stat_.mean[i]);
    }
    return res;
}

void FitsImageStatistic::setMean(const py::object& value) {
    setDoubleArray(stat_.mean, 3, value);
}

py::list FitsImageStatistic::getMedian() const {
    py::list res;
    for (unsigned int i = 0; i <= 2; i++) {
        res.append(stat_.median[i]);
    }
    return res;
}

void FitsImageStatistic::setMedian(const py::object& value) {
    setDoubleArray(stat_.median, 3, value);
}

double FitsImageStatistic::getSnr() const { return stat_.SNR; }

void FitsImageStatistic::setSnr(double value) { stat_.SNR = value; }

uint32_t FitsImageStatistic::getDataType() const { return stat_.dataType; }

void FitsImageStatistic::setDataType(uint32_t value) { stat_.dataType = value; }

uint32_t FitsImageStatistic::getSamplesPerChannel() const {
    return stat_.samples_per_channel;
}

void FitsImageStatistic::setSamplesPerChannel(uint32_t value) {
    stat_.samples_per_channel = value;
}

uint16_t FitsImageStatistic::getWidth() const { return stat_.width; }

void FitsImageStatistic::setWidth(uint16_t value) { stat_.width = value; }

uint16_t FitsImageStatistic::getHeight() const { return stat_.height; }

void FitsImageStatistic::setHeight(uint16_t value) { stat_.height = value; }

uint8_t FitsImageStatistic::getChannels() const { return stat_.channels; }

void FitsImageStatistic::setChannels(uint8_t value) { stat_.channels = value; }

FITSImage::Statistic& FitsImageStatistic::getStat() { return stat_; }

void FitsImageStatistic::setDoubleArray(double* array, unsigned int length,
                                        const py::object& value) {
    if (py::isinstance<py::float_>(value) || py::isinstance<py::int_>(value)) {
        auto val = value.cast<double>();
        for (unsigned int i = 0; i < length; i++) {
            array[i] = val;
        }
    } else if (py::isinstance<py::sequence>(value)) {
        auto seq = value.cast<py::sequence>();
        for (unsigned int i = 0; i < length && i < py::len(seq); i++) {
            array[i] = seq[i].cast<double>();
        }
    }
}