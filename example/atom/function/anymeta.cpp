#include <iostream>

#include "atom/function/anymeta.hpp"

// Sample class to demonstrate the functionality
class Sample {
public:
    Sample(int initialValue) : value(initialValue) {}

    int getValue() const { return value; }

    void setValue(int newValue) { value = newValue; }

    void display() const {
        std::cout << "Current value: " << value << std::endl;
    }

private:
    int value;
};

// Register the Sample class in the TypeRegistry
void registerSampleType() {
    atom::meta::TypeMetadata metadata;

    // Adding methods
    metadata.addMethod(
        "display",
        [](std::vector<atom::meta::BoxedValue> args) -> atom::meta::BoxedValue {
            auto& obj = std::any_cast<Sample&>(args[0].get());
            obj.display();
            return {};
        });

    // Adding properties
    metadata.addProperty(
        "value",
        [](const atom::meta::BoxedValue& obj) -> atom::meta::BoxedValue {
            const Sample& sample = std::any_cast<const Sample&>(obj.get());
            return atom::meta::makeBoxedValue(sample.getValue());
        },
        [](atom::meta::BoxedValue& obj, const atom::meta::BoxedValue& value) {
            Sample& sample = std::any_cast<Sample&>(obj.get());
            sample.setValue(std::any_cast<int>(value.get()));
        });

    // Registering the type
    atom::meta::TypeRegistry::instance().registerType("Sample", metadata);
}

int main() {
    // Register the Sample type with its metadata
    registerSampleType();

    // Create an instance of Sample and box it
    Sample sampleObj(10);
    atom::meta::BoxedValue boxedSample = atom::meta::makeBoxedValue(sampleObj);

    // Call the display method dynamically
    callMethod(boxedSample, "display", {});

    // Get the value property
    auto value = getProperty(boxedSample, "value");
    std::cout << "Value from property: " << std::any_cast<int>(value.get())
              << std::endl;

    // Set a new value using the setter property
    setProperty(boxedSample, "value", atom::meta::makeBoxedValue(42));
    std::cout << "Updated value." << std::endl;

    // Call the display method again to show updated value
    callMethod(boxedSample, "display", {});

    return 0;
}
