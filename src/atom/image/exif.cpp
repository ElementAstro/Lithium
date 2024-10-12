#include "exif.hpp"

#include <cstdint>
#include <cstring>
#include <fstream>
#include <vector>

#include "atom/log/loguru.hpp"

namespace atom::image {

constexpr int BYTE_SHIFT_8 = 8;
constexpr int BYTE_SHIFT_16 = 16;
constexpr int BYTE_SHIFT_24 = 24;
constexpr int EXIF_HEADER_OFFSET = 10;
constexpr int EXIF_HEADER_SIZE = 6;
constexpr int IFD_ENTRY_SIZE = 12;
constexpr int GPS_COORDINATE_SIZE = 24;
constexpr int RATIONAL_SIZE = 8;
constexpr int JPEG_SOI_MARKER = 0xFFD8;
constexpr int EXIF_MARKER = 0xFFE1;
constexpr int TIFF_LITTLE_ENDIAN = 0x4949;
constexpr int TIFF_BIG_ENDIAN = 0x4D4D;

ExifParser::ExifParser(const std::string& filename) : m_filename(filename) {}

auto ExifParser::readUint16Be(const std::byte* data) -> uint16_t {
    return static_cast<uint16_t>(std::to_integer<int>(data[0])
                                 << BYTE_SHIFT_8) |
           static_cast<uint16_t>(std::to_integer<int>(data[1]));
}

auto ExifParser::readUint32Be(const std::byte* data) -> uint32_t {
    return (std::to_integer<uint32_t>(data[0]) << BYTE_SHIFT_24) |
           (std::to_integer<uint32_t>(data[1]) << BYTE_SHIFT_16) |
           (std::to_integer<uint32_t>(data[2]) << BYTE_SHIFT_8) |
           std::to_integer<uint32_t>(data[3]);
}

auto ExifParser::readUint16Le(const std::byte* data) -> uint16_t {
    return static_cast<uint16_t>(std::to_integer<int>(data[0])) |
           static_cast<uint16_t>(std::to_integer<int>(data[1]) << BYTE_SHIFT_8);
}

auto ExifParser::readUint32Le(const std::byte* data) -> uint32_t {
    return std::to_integer<uint32_t>(data[0]) |
           (std::to_integer<uint32_t>(data[1]) << BYTE_SHIFT_8) |
           (std::to_integer<uint32_t>(data[2]) << BYTE_SHIFT_16) |
           (std::to_integer<uint32_t>(data[3]) << BYTE_SHIFT_24);
}

auto ExifParser::parseRational(const std::byte* data,
                               bool isLittleEndian) -> double {
    uint32_t numerator =
        isLittleEndian ? readUint32Le(data) : readUint32Be(data);
    uint32_t denominator =
        isLittleEndian ? readUint32Le(data + 4) : readUint32Be(data + 4);
    return denominator == 0 ? 0.0
                            : static_cast<double>(numerator) / denominator;
}

auto ExifParser::parseGPSCoordinate(const std::byte* data,
                                    bool isLittleEndian) -> std::string {
    double degrees = parseRational(data, isLittleEndian);
    double minutes = parseRational(data + RATIONAL_SIZE, isLittleEndian);
    double seconds = parseRational(data + 2 * RATIONAL_SIZE, isLittleEndian);
    double coordinate = degrees + minutes / 60.0 + seconds / 3600.0;
    return std::to_string(coordinate);
}

auto ExifParser::parseIFD(const std::byte* data, bool isLittleEndian,
                          const std::byte* tiffStart,
                          size_t bufferSize) -> bool {
    uint16_t entryCount =
        isLittleEndian ? readUint16Le(data) : readUint16Be(data);
    data += 2;
    for (int i = 0; i < entryCount; ++i) {
        if (data + IFD_ENTRY_SIZE > tiffStart + bufferSize) {
            LOG_F(ERROR, "Invalid IFD entry position, out of bounds.");
            return false;
        }
        uint16_t tag = isLittleEndian ? readUint16Le(data) : readUint16Be(data);
        uint16_t type =
            isLittleEndian ? readUint16Le(data + 2) : readUint16Be(data + 2);
        uint32_t count =
            isLittleEndian ? readUint32Le(data + 4) : readUint32Be(data + 4);
        uint32_t valueOffset =
            isLittleEndian ? readUint32Le(data + 8) : readUint32Be(data + 8);
        data += IFD_ENTRY_SIZE;

        std::string value;
        if (type == 2) {
            const std::byte* valuePtr =
                (count <= 4) ? reinterpret_cast<const std::byte*>(&valueOffset)
                             : (tiffStart + valueOffset);
            if (valuePtr + count - 1 > tiffStart + bufferSize) {
                LOG_F(ERROR, "Invalid string offset, out of bounds.");
                continue;
            }
            value =
                std::string(reinterpret_cast<const char*>(valuePtr), count - 1);
        } else if ((type == 3 || type == 4) && count == 1) {
            value = std::to_string(valueOffset & 0xFFFF);
        } else if (type == 5 && count == 1) {
            if (tiffStart + valueOffset + RATIONAL_SIZE >
                tiffStart + bufferSize) {
                LOG_F(ERROR, "Invalid rational offset, out of bounds.");
                continue;
            }
            double rationalValue =
                parseRational(tiffStart + valueOffset, isLittleEndian);
            value = std::to_string(rationalValue);
        } else if (tag == 0x0002 || tag == 0x0004) {
            if (tiffStart + valueOffset + GPS_COORDINATE_SIZE >
                tiffStart + bufferSize) {
                LOG_F(ERROR, "Invalid GPS coordinate offset, out of bounds.");
                continue;
            }
            value = parseGPSCoordinate(tiffStart + valueOffset, isLittleEndian);
        } else {
            value = "Unsupported format";
        }

        switch (tag) {
            case 0x010F:
                m_exifData.cameraMake = value;
                break;
            case 0x0110:
                m_exifData.cameraModel = value;
                break;
            case 0x9003:
                m_exifData.dateTime = value;
                break;
            case 0x829A:
                m_exifData.exposureTime = value;
                break;
            case 0x829D:
                m_exifData.fNumber = value;
                break;
            case 0x8827:
                m_exifData.isoSpeed = value;
                break;
            case 0x920A:
                m_exifData.focalLength = value;
                break;
            case 0x0002:
                m_exifData.gpsLatitude = value;
                break;
            case 0x0004:
                m_exifData.gpsLongitude = value;
                break;
            default:
                break;
        }
    }
    return true;
}

auto ExifParser::parse() -> bool {
    std::ifstream file(m_filename, std::ios::binary);
    if (!file.is_open()) {
        LOG_F(ERROR, "Cannot open file: {}", m_filename);
        return false;
    }

    std::vector<char> charBuffer((std::istreambuf_iterator<char>(file)),
                                 std::istreambuf_iterator<char>());
    std::vector<std::byte> buffer(charBuffer.size());
    std::transform(
        charBuffer.begin(), charBuffer.end(), buffer.begin(),
        [](char c) { return std::byte{static_cast<unsigned char>(c)}; });
    file.close();

    if (buffer.size() < 2 || buffer[0] != std::byte{0xFF} ||
        buffer[1] != std::byte{0xD8}) {
        LOG_F(ERROR, "Not a valid JPEG file!");
        return false;
    }

    size_t pos = 2;
    while (pos < buffer.size()) {
        if (pos + 4 > buffer.size()) {
            LOG_F(ERROR, "Unexpected end of file while searching for markers.");
            return false;
        }

        if (buffer[pos] == std::byte{0xFF}) {
            uint8_t marker = std::to_integer<uint8_t>(buffer[pos + 1]);
            uint16_t segmentLength = readUint16Be(&buffer[pos + 2]);

            if (pos + 2 + segmentLength > buffer.size()) {
                LOG_F(ERROR,
                      "Invalid segment length, segment exceeds file bounds.");
                return false;
            }

            if (marker == EXIF_MARKER &&
                std::memcmp(&buffer[pos + 4], "Exif\0\0", EXIF_HEADER_SIZE) ==
                    0) {
                const std::byte* tiffHeader = &buffer[pos + EXIF_HEADER_OFFSET];
                uint16_t byteOrder = readUint16Be(tiffHeader);
                bool isLittleEndian = (byteOrder == TIFF_LITTLE_ENDIAN);

                uint32_t ifdOffset = isLittleEndian
                                         ? readUint32Le(&tiffHeader[4])
                                         : readUint32Be(&tiffHeader[4]);
                if (tiffHeader + ifdOffset > &buffer[pos] + segmentLength) {
                    LOG_F(ERROR,
                          "Invalid IFD offset, exceeds EXIF data bounds.");
                    return false;
                }

                return parseIFD(tiffHeader + ifdOffset, isLittleEndian,
                                tiffHeader, segmentLength);
            }

            pos += 2 + segmentLength;
        } else {
            ++pos;
        }
    }
    return true;
}

auto ExifParser::getExifData() const -> const ExifData& { return m_exifData; }

}  // namespace atom::image
