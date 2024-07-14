#ifndef LITHIUM_CXXTOOLS_PCI_GENERATOR_HPP
#define LITHIUM_CXXTOOLS_PCI_GENERATOR_HPP

#include <string_view>

// Parses the given PCI info file and generates the corresponding C++ code.
void parseAndGeneratePCIInfo(std::string_view filename);

#endif  // LITHIUM_CXXTOOLS_PCI_GENERATOR_HPP
