/*
 * ongc.cpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-7-13

Description: C++ Version of PyOngc

**************************************************/

#include "ongc.hpp"

#include <iostream>
#include <cmath>
#include <regex>
#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif
#include <sqlite3.h>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

#define DBPATH "ognc.db"

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

double rad2deg(const double &radians)
{
    const double conversionFactor = 180.0 / M_PI;
    return radians * conversionFactor;
}

template <int i>
std::vector<std::string> get_identifiers_helper(const std::tuple<std::string, std::vector<std::string>, std::vector<std::string>, std::vector<std::string>, std::vector<std::string>> &identifiers)
{
    return std::get<i>(identifiers);
}

template <>
std::vector<std::string> get_identifiers_helper<0>(const std::tuple<std::string, std::vector<std::string>, std::vector<std::string>, std::vector<std::string>, std::vector<std::string>> &identifiers)
{
    return {std::get<0>(identifiers)};
}

Dso::Dso(const std::string &name, bool returndup = false)
{
    if (name.empty())
    {
        throw std::invalid_argument("Name parameter cannot be empty.");
    }

    // Initialize object properties
    _id = 0;
    _ra = 0.0;
    _dec = 0.0;
    _majax = 0.0;
    _minax = 0.0;
    _pa = 0;
    _bmag = 0.0;
    _vmag = 0.0;
    _jmag = 0.0;
    _hmag = 0.0;
    _kmag = 0.0;
    _sbrightn = 0.0;
    _parallax = 0.0;
    _pmra = 0.0;
    _pmdec = 0.0;
    _radvel = 0.0;
    _redshift = 0.0;
    _cstarumag = 0.0;
    _cstarbmag = 0.0;
    _cstarvmag = 0.0;

    std::string catalog, objectname;
    std::string tables = "objects JOIN objTypes ON objects.type = objTypes.type ";
    tables += "JOIN objIdentifiers ON objects.name = objIdentifiers.name";
    recognize_name(name, catalog, objectname);
    std::stringstream params;

    if (catalog == "Messier")
    {
        params << "messier=\"" << objectname << "\"";
    }
    else
    {
        params << "objIdentifiers.identifier=\"" << objectname << "\"";
    }

    // Perform query and retrieve object data
    std::vector<std::string> objectData = _queryFetchOne(catalog, tables, params.str());

    if (objectData.empty())
    {
        throw std::runtime_error("Object not found: " + objectname);
    }

    // If object is a duplicate and returndup is false, get the main object data
    if (objectData[2] == "Dup" && !returndup)
    {
        if (!objectData[26].empty())
        {
            objectname = "NGC" + objectData[26];
        }
        else
        {
            objectname = "IC" + objectData[27];
        }
        objectData = _queryFetchOne(catalog, tables, params.str());
    }

    // Assign object properties
    _id = std::stoi(objectData[0]);
    _name = objectData[1];
    _type = objectData[3];
    _ra = std::stod(objectData[4]);
    _dec = std::stod(objectData[5]);
    _const = objectData[6];
    _notngc = objectData[33];

    // Assign optional properties
    if (!objectData[7].empty())
    {
        _majax = std::stod(objectData[7]);
    }
    if (!objectData[8].empty())
    {
        _minax = std::stod(objectData[8]);
    }
    if (!objectData[9].empty())
    {
        _pa = std::stoi(objectData[9]);
    }
    if (!objectData[10].empty())
    {
        _bmag = std::stod(objectData[10]);
    }
    if (!objectData[11].empty())
    {
        _vmag = std::stod(objectData[11]);
    }
    if (!objectData[12].empty())
    {
        _jmag = std::stod(objectData[12]);
    }
    if (!objectData[13].empty())
    {
        _hmag = std::stod(objectData[13]);
    }
    if (!objectData[14].empty())
    {
        _kmag = std::stod(objectData[14]);
    }
    if (!objectData[15].empty())
    {
        _sbrightn = std::stod(objectData[15]);
    }
    _hubble = objectData[16];
    if (!objectData[17].empty())
    {
        _parallax = std::stod(objectData[17]);
    }
    if (!objectData[18].empty())
    {
        _pmra = std::stod(objectData[18]);
    }
    if (!objectData[19].empty())
    {
        _pmdec = std::stod(objectData[19]);
    }
    if (!objectData[20].empty())
    {
        _radvel = std::stod(objectData[20]);
    }
    if (!objectData[21].empty())
    {
        _redshift = std::stod(objectData[21]);
    }
    if (!objectData[22].empty())
    {
        _cstarumag = std::stod(objectData[22]);
    }
    if (!objectData[23].empty())
    {
        _cstarbmag = std::stod(objectData[23]);
    }
    if (!objectData[24].empty())
    {
        _cstarvmag = std::stod(objectData[24]);
    }
    _messier = objectData[25];
    _ngc = objectData[26];
    _ic = objectData[27];
    _cstarnames = objectData[28];
    _identifiers = objectData[29];
    _commonnames = objectData[30];
    _nednotes = objectData[31];
    _ongcnotes = objectData[32];
}

