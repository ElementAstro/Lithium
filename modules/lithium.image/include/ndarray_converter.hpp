// NDArrayConverter.hpp
#ifndef NDARRAY_CONVERTER_HPP
#define NDARRAY_CONVERTER_HPP

#include <Python.h>
#include <pybind11/pybind11.h>
#include <memory>
#include <opencv2/core.hpp>
#include <type_traits>
#include <vector>

class NDArrayConverter {
public:
    // 初始化Numpy
    static bool init_numpy();

    // 转换PyObject到cv::Mat
    static bool toMat(PyObject* o, cv::Mat& m);

    // 转换cv::Mat到PyObject
    static PyObject* toNDArray(const cv::Mat& mat);

    // 额外功能：转换std::vector<cv::Mat>到PyObject列表
    static PyObject* toNDArrayList(const std::vector<cv::Mat>& mats);

    // 额外功能：从PyObject列表转换到std::vector<cv::Mat>
    static bool toMatList(PyObject* o, std::vector<cv::Mat>& mats);
};

namespace pybind11 {
namespace detail {

template <>
struct type_caster<cv::Mat> {
public:
    PYBIND11_TYPE_CASTER(cv::Mat, _("numpy.ndarray"));

    bool load(handle src, bool /* convert */) {
        return NDArrayConverter::toMat(src.ptr(), value);
    }

    static handle cast(const cv::Mat& m, return_value_policy, handle defval) {
        return {NDArrayConverter::toNDArray(m)};
    }
};

}  // namespace detail
}  // namespace pybind11

#endif  // NDARRAY_CONVERTER_HPP