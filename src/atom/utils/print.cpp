#include "print.hpp"

namespace atom::utils {

void printProgressBar(float progress, int barWidth) {
    int pos = static_cast<int>(barWidth * progress);
    std::cout << "[";
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) {
            std::cout << "=";
        } else if (i == pos) {
            std::cout << ">";
        } else {
            std::cout << " ";
        }
    }
    std::cout << "] " << static_cast<int>(progress * PERCENTAGE_MULTIPLIER)
              << " %\r";
    std::cout.flush();
}

void printTable(const std::vector<std::vector<std::string>>& data) {
    if (data.empty()) {
        return;
    }

    // 计算每列的最大宽度
    std::vector<size_t> colWidths(data[0].size(), 0);
    for (const auto& row : data) {
        for (size_t i = 0; i < row.size(); ++i) {
            colWidths[i] = std::max(colWidths[i], row[i].length());
        }
    }

    // 打印表格
    for (const auto& row : data) {
        for (size_t i = 0; i < row.size(); ++i) {
            std::cout << "| " << std::setw(static_cast<int>(colWidths[i]))
                      << std::left << row[i] << " ";
        }
        std::cout << "|" << std::endl;

        // 打印分隔线
        if (&row == data.data()) {
            for (size_t i = 0; i < row.size(); ++i) {
                std::cout << "+-" << std::string(colWidths[i], '-') << "-";
            }
            std::cout << "+" << std::endl;
        }
    }
}

void printJson(const std::string& json, int indent) {
    int level = 0;
    bool inQuotes = false;
    bool inEscape = false;

    for (char character : json) {
        if (inEscape) {
            std::cout << character;
            inEscape = false;
        } else {
            switch (character) {
                case '{':
                case '[':
                    std::cout << character << std::endl;
                    level++;
                    std::cout << std::string(
                        static_cast<size_t>(level) * indent, ' ');
                    break;
                case '}':
                case ']':
                    std::cout << std::endl;
                    level--;
                    std::cout
                        << std::string(static_cast<size_t>(level) * indent, ' ')
                        << character;
                    break;
                case ',':
                    std::cout << character << std::endl;
                    std::cout << std::string(
                        static_cast<size_t>(level) * indent, ' ');
                    break;
                case ':':
                    std::cout << character << " ";
                    break;
                case '\"':
                    if (!inQuotes) {
                        inQuotes = true;
                    } else if (!inEscape) {
                        inQuotes = false;
                    }
                    std::cout << character;
                    break;
                case '\\':
                    if (inQuotes) {
                        inEscape = true;
                    }
                    std::cout << character;
                    break;
                default:
                    std::cout << character;
            }
        }
    }
    std::cout << std::endl;
}

void printBarChart(const std::map<std::string, int>& data, int maxWidth) {
    int maxValue =
        std::max_element(data.begin(), data.end(),
                         [](const auto& elementA, const auto& elementB) {
                             return elementA.second < elementB.second;
                         })
            ->second;

    for (const auto& [label, value] : data) {
        int barWidth =
            static_cast<int>(static_cast<double>(value) / maxValue * maxWidth);
        std::cout << std::setw(MAX_LABEL_WIDTH) << std::left << label << " |";
        std::cout << std::string(barWidth, '#')
                  << std::string(maxWidth - barWidth, ' ');
        std::cout << "| " << value << std::endl;
    }
}

auto generateRandomString(size_t length) -> std::string {
    const std::string characters =
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::random_device randomDevice;
    std::mt19937 generator(randomDevice());
    std::uniform_int_distribution<> distribution(
        0, static_cast<int>(characters.size() - 1));

    std::string result;
    result.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        result += characters[distribution(generator)];
    }
    return result;
}

auto xorEncryptDecrypt(const std::string& input,
                       const std::string& key) -> std::string {
    std::string output = input;
    for (size_t i = 0; i < input.length(); ++i) {
        output[i] = static_cast<char>(input[i] ^ key[i % key.length()]);
    }
    return output;
}

}  // namespace atom::utils
