/*
 * createdb.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-13

Description: A simple database creation tool

**************************************************/

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <regex>
#include <cmath>
#include <sqlite3.h>

#include "object.hpp"

#include "atom/log/loguru.hpp"

std::vector<std::string> splitString(std::string input, const std::string &delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    size_t delimiterPos = input.find(delimiter);

    while (delimiterPos != std::string::npos)
    {
        token = input.substr(0, delimiterPos);
        tokens.push_back(token);
        input.erase(0, delimiterPos + delimiter.length() - 1);
        delimiterPos = input.find(delimiter);
    }

    tokens.push_back(input);

    return tokens;
}

std::string padNumber(const std::string &input, int length)
{
    std::stringstream ss;
    ss << std::setw(length) << std::setfill('0') << input;
    return ss.str();
}

const std::unordered_map<std::string, std::string> objectTypes = {
    {"*", "Star"},
    {"**", "Double star"},
    {"*Ass", "Association of stars"},
    {"OCl", "Open Cluster"},
    {"GCl", "Globular Cluster"},
    {"Cl+N", "Star cluster + Nebula"},
    {"G", "Galaxy"},
    {"GPair", "Galaxy Pair"},
    {"GTrpl", "Galaxy Triplet"},
    {"GGroup", "Group of galaxies"},
    {"PN", "Planetary Nebula"},
    {"HII", "HII Ionized region"},
    {"DrkN", "Dark Nebula"},
    {"EmN", "Emission Nebula"},
    {"Neb", "Nebula"},
    {"RfN", "Reflection Nebula"},
    {"SNR", "Supernova remnant"},
    {"Nova", "Nova star"},
    {"NonEx", "Nonexistent object"},
    {"Other", "Object of other/unknown type"},
    {"Dup", "Duplicated record"}};

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

void openDatabase(sqlite3 **db, const std::string &dbFile)
{
    int rc = sqlite3_open(dbFile.c_str(), db);
    if (rc != SQLITE_OK)
    {
        LOG_F(ERROR, "Cannot open database: {}", std::string(sqlite3_errmsg(db)));
        throw std::runtime_error("Cannot open database: " + std::string(sqlite3_errmsg(*db)));
    }
    LOG_F(INFO, "Opened database: {}", dbFile);
}

void beginTransaction(sqlite3 *db)
{
    char *errMsg;
    int rc = sqlite3_exec(db, "BEGIN", nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK)
    {
        LOG_F(ERROR, "Failed to begin transaction: {}", std::string(errMsg));
        throw std::runtime_error("Failed to begin transaction: " + std::string(errMsg));
    }
    LOG_F(INFO, "Begin transaction");
}

void createObjTypesTable(sqlite3 *db)
{
    char *errMsg;
    int rc = sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS objTypes("
                              "type TEXT PRIMARY KEY NOT NULL, "
                              "typedesc TEXT NOT NULL)",
                          nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK)
    {
        LOG_F(ERROR, "Failed to create objTypes table: {}", std::string(errMsg));
        throw std::runtime_error("Failed to create objTypes table: " + std::string(errMsg));
    }
    LOG_F(INFO, "Created objTypes table");
}

void insertObjectTypes(sqlite3 *db)
{
    std::string insertObjectTypes = "INSERT INTO objTypes VALUES(?, ?)";
    char *errMsg;
    for (const auto &objType : objectTypes)
    {
        sqlite3_stmt *stmt;
        int rc = sqlite3_prepare_v2(db, insertObjectTypes.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK)
        {
            sqlite3_finalize(stmt);
            LOG_F(ERROR, "Failed to prepare statement: {}", std::string(sqlite3_errmsg(db)));
            throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
        }

        rc = sqlite3_bind_text(stmt, 1, objType.first.c_str(), -1, SQLITE_STATIC);
        if (rc != SQLITE_OK)
        {
            sqlite3_finalize(stmt);
            LOG_F(ERROR, "Failed to bind parameter: {}", std::string(sqlite3_errmsg(db)));
            throw std::runtime_error("Failed to bind parameter: " + std::string(sqlite3_errmsg(db)));
        }

        rc = sqlite3_bind_text(stmt, 2, objType.second.c_str(), -1, SQLITE_STATIC);
        if (rc != SQLITE_OK)
        {
            sqlite3_finalize(stmt);
            LOG_F(ERROR, "Failed to bind parameter: {}", std::string(sqlite3_errmsg(db)));
            throw std::runtime_error("Failed to bind parameter: " + std::string(sqlite3_errmsg(db)));
        }

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE)
        {
            sqlite3_finalize(stmt);
            LOG_F(ERROR, "Failed to insert object type: {}", std::string(sqlite3_errmsg(db)));
            throw std::runtime_error("Failed to insert object type: " + std::string(sqlite3_errmsg(db)));
        }
        sqlite3_finalize(stmt);
        LOG_F(INFO, "Inserted object type: {}", objType.first);
    }
    LOG_F(INFO, "Inserted object types");
}

