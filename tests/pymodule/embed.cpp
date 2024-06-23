#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pocketpy.h>
namespace py = pybind11;

#include <cassert>
#include <iostream>
#include <sstream>

#include "atom/script/_json.hpp"

PYBIND11_EMBEDDED_MODULE(pocketpy, m) {
    m.doc() = "pocketpy";
    m.def("hello", []() {
        std::cout << "hello, world!" << std::endl;
    });
}

int main() {
    py::scoped_interpreter guard{};
    py::module_ m = py::module_::import("pocketpy");
    m.attr("hello")();
}