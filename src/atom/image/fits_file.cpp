#include "fits_file.hpp"
#include <fstream>
#include <stdexcept>

void FITSFile::readFITS(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    hdus.clear();
    while (file.peek() != EOF) {
        auto hdu = std::make_unique<ImageHDU>();
        hdu->readHDU(file);
        hdus.push_back(std::move(hdu));
    }
}

void FITSFile::writeFITS(const std::string& filename) const {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot create file: " + filename);
    }

    for (const auto& hdu : hdus) {
        hdu->writeHDU(file);
    }
}

size_t FITSFile::getHDUCount() const { return hdus.size(); }

const HDU& FITSFile::getHDU(size_t index) const {
    if (index >= hdus.size()) {
        throw std::out_of_range("HDU index out of range");
    }
    return *hdus[index];
}

HDU& FITSFile::getHDU(size_t index) {
    if (index >= hdus.size()) {
        throw std::out_of_range("HDU index out of range");
    }
    return *hdus[index];
}

void FITSFile::addHDU(std::unique_ptr<HDU> hdu) {
    hdus.push_back(std::move(hdu));
}