void createObjTypesTable()
{
    char *errMsg;
    int rc = sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS objects("
                              "id INTEGER PRIMARY KEY NOT NULL, "
                              "name TEXT NOT NULL UNIQUE, "
                              "type TEXT NOT NULL, "
                              "ra REAL, "
                              "dec REAL, "
                              "const TEXT, "
                              "majax REAL, "
                              "minax REAL, "
                              "pa INTEGER, "
                              "bmag REAL, "
                              "vmag REAL, "
                              "jmag REAL, "
                              "hmag REAL, "
                              "kmag REAL, "
                              "sbrightn REAL, "
                              "hubble TEXT, "
                              "parallax REAL, "
                              "pmra REAL, "
                              "pmdec REAL, "
                              "radvel INTEGER, "
                              "redshift REAL, "
                              "cstarumag REAL, "
                              "cstarbmag REAL, "
                              "cstarvmag REAL, "
                              "messier TEXT, "
                              "ngc TEXT, "
                              "ic TEXT, "
                              "cstarnames TEXT, "
                              "identifiers TEXT, "
                              "commonnames TEXT, "
                              "nednotes TEXT, "
                              "ongcnotes TEXT, "
                              "notngc BOOL DEFAULT FALSE)",
                          nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK)
    {
        LOG_F(ERROR, "Failed to create objects table: {}", std::string(errMsg));
        throw std::runtime_error("Failed to create objects table: " + std::string(errMsg));
    }
    LOG_F(INFO, "Created objects table");
}

void createObjectIdentifiersTable(sqlite3 *db)
{
    char *errMsg;
    int rc = sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS objIdentifiers("
                              "id INTEGER PRIMARY KEY NOT NULL, "
                              "name TEXT NOT NULL, "
                              "identifier TEXT NOT NULL UNIQUE)",
                          nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK)
    {
        LOG_F(ERROR, "Failed to create objIdentifiers table: {}", std::string(errMsg));
        throw std::runtime_error("Failed to create objIdentifiers table: " + std::string(errMsg));
    }
    LOG_F(INFO, "Created objIdentifiers table");
}

void bindParameter(sqlite3_stmt *stmt, int index, const std::string &value)
{
    int rc = sqlite3_bind_text(stmt, index, value.c_str(), -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        LOG_F(ERROR, "Failed to bind parameter: {}", std::string(sqlite3_errmsg(sqlite3_db_handle(stmt))));
        throw std::runtime_error("Failed to bind parameter: " + std::string(sqlite3_errmsg(sqlite3_db_handle(stmt))));
    }
    LOG_F(INFO, "Bound parameter: {}", value);
}

void bindParameter(sqlite3_stmt *stmt, int index, double value)
{
    int rc = sqlite3_bind_double(stmt, index, value);
    if (rc != SQLITE_OK)
    {
        LOG_F(ERROR, "Failed to bind parameter: {}", std::string(sqlite3_errmsg(sqlite3_db_handle(stmt))));
        throw std::runtime_error("Failed to bind parameter: " + std::string(sqlite3_errmsg(sqlite3_db_handle(stmt))));
    }
    LOG_F(INFO, "Bound parameter: {}", value);
}

