#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "atom/function/conversion.hpp"  // Include your conversion header file

// Define some sample classes and types
class Base {
public:
    virtual ~Base() = default;
    virtual void print() const { std::cout << "Base class\n"; }
};

class Derived : public Base {
public:
    void print() const override { std::cout << "Derived class\n"; }
};

class AnotherBase {
public:
    virtual ~AnotherBase() = default;
    virtual void print() const { std::cout << "AnotherBase class\n"; }
};

class AnotherDerived : public AnotherBase {
public:
    void print() const override { std::cout << "AnotherDerived class\n"; }
};

// Define some sample conversion functions
void setupConversions() {
    // Create a shared instance of TypeConversions
    auto typeConversions = atom::meta::TypeConversions::createShared();

    // Add base and derived class conversions
    typeConversions->addBaseClass<Base, Derived>();
    typeConversions->addBaseClass<AnotherBase, AnotherDerived>();

    // Add vector conversions
    typeConversions->addVectorConversion<Derived, Base>();

    // Add map conversions (for demonstration purposes)
    typeConversions->addMapConversion<std::unordered_map, std::string,
                                      std::shared_ptr<Base>, std::string,
                                      std::shared_ptr<Derived>>();

    // Add sequence conversions
    typeConversions->addSequenceConversion<std::vector, Base, Derived>();
}

void conversionExamples() {
    // Create the conversions setup
    setupConversions();

    // Create a TypeConversions instance
    auto typeConversions = atom::meta::TypeConversions::createShared();

    // Sample objects for conversion
    std::shared_ptr<Derived> derived = std::make_shared<Derived>();
    std::shared_ptr<Base> base;

    // Perform conversions
    try {
        // Convert from Derived* to Base*
        base = std::any_cast<std::shared_ptr<Base>>(
            typeConversions->convert<std::shared_ptr<Base>,
                                     std::shared_ptr<Derived>>(derived));
        base->print();  // Should output: Derived class

        // Convert a vector of Derived to vector of Base
        std::vector<std::shared_ptr<Derived>> derivedVec = {derived};
        std::vector<std::shared_ptr<Base>> baseVec =
            std::any_cast<std::vector<std::shared_ptr<Base>>>(
                typeConversions->convert<std::vector<std::shared_ptr<Base>>,
                                         std::vector<std::shared_ptr<Derived>>>(
                    derivedVec));
        for (const auto& b : baseVec) {
            b->print();  // Should output: Derived class
        }

        // Convert a map from <string, shared_ptr<Base>> to <string,
        // shared_ptr<Derived>>
        std::unordered_map<std::string, std::shared_ptr<Base>> baseMap;
        baseMap["key"] = derived;
        auto convertedMap = std::any_cast<
            std::unordered_map<std::string, std::shared_ptr<Derived>>>(
            typeConversions->convert<
                std::unordered_map<std::string, std::shared_ptr<Derived>>,
                std::unordered_map<std::string, std::shared_ptr<Base>>>(
                baseMap));
        for (const auto& [key, value] : convertedMap) {
            value->print();  // Should output: Derived class
        }

    } catch (const atom::meta::BadConversionException& e) {
        std::cerr << "Conversion error: " << e.what() << std::endl;
    }
}

int main() {
    conversionExamples();
    return 0;
}
