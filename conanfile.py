from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps, cmake_layout


class ConanFile(ConanFile):
    name = "lithium"
    version = "1.0.0"

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "CMakeLists.txt", "src/*"

    requires = [
        "argparse/3.0",
        "backward-cpp/1.6",
        "cpp-httplib/0.15.3",
        "libzippp/7.1-1.10.1",
        "openssl/3.2.1",
        "zlib/1.3.1",
        "oatpp/1.3.0",
        "oatpp-websocket/1.3.0",
        "oatpp-openssl/1.3.0",
        "oatpp-swagger/1.3.0",
        "loguru/cci.20230406",
        "magic_enum/0.9.5",
        "cfitsio/4.3.1",
        "tinyxml2/10.0.0",
        "pybind11/2.12.0",
        "pybind11_json/0.2.13",
        "cpython/3.12.2",
        "fmt/10.2.1",
        "opencv/4.9.0"
    ]

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()

        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["my_project"]