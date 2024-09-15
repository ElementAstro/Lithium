import pytest
import os
import tempfile
from ..tools.generator import generate_pybind11_bindings, parse_hpp_files

@pytest.fixture
def temp_dir():
    with tempfile.TemporaryDirectory() as tmpdirname:
        yield tmpdirname

@pytest.fixture
def sample_header_files(temp_dir):
    header_files = {
        "functions.hpp": """
            #pragma once
            int add(int a, int b);
            double multiply(double x, double y);
        """,
        "classes.hpp": """
            #pragma once
            class MyClass {
            public:
                MyClass();
                ~MyClass();
                int getValue() const;
                void setValue(int value);
            private:
                int value;
            };
        """,
        "enums.hpp": """
            #pragma once
            enum Color {
                RED,
                GREEN,
                BLUE
            };
        """,
        "namespaces.hpp": """
            #pragma once
            namespace MyNamespace {
                void doSomething();
                class MyClass {
                public:
                    void method();
                };
            }
        """
    }
    for filename, content in header_files.items():
        with open(os.path.join(temp_dir, filename), 'w') as f:
            f.write(content)
    return temp_dir

def test_generate_pybind11_bindings_functions(sample_header_files):
    ast_info_list = parse_hpp_files(sample_header_files)
    bindings_file = os.path.join(sample_header_files, "bindings_functions.cpp")
    generate_pybind11_bindings(ast_info_list, bindings_file)
    
    with open(bindings_file, 'r') as f:
        content = f.read()
    
    assert 'm.def("add", &add);' in content
    assert 'm.def("multiply", &multiply);' in content

def test_generate_pybind11_bindings_classes(sample_header_files):
    ast_info_list = parse_hpp_files(sample_header_files)
    bindings_file = os.path.join(sample_header_files, "bindings_classes.cpp")
    generate_pybind11_bindings(ast_info_list, bindings_file)
    
    with open(bindings_file, 'r') as f:
        content = f.read()
    
    assert 'py::class_<MyClass>(m, "MyClass")' in content
    assert '.def(py::init<>())' in content
    assert '.def("__del__", &MyClass::~MyClass)' in content
    assert '.def("getValue", &MyClass::getValue)' in content
    assert '.def("setValue", &MyClass::setValue)' in content
    assert '.def_readwrite("value", &MyClass::value)' in content

def test_generate_pybind11_bindings_enums(sample_header_files):
    ast_info_list = parse_hpp_files(sample_header_files)
    bindings_file = os.path.join(sample_header_files, "bindings_enums.cpp")
    generate_pybind11_bindings(ast_info_list, bindings_file)
    
    with open(bindings_file, 'r') as f:
        content = f.read()
    
    assert 'py::enum_<Color>(m, "Color")' in content
    assert '.value("RED", Color::RED)' in content
    assert '.value("GREEN", Color::GREEN)' in content
    assert '.value("BLUE", Color::BLUE)' in content
    assert '.export_values();' in content

def test_generate_pybind11_bindings_namespaces(sample_header_files):
    ast_info_list = parse_hpp_files(sample_header_files)
    bindings_file = os.path.join(sample_header_files, "bindings_namespaces.cpp")
    generate_pybind11_bindings(ast_info_list, bindings_file)
    
    with open(bindings_file, 'r') as f:
        content = f.read()
    
    assert 'py::module_ MyNamespace = m.def_submodule("MyNamespace");' in content
    assert 'MyNamespace.def("doSomething", &MyNamespace::doSomething);' in content
    assert 'py::class_<MyNamespace::MyClass>(MyNamespace, "MyClass")' in content
    assert '.def("method", &MyNamespace::MyClass::method)' in content