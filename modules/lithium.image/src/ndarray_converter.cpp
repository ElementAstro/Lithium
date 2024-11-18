// NDArrayConverter.cpp
#include "ndarray_converter.hpp"

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/ndarrayobject.h>
#include <pybind11/pybind11.h>
#include <opencv2/core.hpp>
#include <stdexcept>
#include <vector>

#if PY_VERSION_HEX >= 0x03000000
#define PyInt_Check PyLong_Check
#define PyInt_AsLong PyLong_AsLong
#endif

struct Tmp {
public:
    const char* name;
    explicit Tmp(const char* name) : name(name) {}
};

Tmp info("return value");

auto NDArrayConverter::init_numpy() -> bool {
    import_array1(false);
    return true;
}

static PyObject* opencv_error = nullptr;

static auto failmsg(const char* fmt, ...) -> int {
    constexpr size_t STR_SIZE = 1000;
    char str[STR_SIZE];
    va_list args;
    va_start(args, fmt);
    vsnprintf(str, STR_SIZE, fmt, args);
    va_end(args);
    PyErr_SetString(PyExc_TypeError, str);
    return 0;
}

class PyAllowThreads {
public:
    PyAllowThreads() : state_(PyEval_SaveThread()) {}
    ~PyAllowThreads() { PyEval_RestoreThread(state_); }

private:
    PyThreadState* state_;
};

class PyEnsureGIL {
public:
    PyEnsureGIL() : state_(PyGILState_Ensure()) {}
    ~PyEnsureGIL() { PyGILState_Release(state_); }

private:
    PyGILState_STATE state_;
};

#define ERRWRAP2(expr)                           \
    try {                                        \
        PyAllowThreads allowThreads;             \
        expr;                                    \
    } catch (const cv::Exception& e) {           \
        PyErr_SetString(opencv_error, e.what()); \
        return 0;                                \
    }

using namespace cv;

class NumpyAllocator : public MatAllocator {
public:
    NumpyAllocator() { stdAllocator = Mat::getStdAllocator(); }
    ~NumpyAllocator() override = default;

    auto allocate(PyObject* obj, int dims, const int* sizes, int type,
                  size_t* step) const -> UMatData* {
        auto* u = new UMatData(this);
        u->data = u->origdata = static_cast<uchar*>(
            PyArray_DATA(reinterpret_cast<PyArrayObject*>(obj)));
        auto* strides = PyArray_STRIDES(reinterpret_cast<PyArrayObject*>(obj));
        for (int i = 0; i < dims - 1; i++) {
            step[i] = static_cast<size_t>(strides[i]);
        }
        step[dims - 1] = CV_ELEM_SIZE(type);
        u->size = sizes[0] * step[0];
        u->userdata = obj;
        return u;
    }

#if CV_MAJOR_VERSION < 4
    auto allocate(int dims0, const int* sizes, int type, void* data,
                  size_t* step, int flags,
                  UMatUsageFlags usageFlags) const -> UMatData* override
#else
    auto allocate(int dims0, const int* sizes, int type, void* data,
                  size_t* step, AccessFlag flags,
                  UMatUsageFlags usageFlags) const -> UMatData* override
#endif
    {
        if (data != nullptr) {
            CV_Error(Error::StsAssert, "The data should normally be NULL!");
            return stdAllocator->allocate(dims0, sizes, type, data, step, flags,
                                          usageFlags);
        }
        PyEnsureGIL gil;
        int depth = CV_MAT_DEPTH(type);
        int cn = CV_MAT_CN(type);
        int typenum = 0;
        switch (depth) {
            case CV_8U:
                typenum = NPY_UBYTE;
                break;
            case CV_8S:
                typenum = NPY_BYTE;
                break;
            case CV_16U:
                typenum = NPY_USHORT;
                break;
            case CV_16S:
                typenum = NPY_SHORT;
                break;
            case CV_32S:
                typenum = NPY_INT;
                break;
            case CV_32F:
                typenum = NPY_FLOAT;
                break;
            case CV_64F:
                typenum = NPY_DOUBLE;
                break;
            default:
                CV_Error(Error::StsUnsupportedFormat, "Unsupported data type");
        }
        std::vector<npy_intp> sizes_vec(dims0 + (cn > 1 ? 1 : 0));
        for (int i = 0; i < dims0; i++) {
            sizes_vec[i] = sizes[i];
        }
        if (cn > 1) {
            sizes_vec[dims0] = cn;
        }

        PyObject* obj = PyArray_SimpleNew(dims0 + (cn > 1 ? 1 : 0),
                                          sizes_vec.data(), typenum);
        if (!obj) {
            CV_Error_(
                Error::StsError,
                ("Cannot create numpy array with type %d and %d dimensions",
                 typenum, dims0));
        }

        return allocate(obj, dims0, sizes, type, step);
    }

#if CV_MAJOR_VERSION < 4
    auto allocate(UMatData* u, int accessFlags,
                  UMatUsageFlags usageFlags) const -> bool override
#else
    auto allocate(UMatData* u, AccessFlag accessFlags,
                  UMatUsageFlags usageFlags) const -> bool override
#endif
    {
        return stdAllocator->allocate(u, accessFlags, usageFlags);
    }

