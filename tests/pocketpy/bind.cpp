#include <cassert>
#include <iostream>
#include <sstream>

#include "pybind11/pybind11.h"

std::string read_stdin(){
    std::stringstream ss; char ch;
    while(std::cin.get(ch)) ss << ch;
    return ss.str();
}

// test for simple struct, all member is built-in type.
struct Point {
    int x;
    int y;

private:
    int z;

public:
    inline static int constructor_calls = 0;
    inline static int copy_constructor_calls = 0;
    inline static int move_constructor_calls = 0;
    inline static int destructor_calls = 0;

    Point() : x(0), y(0), z(0) { constructor_calls++; }

    Point(int x, int y, int z) : x(x), y(y), z(z) { constructor_calls++; }

    Point(const Point& p) : x(p.x), y(p.y) {
        copy_constructor_calls++;
        constructor_calls++;
    }

    Point(Point&& p) noexcept : x(p.x), y(p.y) {
        move_constructor_calls++;
        constructor_calls++;

    }

    Point& operator= (const Point& p) {
        x = p.x;
        y = p.y;
        z = p.z;
        return *this;
    }

    ~Point() { destructor_calls++; }

    int& get_z() { return z; }

    void set_z(int z) { this->z = z; }

    std::string stringfy() const {
        return "(" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")";
    }
};

struct Line {
    Point start;
    Point end;
};

namespace py = pybind11;

int main() {
    py::initialize();
    try {
        py::module m = py::module::import("__main__");
        py::class_<Point>(m, "Point")
            .def(py::init<>())
            .def(py::init<int, int, int>())
            .def_readwrite("x", &Point::x)
            .def_readwrite("y", &Point::y)
            .def_property("z", &Point::get_z, &Point::set_z)
            .def("__repr__", &Point::stringfy);

        py::class_<Line>(m, "Line")
            .def(py::init<>())
            .def_readwrite("start", &Line::start)
            .def_readwrite("end", &Line::end);

        py::vm->exec(read_stdin());
    } catch(const pkpy::Exception& e) { std::cerr << e.msg << '\n'; }

    py::finalize();
    assert(Point::constructor_calls == Point::destructor_calls);
    assert(Point::copy_constructor_calls == 0);
    assert(Point::move_constructor_calls == 0);
    return 0;
}
