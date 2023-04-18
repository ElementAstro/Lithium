#include "jsearch.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>

#include <spdlog/spdlog.h>

namespace OpenAPT::JASX
{
    std::vector<Data> ReadFromJson(const std::string& filename) {
        std::ifstream file(filename);
        if (!file) {
            spdlog::error("Error opening file for reading.");
            throw std::runtime_error("Error opening file for reading.");
        }

        json j;
        file >> j;

        std::vector<Data> data;
        for (const auto& element : j) {
            Data d;
            d.Id = element["Id"];
            d.Name = element["Name"];
            d.Type = element["Type"];
            d.RA = element["RA"];
            d.Dec = element["Dec"];
            d.Const = element["Const"];
            data.push_back(d);
        }

        return data;
    }

    void WriteToJson(const std::vector<Data>& data, const std::string& filename) {
        json j;
        for (const auto& d : data) {
            j.push_back({{"Id", d.Id}, {"Name", d.Name}, {"Type", d.Type}, {"RA", d.RA}, {"Dec", d.Dec}, {"Const", d.Const}});
        }

        std::ofstream out_file(filename);
        if (!out_file) {
            spdlog::error("Error opening file for writing.");
            throw std::runtime_error("Error opening file for writing.");
        }

        out_file << std::setw(4) << j;
    }

    void InsertData(std::vector<Data>& data, const Data& d) {
        data.push_back(d);
    }

    void DeleteData(std::vector<Data>& data, const std::string& name) {
        data.erase(std::remove_if(data.begin(), data.end(), [&name](const Data& d) { return d.Name == name; }), data.end());
    }

    void SortByName(std::vector<Data>& data) {
        std::sort(data.begin(), data.end(), [](const Data& a, const Data& b) { return a.Name < b.Name; });
    }

    std::vector<Data> FilterBy(const std::vector<Data>& data, std::function<bool(const Data&)> filter) {
        std::vector<Data> result;

        std::copy_if(data.begin(), data.end(), std::back_inserter(result), filter);

        return result;
    }

    std::vector<Data> SearchByName(const std::vector<Data>& data, const std::string& name) {
        return FilterBy(data, [&name](const Data& d) { return d.Name.find(name) != std::string::npos; });
    }

    double ToDecimal(const std::string& str) {
        double deg, min, sec;
        sscanf(str.c_str(), "%lf:%lf:%lf", &deg, &min, &sec);
        deg += min / 60 + sec / 3600;
        return deg;
    }

    std::vector<Data> SearchByRaDec(const std::vector<Data>& data, std::string ra, std::string dec, double ra_range, double dec_range) {
        double target_ra = ToDecimal(ra);
        double target_dec = ToDecimal(dec);

        auto within_range = [target_ra, target_dec, ra_range, dec_range](const Data& d) -> bool {
            double d_ra = ToDecimal(d.RA);
            double d_dec = ToDecimal(d.Dec);

            return std::abs(d_ra - target_ra) <= ra_range && std::abs(d_dec - target_dec) <= dec_range;
        };

        return FilterBy(data, within_range);
    }
}



/*
int main() {
    std::vector<Data> data = ReadFromJson("data.json");

    // 插入一条数据
    Data d;
    d.Id = 10001;
    d.Name = "NGC 2264";
    d.Type = "Open Cluster";
    d.RA = "06:41:08.9";
    d.Dec = "+09:53:00";
    d.Const = "Monoceros";
    InsertData(data, d);

    // 根据名字删除一条数据
    DeleteData(data, "Pleiades");

    // 按名字排序
    SortByName(data);

    // 根据名字查找数据
    auto result = SearchByName(data, "Andromeda");

    // 根据赤经和赤纬以及范围查找数据
    auto result2 = SearchByRaDec(data, "05:22:00", "+45:00:00", 0.5, 0.5);

    // 将修改后的数据写回文件
    WriteToJson(data, "data.json");

    return 0;
}

[
    {
        "Id": 10001,
        "Name": "Orion Nebula",
        "Type": "Nebula",
        "RA": "05:35:17.3",
        "Dec": "-05:23:28",
        "Const": "Orion"
    },
    {
        "Id": 10002,
        "Name": "Pleiades",
        "Type": "Open Cluster",
        "RA": "03:47:24",
        "Dec": "+24:07:00",
        "Const": "Taurus"
    },
    {
        "Id": 10003,
        "Name": "Andromeda Galaxy",
        "Type": "Galaxy",
        "RA": "00:42:44.4",
        "Dec": "+41:16:09",
        "Const": "Andromeda"
    }
]

*/

