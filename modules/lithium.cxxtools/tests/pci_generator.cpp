#include "pci_generator.hpp"

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace fs = std::filesystem;

class PCIGeneratorTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::ofstream outfile("test_pci_data.txt");
        outfile << "1234  Vendor A\n"
                << "\t5678  Device A1\n"
                << "\t5679  Device A2\n"
                << "1235  Vendor B\n"
                << "\t6789  Device B1\n";
        outfile.close();
    }

    void TearDown() override { fs::remove("test_pci_data.txt"); }
};

TEST_F(PCIGeneratorTest, ParseAndGeneratePCIInfo) {
    std::stringstream output;
    std::streambuf* oldCoutBuf = std::cout.rdbuf(output.rdbuf());

    EXPECT_NO_THROW(parseAndGeneratePCIInfo("test_pci_data.txt"));

    std::cout.rdbuf(oldCoutBuf);

    std::string expectedOutput =
        "#define ATOM_SYSTEM_GENERATED_PCI_INDICES\n"
        " \\\n\t{0x1234, 0}, \\\n\t{0x1235, 1},"
        "\n\n\n#define ATOM_SYSTEM_GENERATED_PCI_VENDORS"
        " \\\n\t{0x1234, R\"(Vendor A)\", {0, 1, }}, \\\n\t{0x1235, R\"(Vendor "
        "B)\", {2, }},"
        "\n\n\n#define ATOM_SYSTEM_GENERATED_PCI_DEVICES"
        " \\\n\t{0x5678, R\"(Device A1)\"}, \\\n\t{0x5679, R\"(Device A2)\"}, "
        "\\\n\t{0x6789, R\"(Device B1)\"},"
        "\n\n\nnamespace {}\n";

    EXPECT_EQ(output.str(), expectedOutput);
}

TEST_F(PCIGeneratorTest, FileNotFound) {
    std::stringstream errorOutput;
    std::streambuf* oldCerrBuf = std::cerr.rdbuf(errorOutput.rdbuf());

    EXPECT_THROW(parseAndGeneratePCIInfo("nonexistent.txt"),
                 std::runtime_error);

    std::cerr.rdbuf(oldCerrBuf);

    EXPECT_EQ(errorOutput.str(), "Couldn't open input file\n");
}
