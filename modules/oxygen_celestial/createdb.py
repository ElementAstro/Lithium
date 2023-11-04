# coding=utf-8

"""

Copyright(c) 2022-2023 Max Qian  <lightapt.com>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License version 3 as published by the Free Software Foundation.
This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.
You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.

"""

import os
import csv
import numpy as np
import sqlite3

outputFile = os.path.join(os.path.dirname(__file__), "data.db")

# Dictionaries
objectTypes = {
    "*": "Star",
    "**": "Double star",
    "*Ass": "Association of stars",
    "OCl": "Open Cluster",
    "GCl": "Globular Cluster",
    "Cl+N": "Star cluster + Nebula",
    "G": "Galaxy",
    "GPair": "Galaxy Pair",
    "GTrpl": "Galaxy Triplet",
    "GGroup": "Group of galaxies",
    "PN": "Planetary Nebula",
    "HII": "HII Ionized region",
    "DrkN": "Dark Nebula",
    "EmN": "Emission Nebula",
    "Neb": "Nebula",
    "RfN": "Reflection Nebula",
    "SNR": "Supernova remnant",
    "Nova": "Nova star",
    "NonEx": "Nonexistent object",
    "Other": "Object of other/unknown type",
    "Dup": "Duplicated record",
}

# Create db
try:
    db = sqlite3.connect(outputFile)
    cursor = db.cursor()

    # Create objects types table
    cursor.execute("DROP TABLE IF EXISTS objTypes")
    cursor.execute(
        "CREATE TABLE IF NOT EXISTS objTypes("
        "type TEXT PRIMARY KEY NOT NULL, "
        "typedesc TEXT NOT NULL)"
    )
    listTypes = objectTypes.items()
    cursor.executemany("INSERT INTO objTypes VALUES(?,?)", listTypes)

    # Create main objects table
    cursor.execute("DROP TABLE IF EXISTS objects")
    cursor.execute(
        "CREATE TABLE IF NOT EXISTS objects("
        "Name TEXT NOT NULL UNIQUE, "
        "Type TEXT NOT NULL, "
        "RA REAL, "
        "Dec REAL, "
        "Const TEXT) "
    )

    # Create object identifiers table
    cursor.execute("DROP TABLE IF EXISTS objIdentifiers")
    cursor.execute(
        "CREATE TABLE IF NOT EXISTS objIdentifiers("
        "id INTEGER PRIMARY KEY NOT NULL, "
        "name TEXT NOT NULL, "
        "identifier TEXT NOT NULL UNIQUE)"
    )
    filename = "data.csv"
    if True:
        notngc = True if filename != "data.csv" else False
        with open(filename, "r") as csvFile:
            reader = csv.DictReader(csvFile, delimiter=",")
            # List of columns that are not text and should be transformed in NULL if empty
            columns_maybe_null = [
               
            ]
            for line in reader:
                for column in columns_maybe_null:
                    if line[column] == "":
                        line[column] = None
                # Convert RA and Dec in radians
                if line["RA"] != "":
                    ra_array = np.array([float(x) for x in line["RA"].split(":")])
                    ra_rad = np.radians(np.sum(ra_array * [15, 1 / 4, 1 / 240]))
                else:
                    ra_rad = None
                if line["Dec"] != "":
                    dec_array = np.array([float(x) for x in line["Dec"].split(":")])
                    if np.signbit(dec_array[0]):
                        dec_rad = np.radians(
                            np.sum(dec_array * [1, -1 / 60, -1 / 3600])
                        )
                    else:
                        dec_rad = np.radians(np.sum(dec_array * [1, 1 / 60, 1 / 3600]))
                else:
                    dec_rad = None
                cursor.execute(
                    "INSERT INTO objects(name,type,ra,dec,const)"
                    "VALUES(?,?,?,?,?)",
                    (
                        line["Name"],
                        line["Type"],
                        ra_rad,
                        dec_rad,
                        line["Const"],
                    ),
                )
                cursor.execute(
                    "INSERT INTO objIdentifiers(name,identifier) VALUES(?,?)",
                    (line["Name"], line["Name"].upper()),
                )
                
    cursor.execute(
        'CREATE UNIQUE INDEX "idx_identifiers" ON "objIdentifiers" ("identifier");'
    )
    db.commit()
except Exception as e:
    db.rollback()
    raise e
finally:
    db.close()
