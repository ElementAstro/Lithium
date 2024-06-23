#include <pybind11/pybind11.h>

#include "cache.hpp"

using py = pybind11;
using namespace atom::search;

template <typename T>
void bind_cache(py::module &m, const char *name) {
    py::class_<ResourceCache<T>>(m, name)
        .def(py::init<int>())
        .def("insert", &ResourceCache<T>::insert)
        .def("contains", &ResourceCache<T>::contains)
        .def("get", &ResourceCache<T>::get)
        .def("remove", &ResourceCache<T>::remove)
        .def("clear", &ResourceCache<T>::clear);
    }