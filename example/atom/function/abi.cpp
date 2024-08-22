#include <iostream>
#include <vector>

#include "atom/function/abi.hpp"

// Example structures and classes to test demangling
struct MyStruct {
    int a;
    double b;
};

class MyClass {
public:
    void myMethod(int x) {}
};

int main() {
    // Demangle a simple type
    std::cout << "Demangled type for int: "
              << atom::meta::DemangleHelper::demangleType<int>() << std::endl;

    // Demangle a struct
    std::cout << "Demangled type for MyStruct: "
              << atom::meta::DemangleHelper::demangleType<MyStruct>()
              << std::endl;

    // Demangle a class
    std::cout << "Demangled type for MyClass: "
              << atom::meta::DemangleHelper::demangleType<MyClass>()
              << std::endl;

    // Use an instance to demangle
    MyClass myClassInstance;
    std::cout << "Demangled type for instance of MyClass: "
              << atom::meta::DemangleHelper::demangleType(myClassInstance)
              << std::endl;

    // Demangle multiple types
    std::vector<std::string_view> typesToDemangle = {
        "std::vector<int>", "std::map<std::string, std::vector<double>>",
        "MyClass::myMethod(int)"};

    auto demangledTypes =
        atom::meta::DemangleHelper::demangleMany(typesToDemangle);
    std::cout << "Demangled multiple types:\n";
    for (const auto& type : demangledTypes) {
        std::cout << " - " << type << std::endl;
    }
    return 0;
}