void createDatabase(const std::string &dbFile)
{
    sqlite3 *db;
    int rc;

    openDatabase(&db, dbFile);
    beginTransaction(db);
    createObjTypesTable(db);
    insertObjectTypes(db);

    // Create objects table
    createObjTypesTable(db);

    // Create object identifiers table
    createObjectIdentifiersTable(db);

    std::regex emptyColumnRegex(R"(^$)");

    std::vector<std::string> filenames = {"NGC.csv", "addendum.csv"};
    for (const auto &filename : filenames)
    {
        bool notNGC = (filename != "NGC.csv");

        std::ifstream file(filename);
        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open file: " + filename);
        }

        std::string line;
        std::getline(file, line); // Skip header line

        while (std::getline(file, line))
        {
            std::istringstream iss(line);
            std::string column;
            Object object;

            int columnIndex = 0;
            while (std::getline(iss, column, ';'))
            {
                if (columnIndex == 0)
                {
                    object.name = column;
                }
                else if (columnIndex == 1)
                {
                    object.type = column;
                }
                else if (columnIndex == 2)
                {
                    object.ra = (column.empty()) ? 0.0 : std::stod(column);
                }
                else if (columnIndex == 3)
                {
                    object.dec = (column.empty()) ? 0.0 : std::stod(column);
                }
                else if (columnIndex == 4)
                {
                    object.constellation = column;
                }
                else if (columnIndex == 5)
                {
                    object.majorAxis = (column.empty()) ? 0.0 : std::stod(column);
                }
                else if (columnIndex == 6)
                {
                    object.minorAxis = (column.empty()) ? 0.0 : std::stod(column);
                }
                else if (columnIndex == 7)
                {
                    object.positionAngle = (column.empty()) ? 0 : std::stoi(column);
                }
                else if (columnIndex == 8)
                {
                    object.bMagnitude = (column.empty()) ? 0.0 : std::stod(column);
                }
                else if (columnIndex == 9)
                {
                    object.vMagnitude = (column.empty()) ? 0.0 : std::stod(column);
                }
                else if (columnIndex == 10)
                {
                    object.jMagnitude = (column.empty()) ? 0.0 : std::stod(column);
                }
                else if (columnIndex == 11)
                {
                    object.hMagnitude = (column.empty()) ? 0.0 : std::stod(column);
                }
                else if (columnIndex == 12)
                {
                    object.kMagnitude = (column.empty()) ? 0.0 : std::stod(column);
                }
                else if (columnIndex == 13)
                {
                    object.surfaceBrightness = (column.empty()) ? 0.0 : std::stod(column);
                }
                else if (columnIndex == 14)
                {
                    object.hubbleType = column;
                }
                else if (columnIndex == 15)
                {
                    object.parallax = (column.empty()) ? 0.0 : std::stod(column);
                }
                else if (columnIndex == 16)
                {
                    object.properMotionRA = (column.empty()) ? 0.0 : std::stod(column);
                }
                else if (columnIndex == 17)
                {
                    object.properMotionDec = (column.empty()) ? 0.0 : std::stod(column);
                }
                else if (columnIndex == 18)
                {
                    object.radialVelocity = (column.empty()) ? 0 : std::stoi(column);
                }
                else if (columnIndex == 19)
                {
                    object.redshift = (column.empty()) ? 0.0 : std::stod(column);
                }
                else if (columnIndex == 20)
                {
                    object.cstarUMagnitude = (column.empty()) ? 0.0 : std::stod(column);
                }
                else if (columnIndex == 21)
                {
                    object.cstarBMagnitude = (column.empty()) ? 0.0 : std::stod(column);
                }
                else if (columnIndex == 22)
                {
                    object.cstarVMagnitude = (column.empty()) ? 0.0 : std::stod(column);
                }
                else if (columnIndex == 23)
                {
                    object.messier = column;
                }
                else if (columnIndex == 24)
                {
                    object.ngc = column;
                }
                else if (columnIndex == 25)
                {
                    object.ic = column;
                }
                else if (columnIndex == 26)
                {
                    object.cstarNames = column;
                }
                else if (columnIndex == 27)
                {
                    object.identifiers = column;
                }
                else if (columnIndex == 28)
                {
                    object.commonNames = column;
                }
                else if (columnIndex == 29)
                {
                    object.nedNotes = column;
                }
                else if (columnIndex == 30)
                {
                    object.ongcNotes = column;
                }
                else if (columnIndex == 31)
                {
                    object.notNGC = (column.empty()) ? false : std::stoi(column);
                }
                columnIndex++;
            }

            std::string insertObjectQuery = "INSERT INTO objects(name, type, ra, dec, const, majax, minax, pa, bmag, vmag, jmag, hmag, kmag, sb, htype, parallax, pmra, pmdec, rv, z, cstarumag, cstarbmag, cstarvmag, messier, ngc, ic, cstarnames, identifiers, commonnames, nednotes, ongcnotes, notngc)"
                                            "VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
            sqlite3_stmt *stmt;
            rc = sqlite3_prepare_v2(db, insertObjectQuery.c_str(), -1, &stmt, nullptr);
            if (rc != SQLITE_OK)
            {
                throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
            }

            bindParameter(stmt, 1, object.name);
            bindParameter(stmt, 2, object.type);
            bindParameter(stmt, 3, object.ra);
            bindParameter(stmt, 4, object.dec);
            bindParameter(stmt, 5, object.constellation);
            bindParameter(stmt, 6, object.majorAxis);
            bindParameter(stmt, 7, object.minorAxis);
            bindParameter(stmt, 8, object.positionAngle);
            bindParameter(stmt, 9, object.bMagnitude);
            bindParameter(stmt, 10, object.vMagnitude);
            bindParameter(stmt, 11, object.jMagnitude);
            bindParameter(stmt, 12, object.hMagnitude);
            bindParameter(stmt, 13, object.kMagnitude);
            bindParameter(stmt, 14, object.surfaceBrightness);
            bindParameter(stmt, 15, object.hubbleType);
            bindParameter(stmt, 16, object.parallax);
            bindParameter(stmt, 17, object.properMotionRA);
            bindParameter(stmt, 18, object.properMotionDec);
            bindParameter(stmt, 19, object.radialVelocity);
            bindParameter(stmt, 20, object.redshift);
            bindParameter(stmt, 21, object.cstarUMagnitude);
            bindParameter(stmt, 22, object.cstarVMagnitude);
            bindParameter(stmt, 23, object.cstarBMagnitude);
            bindParameter(stmt, 24, object.messier);
            bindParameter(stmt, 25, object.ngc);
            bindParameter(stmt, 26, object.ic);
            bindParameter(stmt, 27, object.cstarNames);
            bindParameter(stmt, 28, object.identifiers);
            bindParameter(stmt, 29, object.commonNames);
            bindParameter(stmt, 30, object.nedNotes);
            bindParameter(stmt, 31, object.ongcNotes);
            bindParameter(stmt, 32, object.notNGC);

            rc = sqlite3_step(stmt);
            if (rc != SQLITE_DONE)
            {
                sqlite3_finalize(stmt);
                LOG_F(ERROR, "Failed to insert object data: {}", std::string(sqlite3_errmsg(db)))
                throw std::runtime_error("Failed to insert object data: " + std::string(sqlite3_errmsg(db)));
            }

            // Insert data into objIdentifiers table
            std::string insertIdentifierQuery = "INSERT INTO objIdentifiers(name, identifier) VALUES(?, ?)";
            sqlite3_stmt *stmt2;
            rc = sqlite3_prepare_v2(db, insertIdentifierQuery.c_str(), -1, &stmt2, nullptr);
            if (rc != SQLITE_OK)
            {
                sqlite3_finalize(stmt);
                LOG_F(ERROR, "Failed to prepare statement: {}", std::string(sqlite3_errmsg(db)))
                throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
            }

            rc = sqlite3_bind_text(stmt2, 1, object.name.c_str(), -1, SQLITE_STATIC);
            if (rc != SQLITE_OK)
            {
                sqlite3_finalize(stmt2);
                LOG_F(ERROR, "Failed to bind parameter: {}", std::string(sqlite3_errmsg(db)))
                throw std::runtime_error("Failed to bind parameter: " + std::string(sqlite3_errmsg(db)));
            }

            rc = sqlite3_bind_text(stmt2, 2, object.name.c_str(), -1, SQLITE_STATIC);
            if (rc != SQLITE_OK)
            {
                sqlite3_finalize(stmt2);
                LOG_F(ERROR, "Failed to bind parameter: {}", std::string(sqlite3_errmsg(db)))
                throw std::runtime_error("Failed to bind parameter: " + std::string(sqlite3_errmsg(db)));
            }

            rc = sqlite3_step(stmt2);
            if (rc != SQLITE_DONE)
            {
                sqlite3_finalize(stmt2);
                LOG_F(ERROR, "Failed to insert identifier data: {}", std::string(sqlite3_errmsg(db)))
                throw std::runtime_error("Failed to insert identifier data: " + std::string(sqlite3_errmsg(db)));
            }

            sqlite3_finalize(stmt);
            sqlite3_finalize(stmt2);

            // Handle identifiers
            std::vector<std::string> identifiers = splitString(object.identifiers, ",");
            for (const auto &identifier : identifiers)
            {
                for (const auto &pattern : patterns)
                {
                    std::smatch match;
                    if (std::regex_search(identifier, match, std::regex(pattern.second)))
                    {
                        // Handle matched identifier
                        std::string objectName;
                        if (pattern.first == "NGC|IC")
                        {
                            if (match[3].matched)
                            {
                                if (match[4].matched)
                                {
                                    objectName = match[1].str() + padNumber(match[2].str(), 4) + " " +
                                                 match[4].str() + padNumber(match[5].str(), 2);
                                }
                                else
                                {
                                    objectName = match[1].str() + padNumber(match[2].str(), 4) +
                                                 match[3].str();
                                }
                            }
                        }
                        else if (pattern.first == "MWSC")
                        {
                            objectName = match[1].str() + padNumber(match[2].str(), 4);
                        }
                        else if (pattern.first == "C*")
                        {
                            objectName = match[1].str() + padNumber(match[2].str(), 4) + " " +
                                         match[3].str() + padNumber(match[4].str(), 2);
                        }
                        else if (pattern.first == "Other")
                        {
                            objectName = match[1].str() + padNumber(match[2].str(), 4);
                        }

                        // Insert data into objIdentifiers table
                        std::string insertIdentifierQuery = "INSERT INTO objIdentifiers(name, identifier) VALUES(?, ?)";
                        sqlite3_stmt *stmt2;
                        rc = sqlite3_prepare_v2(db, insertIdentifierQuery.c_str(), -1, &stmt2, nullptr);
                        if (rc != SQLITE_OK)
                        {
                            throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
                        }

                        rc = sqlite3_bind_text(stmt2, 1, object.name.c_str(), -1, SQLITE_STATIC);
                        if (rc != SQLITE_OK)
                        {
                            sqlite3_finalize(stmt2);
                            throw std::runtime_error("Failed to bind parameter: " + std::string(sqlite3_errmsg(db)));
                        }

                        rc = sqlite3_bind_text(stmt2, 2, objectName.c_str(), -1, SQLITE_STATIC);
                        if (rc != SQLITE_OK)
                        {
                            sqlite3_finalize(stmt2);
                            throw std::runtime_error("Failed to bind parameter: " + std::string(sqlite3_errmsg(db)));
                        }

                        rc = sqlite3_step(stmt2);
                        if (rc != SQLITE_DONE)
                        {
                            sqlite3_finalize(stmt2);
                            throw std::runtime_error("Failed to insert identifier data: " + std::string(sqlite3_errmsg(db)));
                        }

                        sqlite3_finalize(stmt2);
                    }
                }
            }
        }
    }

    rc = sqlite3_exec(db, "CREATE UNIQUE INDEX \"idx_identifiers\" ON \"objIdentifiers\" (\"identifier\")", nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK)
    {
        LOG_F(ERROR, "Failed to create index: {}", std::string(errMsg));
        throw std::runtime_error("Failed to create index: " + std::string(errMsg));
    }

    rc = sqlite3_exec(db, "COMMIT", nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK)
    {
        LOG_F(ERROR, "Failed to commit transaction: {}", std::string(errMsg));
        throw std::runtime_error("Failed to commit transaction: " + std::string(errMsg));
    }

    sqlite3_close(db);
}

void setupLogFile()
{
    std::filesystem::path logsFolder = std::filesystem::current_path() / "logs";
    if (!std::filesystem::exists(logsFolder))
    {
        std::filesystem::create_directory(logsFolder);
    }
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm *local_time = std::localtime(&now_time_t);
    char filename[100];
    std::strftime(filename, sizeof(filename), "%Y%m%d_%H%M%S.log", local_time);
    std::filesystem::path logFilePath = logsFolder / filename;
    loguru::add_file(logFilePath.string().c_str(), loguru::Append, loguru::Verbosity_MAX);

    loguru::set_fatal_handler([](const loguru::Message &message)
                              { loguru::shutdown(); });
}

int main()
{
    try
    {
        setupLogFile();
        createDatabase("ongc.db");
        LOG_F(INFO, "Database created successfully!");
        return 0;
    }
    catch (const std::exception &e)
    {
        LOG_F(ERROR, "Failed to create database: {}", e.what());
        return 1;
    }
}