std::string Dso::to_string() const
{
    return _name + ", " + _type + " in " + _const;
}

std::string Dso::get_constellation() const
{
    return _const;
}

std::vector<double> Dso::get_coords() const
{
    std::vector<double> coords;
    coords.push_back(std::trunc(rad2deg(_ra) / 15));
    double ms = (rad2deg(_ra) / 15 - coords[0]) * 60;
    coords.push_back(std::trunc(ms));
    coords.push_back((ms - coords[1]) * 60);

    double dec = std::trunc(rad2deg(std::abs(_dec)));
    ms = (rad2deg(std::abs(_dec)) - dec) * 60;
    coords.push_back(dec * (_dec < 0 ? -1 : 1));
    coords.push_back(std::trunc(ms));
    coords.push_back((ms - coords[4]) * 60);

    return coords;
}

std::string Dso::get_dec() const
{
    std::vector<double> coords = get_coords();
    return std::to_string(coords[3]) + ":" + std::to_string(coords[4]) + ":" + std::to_string(coords[5]);
}

std::vector<double> Dso::get_dimensions() const
{
    return {_majax, _minax, static_cast<double>(_pa)};
}

std::string Dso::get_hubble() const
{
    return _hubble;
}

int Dso::get_id() const
{
    return _id;
}

std::tuple<std::string, std::vector<std::string>, std::vector<std::string>, std::vector<std::string>, std::vector<std::string>> Dso::get_identifiers() const
{
    std::string messier = (!_messier.empty()) ? "M" + _messier : "";
    std::vector<std::string> ngc, ic, commonNames, other;

    if (!_ngc.empty())
    {
        for (const auto &number : split(_ngc, ','))
        {
            ngc.push_back("NGC" + number);
        }
    }

    if (!_ic.empty())
    {
        for (const auto &number : split(_ic, ','))
        {
            ic.push_back("IC" + number);
        }
    }

    if (!_commonnames.empty())
    {
        commonNames = split(_commonnames, ',');
    }

    if (!_identifiers.empty())
    {
        other = split(_identifiers, ',');
    }

    return std::make_tuple(messier, ngc, ic, commonNames, other);
}

void Dso::recognize_name(const std::string &name, std::string &catalog, std::string &objectname) const
{
    const std::unordered_map<std::string, std::string> patterns = {
        {"NGC|IC", R"(^((?:NGC|IC)\s?)(\d{1,4})\s?((NED)(\d{1,2})|[A-Z]{1,2})?$)"},
        {"Messier", R"(^(M\s?)(\d{1,3})$)"},
        {"Barnard", R"(^(B\s?)(\d{1,3})$)"},
        {"Caldwell", R"(^(C\s?)(\d{1,3})$)"},
        {"Collinder", R"(^(CL\s?)(\d{1,3})$)"},
        {"ESO", R"(^(ESO\s?)(\d{1,3})-(\d{1,3})$)"},
        {"Harvard", R"(^(H\s?)(\d{1,2})$)"},
        {"Hickson", R"(^(HCG\s?)(\d{1,3})$)"},
        {"LBN", R"(^(LBN\s?)(\d{1,3})$)"},
        {"Melotte", R"(^(MEL\s?)(\d{1,3})$)"},
        {"MWSC", R"(^(MWSC\s?)(\d{1,4})$)"},
        {"PGC", R"(^((?:PGC|LEDA)\s?)(\d{1,6})$)"},
        {"UGC", R"(^(UGC\s?)(\d{1,5})$)"}};

    for (const auto &pattern : patterns)
    {
        std::regex regex(pattern.second);
        std::smatch match;
        if (std::regex_search(name, match, regex))
        {
            catalog = pattern.first;
            objectname = match.str();
            break;
        }
    }
}

