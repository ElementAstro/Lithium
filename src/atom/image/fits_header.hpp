#ifndef ATOM_IMAGE_FITS_HEADER_HPP
#define ATOM_IMAGE_FITS_HEADER_HPP

#include <array>
#include <string>
#include <vector>

class FITSHeader {
public:
    static constexpr int FITS_HEADER_UNIT_SIZE = 2880;
    static constexpr int FITS_HEADER_CARD_SIZE = 80;

    struct KeywordRecord {
        std::array<char, 8> keyword;
        std::array<char, 72> value;
    };

    void addKeyword(const std::string& keyword, const std::string& value);
    std::string getKeywordValue(const std::string& keyword) const;
    std::vector<char> serialize() const;
    void deserialize(const std::vector<char>& data);

private:
    std::vector<KeywordRecord> records;
};

#endif