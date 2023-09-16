#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <regex>
#include <cmath>
#include <sqlite3.h>

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

// 将数字字符串填充至指定长度
std::string padNumber(const std::string &input, int length)
{
    std::stringstream ss;
    ss << std::setw(length) << std::setfill('0') << input;
    return ss.str();
}

struct Object
{
    std::string name;
    std::string type;
    double ra;
    double dec;
    std::string constellation;
    double majorAxis;
    double minorAxis;
    int positionAngle;
    double bMagnitude;
    double vMagnitude;
    double jMagnitude;
    double hMagnitude;
    double kMagnitude;
    double surfaceBrightness;
    std::string hubbleType;
    double parallax;
    double properMotionRA;
    double properMotionDec;
    int radialVelocity;
    double redshift;
    double cstarUMagnitude;
    double cstarBMagnitude;
    double cstarVMagnitude;
    std::string messier;
    std::string ngc;
    std::string ic;
    std::string cstarNames;
    std::string identifiers;
    std::string commonNames;
    std::string nedNotes;
    std::string ongcNotes;
    bool notNGC;
};

const std::map<std::string, std::string> objectTypes = {
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

const std::map<std::string, std::string> patterns = {
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

void createDatabase(const std::string &dbFile)
{
    sqlite3 *db;
    int rc;

    rc = sqlite3_open(dbFile.c_str(), &db);
    if (rc != SQLITE_OK)
    {
        throw std::runtime_error("Cannot open database: " + std::string(sqlite3_errmsg(db)));
    }

    char *errMsg;
    rc = sqlite3_exec(db, "BEGIN", nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK)
    {
        throw std::runtime_error("Failed to begin transaction: " + std::string(errMsg));
    }

    // Create object types table
    rc = sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS objTypes("
                          "type TEXT PRIMARY KEY NOT NULL, "
                          "typedesc TEXT NOT NULL)",
                      nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK)
    {
        throw std::runtime_error("Failed to create objTypes table: " + std::string(errMsg));
    }

    // Insert object types
    std::string insertObjectTypes = "INSERT INTO objTypes VALUES(?, ?)";
    for (const auto &objType : objectTypes)
    {
        sqlite3_stmt *stmt;
        rc = sqlite3_prepare_v2(db, insertObjectTypes.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK)
        {
            throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
        }

        rc = sqlite3_bind_text(stmt, 1, objType.first.c_str(), -1, SQLITE_STATIC);
        if (rc != SQLITE_OK)
        {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Failed to bind parameter: " + std::string(sqlite3_errmsg(db)));
        }

        rc = sqlite3_bind_text(stmt, 2, objType.second.c_str(), -1, SQLITE_STATIC);
        if (rc != SQLITE_OK)
        {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Failed to bind parameter: " + std::string(sqlite3_errmsg(db)));
        }

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE)
        {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Failed to insert object type: " + std::string(sqlite3_errmsg(db)));
        }

        sqlite3_finalize(stmt);
    }

    // Create objects table
    rc = sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS objects("
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
        throw std::runtime_error("Failed to create objects table: " + std::string(errMsg));
    }

    // Create object identifiers table
    rc = sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS objIdentifiers("
                          "id INTEGER PRIMARY KEY NOT NULL, "
                          "name TEXT NOT NULL, "
                          "identifier TEXT NOT NULL UNIQUE)",
                      nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK)
    {
        throw std::runtime_error("Failed to create objIdentifiers table: " + std::string(errMsg));
    }

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
                // Add other column assignments...

                columnIndex++;
            }

            // Insert data into objects table
            std::string insertObjectQuery = "INSERT INTO objects(name, type, ra, dec, const, majax, minax, ...)"
                                            "VALUES(?, ?, ?, ?, ?, ?, ?, ...)";
            sqlite3_stmt *stmt;
            rc = sqlite3_prepare_v2(db, insertObjectQuery.c_str(), -1, &stmt, nullptr);
            if (rc != SQLITE_OK)
            {
                throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
            }

            rc = sqlite3_bind_text(stmt, 1, object.name.c_str(), -1, SQLITE_STATIC);
            if (rc != SQLITE_OK)
            {
                sqlite3_finalize(stmt);
                throw std::runtime_error("Failed to bind parameter: " + std::string(sqlite3_errmsg(db)));
            }

            rc = sqlite3_bind_text(stmt, 2, object.type.c_str(), -1, SQLITE_STATIC);
            if (rc != SQLITE_OK)
            {
                sqlite3_finalize(stmt);
                throw std::runtime_error("Failed to bind parameter: " + std::string(sqlite3_errmsg(db)));
            }

            rc = sqlite3_bind_double(stmt, 3, object.ra);
            // Bind other parameters...

            rc = sqlite3_step(stmt);
            if (rc != SQLITE_DONE)
            {
                sqlite3_finalize(stmt);
                throw std::runtime_error("Failed to insert object data: " + std::string(sqlite3_errmsg(db)));
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

            rc = sqlite3_bind_text(stmt2, 2, object.name.c_str(), -1, SQLITE_STATIC);
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
                        // Handle other patterns...

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
        throw std::runtime_error("Failed to create index: " + std::string(errMsg));
    }

    rc = sqlite3_exec(db, "COMMIT", nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK)
    {
        throw std::runtime_error("Failed to commit transaction: " + std::string(errMsg));
    }

    sqlite3_close(db);
}

int main()
{
    std::string outputFile = "ongc.db";

    try
    {
        createDatabase(outputFile);
        std::cout << "Database created successfully!" << std::endl;
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
