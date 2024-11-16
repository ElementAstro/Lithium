#include "base64.hpp"

#include <array>
#include <string>

constexpr std::string_view BASE64_CHARS =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

constexpr unsigned char MASK_0X_FC = 0xFC;
constexpr unsigned char MASK_0X03 = 0x03;
constexpr unsigned char MASK_0X_F0 = 0xF0;
constexpr unsigned char MASK_0X0_F = 0x0F;
constexpr unsigned char MASK_0X_C0 = 0xC0;
constexpr unsigned char MASK_0X3_F = 0x3F;
constexpr unsigned char MASK_0X30 = 0x30;
constexpr unsigned char MASK_0X3_C = 0x3C;
constexpr int SHIFT_6 = 6;
constexpr int SHIFT_4 = 4;
constexpr int SHIFT_2 = 2;

auto base64Encode(unsigned char const* bytes_to_encode,
                  unsigned int input_length) -> std::string {
    std::string result;
    int index3 = 0;
    std::array<unsigned char, 3> charArray3;
    std::array<unsigned char, 4> charArray4;

    while ((input_length--) != 0U) {
        charArray3[index3++] = *(bytes_to_encode++);
        if (index3 == 3) {
            charArray4[0] = (charArray3[0] & MASK_0X_FC) >> SHIFT_2;
            charArray4[1] = ((charArray3[0] & MASK_0X03) << SHIFT_4) +
                            ((charArray3[1] & MASK_0X_F0) >> SHIFT_4);
            charArray4[2] = ((charArray3[1] & MASK_0X0_F) << SHIFT_2) +
                            ((charArray3[2] & MASK_0X_C0) >> SHIFT_6);
            charArray4[3] = charArray3[2] & MASK_0X3_F;

            for (index3 = 0; index3 < 4; index3++) {
                result += BASE64_CHARS[charArray4[index3]];
            }
            index3 = 0;
        }
    }

    if (index3 != 0) {
        for (int j = index3; j < 3; j++) {
            charArray3[j] = '\0';
        }

        charArray4[0] = (charArray3[0] & MASK_0X_FC) >> SHIFT_2;
        charArray4[1] = ((charArray3[0] & MASK_0X03) << SHIFT_4) +
                        ((charArray3[1] & MASK_0X_F0) >> SHIFT_4);
        charArray4[2] = ((charArray3[1] & MASK_0X0_F) << SHIFT_2) +
                        ((charArray3[2] & MASK_0X_C0) >> SHIFT_6);
        charArray4[3] = charArray3[2] & MASK_0X3_F;

        for (int j = 0; j < index3 + 1; j++) {
            result += BASE64_CHARS[charArray4[j]];
        }

        while (index3++ < 3) {
            result += '=';
        }
    }

    return result;
}

static inline auto isBase64(unsigned char character) -> bool {
    return ((isalnum(character) != 0) || (character == '+') ||
            (character == '/'));
}

auto base64Decode(std::string const& encoded_string) -> std::string {
    std::size_t inputLength = encoded_string.size();
    int index3 = 0;
    int inputIndex = 0;
    std::array<unsigned char, 4> charArray4;
    std::array<unsigned char, 3> charArray3;
    std::string result;

    while (((inputLength--) != 0) && (encoded_string[inputIndex] != '=') &&
           isBase64(encoded_string[inputIndex])) {
        charArray4[index3++] = encoded_string[inputIndex];
        inputIndex++;
        if (index3 == 4) {
            for (index3 = 0; index3 < 4; index3++) {
                charArray4[index3] = BASE64_CHARS.find(charArray4[index3]);
            }

            charArray3[0] = (charArray4[0] << SHIFT_2) +
                            ((charArray4[1] & MASK_0X30) >> SHIFT_4);
            charArray3[1] = ((charArray4[1] & MASK_0X0_F) << SHIFT_4) +
                            ((charArray4[2] & MASK_0X3_C) >> SHIFT_2);
            charArray3[2] =
                ((charArray4[2] & MASK_0X03) << SHIFT_6) + charArray4[3];

            for (index3 = 0; index3 < 3; index3++) {
                result += charArray3[index3];
            }
            index3 = 0;
        }
    }

    if (index3 != 0) {
        for (int j = 0; j < index3; j++) {
            charArray4[j] = BASE64_CHARS.find(charArray4[j]);
        }

        charArray3[0] = (charArray4[0] << SHIFT_2) +
                        ((charArray4[1] & MASK_0X30) >> SHIFT_4);
        charArray3[1] = ((charArray4[1] & MASK_0X0_F) << SHIFT_4) +
                        ((charArray4[2] & MASK_0X3_C) >> SHIFT_2);
        charArray3[2] =
            ((charArray4[2] & MASK_0X03) << SHIFT_6) + charArray4[3];

        for (int j = 0; j < index3 - 1; j++) {
            result += charArray3[j];
        }
    }

    return result;
}