namespace OpenAPT::CASX
{
    std::vector<Data> ReadFromCsv(const std::string& filename) {
        std::vector<Data> result;

        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filename);
        }

        std::string line;
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string cell;

            Data d;
            std::getline(ss, cell, ',');
            d.Id = std::stoi(cell);

            std::getline(ss, d.Name, ',');
            std::getline(ss, d.Type, ',');
            std::getline(ss, d.RA, ',');
            std::getline(ss, d.Dec, ',');
            std::getline(ss, d.Const, ',');

            result.push_back(d);
        }

        file.close();

        return result;
    }

    void SaveToCsv(const std::vector<Data>& data, const std::string& filename) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filename);
        }

        for (const auto& d : data) {
            file << d.Id << "," << d.Name << "," << d.Type << "," << d.RA << ","
                 << d.Dec << "," << d.Const << "\n";
        }

        file.close();
    }

    void SortByName(std::vector<Data>& data) {
        std::sort(data.begin(), data.end(), [](const Data& a, const Data& b) {
            return a.Name < b.Name;
        });
    }

    std::vector<Data> FilterBy(const std::vector<Data>& data, std::function<bool(const Data&)> filter) {
        std::vector<Data> result;

        std::copy_if(data.begin(), data.end(), std::back_inserter(result), filter);

        return result;
    }

    std::vector<Data> SearchByName(const std::vector<Data>& data, const std::string& name) {
        std::vector<Data> result;

        std::copy_if(data.begin(), data.end(), std::back_inserter(result), [&](const Data& d){
            return d.Name.find(name) != std::string::npos;
        });

        return result;
    }

    std::vector<Data> SearchByRaDec(const std::vector<Data>& data, std::string ra, std::string dec, double ra_range, double dec_range) {
        std::vector<Data> result;

        for (const auto& d : data) {
            // Convert RA and Dec to decimal degrees
            double d_ra = ToDecimal(d.RA);
            double d_dec = ToDecimal(d.Dec);
            double t_ra = ToDecimal(ra);
            double t_dec = ToDecimal(dec);

            // Calculate difference between target and current coordinates
            double delta_ra = std::abs(d_ra - t_ra);
            if (delta_ra > 180) delta_ra = 360 - delta_ra;
            double delta_dec = std::abs(d_dec - t_dec);

            // Check if current data entry's coordinates fall within search ranges
            if (delta_ra <= ra_range && delta_dec <= dec_range) {
                result.push_back(d);
            }
        }

        return result;
    }

    double ToDecimal(const std::string& str) {
        int sign = 1;

        if (str[0] == '-') {
            sign = -1;
        }

        std::stringstream ss(str.substr(1));

        double result = 0;
        double factor = 1;

        while (!ss.eof()) {
            std::string part;
            std::getline(ss, part, ':');

            if (part.empty()) {
                continue;
            }

            double value = std::stod(part);

            if (factor == 1) {
                result += value;
            } else if (factor == 60) {
                result += value / 60;
            } else if (factor == 3600) {
                result += value / 3600;
            }

            factor /= 60;
        }

        return sign * result * 15;
    }
}

/*
int main() {
    // Read data from CSV file
    std::vector<Data> data = OpenAPT::ASX::ReadFromCsv("stars.csv");

    // Sort data by name
    OpenAPT::ASX::SortByName(data);

    // Search for objects with names containing "M"
    std::vector<Data> search_results = OpenAPT::ASX::SearchByName(data, "M");

    // Filter objects to only include galaxies
    std::vector<Data> filtered_data = OpenAPT::ASX::FilterBy(data, [](const Data& d) {
        return d.Type == "Galaxy";
    });

    // Save data to new CSV file
    OpenAPT::ASX::SaveToCsv(filtered_data, "galaxies.csv");

    // Print out results
    std::cout << "All Objects:\n";
    for (const auto& d : data) {
        std::cout << d.Id << ", " << d.Name << ", " << d.Type << ", " << d.RA << ", " << d.Dec << ", " << d.Const << "\n";
    }

    std::cout << "\nSearch Results:\n";
    for (const auto& d : search_results) {
        std::cout << d.Id << ", " << d.Name << ", " << d.Type << ", " << d.RA << ", " << d.Dec << ", " << d.Const << "\n";
    }

    std::cout << "\nFiltered Data:\n";
    for (const auto& d : filtered_data) {
        std::cout << d.Id << ", " << d.Name << ", " << d.Type << ", " << d.RA << ", " << d.Dec << ", " << d.Const << "\n";
    }

    return 0;
}

1,Sirius,Star,06:45:08,-16:42:58.0,Canis Major
2,Vega,Star,18:36:56.33635,+38:47:01.2802,Lyra
3,Alpha Centauri,Star,14:29:42.9474,-60:49:57.236,Apus
4,M31,Galaxy,00:42:44,+41:16:09,Andromeda
5,M104,Galaxy,12:39:59,-11:37:23.37,Virgo

*/