    void deallocate(UMatData* u) const override {
        if (!u) {
            return;
        }
        PyEnsureGIL gil;
        if (u->refcount == 0) {
            PyObject* obj = reinterpret_cast<PyObject*>(u->userdata);
            Py_XDECREF(obj);
            delete u;
        }
    }

    const MatAllocator* stdAllocator;
};

static NumpyAllocator g_numpyAllocator;

auto NDArrayConverter::toMat(PyObject* obj, Mat& mat) -> bool {
    if (!obj || obj == Py_None) {
        if (!mat.data) {
            mat.allocator = &g_numpyAllocator;
        }
        return true;
    }

    if (PyInt_Check(obj)) {
        double values[] = {static_cast<double>(PyInt_AsLong(obj)), 0., 0., 0.};
        mat = Mat(4, 1, CV_64F, values).clone();
        return true;
    }
    if (PyFloat_Check(obj)) {
        double values[] = {PyFloat_AsDouble(obj), 0., 0., 0.};
        mat = Mat(4, 1, CV_64F, values).clone();
        return true;
    }
    if (PyTuple_Check(obj)) {
        int size = static_cast<int>(PyTuple_Size(obj));
        mat = Mat(size, 1, CV_64F);
        for (int i = 0; i < size; i++) {
            PyObject* item = PyTuple_GetItem(obj, i);
            if (PyInt_Check(item)) {
                mat.at<double>(i) = static_cast<double>(PyInt_AsLong(item));
            } else if (PyFloat_Check(item)) {
                mat.at<double>(i) = PyFloat_AsDouble(item);
            } else {
                failmsg("%s is not a numerical tuple", info.name);
                mat.release();
                return false;
            }
        }
        return true;
    }

    if (!PyArray_Check(obj)) {
        failmsg("%s is not a numpy array, neither a scalar", info.name);
        return false;
    }

    PyArrayObject* arr = reinterpret_cast<PyArrayObject*>(obj);
    int typenum = PyArray_TYPE(arr);
    int type = 0;
    switch (typenum) {
        case NPY_UBYTE:
            type = CV_8U;
            break;
        case NPY_BYTE:
            type = CV_8S;
            break;
        case NPY_USHORT:
            type = CV_16U;
            break;
        case NPY_SHORT:
            type = CV_16S;
            break;
        case NPY_INT:
            type = CV_32S;
            break;
        case NPY_FLOAT:
            type = CV_32F;
            break;
        case NPY_DOUBLE:
            type = CV_64F;
            break;
        default:
            failmsg("%s data type = %d is not supported", info.name, typenum);
            return false;
    }

    int ndims = PyArray_NDIM(arr);
    const npy_intp* sizes = PyArray_DIMS(arr);
    const npy_intp* strides = PyArray_STRIDES(arr);
    bool isMultichannel = (ndims == 3) && (sizes[2] <= CV_CN_MAX);

    int dims = isMultichannel ? ndims - 1 : ndims;
    std::vector<int> size(dims);
    std::vector<size_t> step(dims);

    size_t elemsize = CV_ELEM_SIZE1(type);
    for (int i = dims - 1; i >= 0; --i) {
        size[i] = static_cast<int>(sizes[i]);
        step[i] = static_cast<size_t>(strides[i]) / elemsize;
    }

    if (isMultichannel) {
        type |= CV_MAKETYPE(0, sizes[2]);
    }

    mat = Mat(dims, size.data(), type, PyArray_DATA(arr));
    mat.allocator = &g_numpyAllocator;
    mat.u =
        g_numpyAllocator.allocate(obj, dims, size.data(), type, step.data());

    Py_INCREF(obj);
    return true;
}

auto NDArrayConverter::toNDArray(const cv::Mat& mat) -> PyObject* {
    if (!mat.data) {
        Py_RETURN_NONE;
    }
    Mat temp, *p = const_cast<Mat*>(&mat);
    if (!p->u || p->allocator != &g_numpyAllocator) {
        temp.allocator = &g_numpyAllocator;
        ERRWRAP2(p->copyTo(temp));
        p = &temp;
    }
    PyObject* obj = reinterpret_cast<PyObject*>(p->u->userdata);
    Py_INCREF(obj);
    return obj;
}

auto NDArrayConverter::toNDArrayList(const std::vector<cv::Mat>& mats)
    -> PyObject* {
    PyObject* list = PyList_New(mats.size());
    if (!list) {
        return nullptr;
    }
    for (size_t i = 0; i < mats.size(); ++i) {
        PyObject* obj = toNDArray(mats[i]);
        if (!obj) {
            Py_DECREF(list);
            return nullptr;
        }
        PyList_SET_ITEM(list, i, obj);
    }
    return list;
}

auto NDArrayConverter::toMatList(PyObject* obj,
                                 std::vector<cv::Mat>& mats) -> bool {
    if (!PyList_Check(obj)) {
        return false;
    }
    Py_ssize_t size = PyList_Size(obj);
    mats.resize(size);
    for (Py_ssize_t i = 0; i < size; ++i) {
        PyObject* item = PyList_GetItem(obj, i);
        if (!toMat(item, mats[i])) {
            return false;
        }
    }
    return true;
}