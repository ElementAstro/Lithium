#include "atom/io/io.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

namespace fs = std::filesystem;

// Function to create a sample file
void createSampleFiles(const std::string& baseDir) {
    fs::create_directory(baseDir);

    std::ofstream outFile(baseDir + "/file1.txt");
    outFile << "Contents of file 1." << std::endl;
    outFile.close();

    outFile.open(baseDir + "/file2.txt");
    outFile << "Contents of file 2." << std::endl;
    outFile.close();

    outFile.open(baseDir + "/file3.txt");
    outFile << "Contents of file 3." << std::endl;
    outFile.close();
}

// Function to demonstrate file operations
void demonstrateFileOperations() {
    const std::string directory = "sample_dir";  // Directory for test files
    createSampleFiles(directory);

    // Check if folder exists
    if (atom::io::isFolderExists(directory)) {
        std::cout << "Folder '" << directory << "' exists." << std::endl;
    }

    // Check if files exist
    std::vector<std::string> filenames = {
        "sample_dir/file1.txt", "sample_dir/file2.txt", "sample_dir/file3.txt"};

    for (const auto& filename : filenames) {
        if (atom::io::isFileExists(filename)) {
            std::cout << "File '" << filename << "' exists." << std::endl;
        }
    }

    // Get file sizes
    for (const auto& filename : filenames) {
        std::size_t size = atom::io::fileSize(filename);
        std::cout << "Size of " << filename << ": " << size << " bytes."
                  << std::endl;
    }

    // Split a file
    const std::string fileToSplit = "sample_dir/file1.txt";
    const std::size_t chunkSize = 10;  // Split into chunks of 10 bytes
    atom::io::splitFile(fileToSplit, chunkSize, "part_");

    // Check split files
    for (size_t i = 0; i < 3; ++i) {  // Assuming 3 parts created from file1.txt
        std::string partName = "part_" + std::to_string(i) + ".txt";
        if (atom::io::isFileExists(partName)) {
            std::cout << "Split file '" << partName << "' exists." << std::endl;
        }
    }

    // Merge split files
    std::vector<std::string> partFiles = {"part_0.txt", "part_1.txt",
                                          "part_2.txt"};
    atom::io::mergeFiles("merged_file1.txt", partFiles);
    std::cout << "Merged files into 'merged_file1.txt'" << std::endl;

    // Clean up by removing sample directory
    fs::remove_all(directory);
    std::cout << "Removed sample directory and its contents." << std::endl;
}

int main() {
    demonstrateFileOperations();
    return 0;
}
