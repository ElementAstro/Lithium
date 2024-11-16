#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "ss.h"
#include "structures.h"

#include "atom/log/loguru.hpp"

namespace py = pybind11;

PYBIND11_MODULE(ssbindings, m) {
    LOG_F(INFO, "Initializing ssbindings module");

    py::class_<FitsImageStatistic>(m, "FitsImageStatistic")
        .def(py::init<>())
        .def_property("min", &FitsImageStatistic::getMin,
                      &FitsImageStatistic::setMin)
        .def_property("max", &FitsImageStatistic::getMax,
                      &FitsImageStatistic::setMax)
        .def_property("mean", &FitsImageStatistic::getMean,
                      &FitsImageStatistic::setMean)
        .def_property("median", &FitsImageStatistic::getMedian,
                      &FitsImageStatistic::setMedian)
        .def_property("snr", &FitsImageStatistic::getSnr,
                      &FitsImageStatistic::setSnr)
        .def_property("data_type", &FitsImageStatistic::getDataType,
                      &FitsImageStatistic::setDataType)
        .def_property("samples_per_channel",
                      &FitsImageStatistic::getSamplesPerChannel,
                      &FitsImageStatistic::setSamplesPerChannel)
        .def_property("width", &FitsImageStatistic::getWidth,
                      &FitsImageStatistic::setWidth)
        .def_property("height", &FitsImageStatistic::getHeight,
                      &FitsImageStatistic::setHeight)
        .def_property("channels", &FitsImageStatistic::getChannels,
                      &FitsImageStatistic::setChannels);

    py::class_<SS>(m, "SS")
        .def(py::init<const FITSImage::Statistic&, py::buffer, py::object>())
        .def("load_new_image_buffer", &SS::loadNewImageBuffer)
        .def("extract", &SS::extract)
        .def("solve", &SS::solve)
        .def("start", &SS::start)
        .def("abort", &SS::abort)
        .def("abort_and_wait", &SS::abortAndWait)
        .def("set_parameter_profile", &SS::setParameterProfile)
        .def("set_search_scale",
             py::overload_cast<double, double, const QString&>(
                 &SS::setSearchScale))
        .def("set_search_scale",
             py::overload_cast<double, double, SSolver::ScaleUnits>(
                 &SS::setSearchScale))
        .def("set_search_position_ra_dec", &SS::setSearchPositionRaDec)
        .def("set_search_position_in_degrees", &SS::setSearchPositionInDegrees)
        .def("set_use_subframe", &SS::setUseSubframe)
        .def("is_running", &SS::isRunning)
        .def_static("ra_string", &SS::raString)
        .def_static("dec_string", &SS::decString)
        .def("pixel_to_wcs", &SS::pixelToWCS)
        .def("wcs_to_pixel", &SS::wcsToPixel)

            LOG_F(INFO, "ssbindings module initialized successfully");
}