std::vector<std::string> Dso::_queryFetchOne(const std::string &cols, const std::string &tables, const std::string &params)
{
    sqlite3 *db;
    int rc = sqlite3_open_v2(DBPATH, &db, SQLITE_OPEN_READONLY, nullptr);
    if (rc != SQLITE_OK)
    {
        throw std::runtime_error("There was a problem accessing the database file.");
    }

    std::vector<std::string> objectData;

    try
    {
        sqlite3_stmt *stmt;
        std::string query = "SELECT " + cols + " FROM " + tables + " WHERE " + params;
        rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK)
        {
            throw std::runtime_error("Failed to prepare the SQL statement.");
        }

        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW)
        {
            int numColumns = sqlite3_column_count(stmt);

            for (int i = 0; i < numColumns; ++i)
            {
                std::string colData = reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));
                objectData.push_back(colData);
            }
        }

        sqlite3_finalize(stmt);
    }
    catch (const std::exception &e)
    {
        sqlite3_close(db);
        throw e;
    }

    sqlite3_close(db);
    return objectData;
}

std::vector<std::string> Dso::split(const std::string &input, char delimiter) const
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(input);
    while (std::getline(tokenStream, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}

std::string join(const std::vector<std::string> &strings, const std::string &delimiter) const
{
    std::string result;
    if (!strings.empty())
    {
        result += strings[0];
        for (size_t i = 1; i < strings.size(); ++i)
        {
            result += delimiter + strings[i];
        }
    }
    return result;
}

/*

int main()
{
    try
    {
        Dso dso("NGC 1234");
        std::cout << dso.to_string() << std::endl;
        std::cout << "Constellation: " << dso.get_constellation() << std::endl;
        std::cout << "Coordinates (RA, Dec): " << dso.get_coords()[0] << ", " << dso.get_coords()[1] << ", " << dso.get_coords()[2] << ", " << dso.get_coords()[3] << ", " << dso.get_coords()[4] << ", " << dso.get_coords()[5] << std::endl;
        std::cout << "Declination: " << dso.get_dec() << std::endl;
        std::cout << "Dimensions (Majax, Minax, PA): " << dso.get_dimensions()[0] << ", " << dso.get_dimensions()[1] << ", " << dso.get_dimensions()[2] << std::endl;
        std::cout << "Hubble Type: " << dso.get_hubble() << std::endl;
        std::cout << "Object ID: " << dso.get_id() << std::endl;
        auto identifiers = dso.get_identifiers();
        std::cout << "Messier: " << std::get<0>(identifiers) << std::endl;
        std::cout << "NGC: ";
        for (const auto &ngc : std::get<1>(identifiers))
        {
            std::cout << ngc << " ";
        }
        std::cout << std::endl;
        std::cout << "IC: ";
        for (const auto &ic : std::get<2>(identifiers))
        {
            std::cout << ic << " ";
        }
        std::cout << std::endl;
        std::cout << "Common Names: ";
        for (const auto &commonName : std::get<3>(identifiers))
        {
            std::cout << commonName << " ";
        }
        std::cout << std::endl;
        std::cout << "Other Identifiers: ";
        for (const auto &identifier : std::get<4>(identifiers))
        {
            std::cout << identifier << " ";
        }
        std::cout << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cout << "Error: " << e.what() << std::endl;
    }
    return 0;
}

*/