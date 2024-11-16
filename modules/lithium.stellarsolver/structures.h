#pragma once

#include <pybind11/pybind11.h>

#include <libstellarsolver/stellarsolver.h>

namespace py = pybind11;

class FitsImageStatistic {
public:
    FitsImageStatistic();
    ~FitsImageStatistic() = default;

    [[nodiscard]] auto getMin() const -> py::list;
    void setMin(const py::object& value);

    [[nodiscard]] auto getMax() const -> py::list;
    void setMax(const py::object& value);

    [[nodiscard]] auto getMean() const -> py::list;
    void setMean(const py::object& value);

    [[nodiscard]] auto getMedian() const -> py::list;
    void setMedian(const py::object& value);

    [[nodiscard]] auto getSnr() const -> double;
    void setSnr(double value);

    [[nodiscard]] auto getDataType() const -> uint32_t;
    void setDataType(uint32_t value);

    [[nodiscard]] auto getSamplesPerChannel() const -> uint32_t;
    void setSamplesPerChannel(uint32_t value);

    [[nodiscard]] auto getWidth() const -> uint16_t;
    void setWidth(uint16_t value);

    [[nodiscard]] auto getHeight() const -> uint16_t;
    void setHeight(uint16_t value);

    [[nodiscard]] auto getChannels() const -> uint8_t;
    void setChannels(uint8_t value);

    auto getStat() -> FITSImage::Statistic&;

private:
    void setDoubleArray(double* array, unsigned int length,
                        const py::object& value);
    FITSImage::Statistic stat_;
};