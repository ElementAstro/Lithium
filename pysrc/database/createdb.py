import csv
import os
import numpy as np
import re
import sqlite3
from typing import Dict, List, Tuple, Optional
from loguru import logger
import argparse

# 默认输出数据库文件路径
default_db_path = os.path.join(os.path.dirname(
    __file__), os.pardir, 'src', 'pyongc', 'ongc.db')

# Object Types
objectTypes: Dict[str, str] = {
    '*': 'Star',
    '**': 'Double star',
    '*Ass': 'Association of stars',
    'OCl': 'Open Cluster',
    'GCl': 'Globular Cluster',
    'Cl+N': 'Star cluster + Nebula',
    'G': 'Galaxy',
    'GPair': 'Galaxy Pair',
    'GTrpl': 'Galaxy Triplet',
    'GGroup': 'Group of galaxies',
    'PN': 'Planetary Nebula',
    'HII': 'HII Ionized region',
    'DrkN': 'Dark Nebula',
    'EmN': 'Emission Nebula',
    'Neb': 'Nebula',
    'RfN': 'Reflection Nebula',
    'SNR': 'Supernova remnant',
    'Nova': 'Nova star',
    'NonEx': 'Nonexistent object',
    'Other': 'Object of other/unknown type',
    'Dup': 'Duplicated record'
}

PATTERNS: Dict[str, str] = {
    'NGC|IC': r'^((?:NGC|IC)\s?)(\d{1,4})\s?((NED)(\d{1,2})|[A-Z]{1,2})?$',
    'Messier': r'^(M\s?)(\d{1,3})$',
    'Barnard': r'^(B\s?)(\d{1,3})$',
    'Caldwell': r'^(C\s?)(\d{1,3})$',
    'Collinder': r'^(CL\s?)(\d{1,3})$',
    'ESO': r'^(ESO\s?)(\d{1,3})-(\d{1,3})$',
    'Harvard': r'^(H\s?)(\d{1,2})$',
    'Hickson': r'^(HCG\s?)(\d{1,3})$',
    'LBN': r'^(LBN\s?)(\d{1,3})$',
    'MCG': r'^(MCG\s?)([+-]\d{2}-\d{2}-\d{3,4})$',
    'Melotte': r'^(MEL\s?)(\d{1,3})$',
    'MWSC': r'^(MWSC\s?)(\d{1,4})$',
    'PGC': r'^((?:PGC|LEDA)\s?)(\d{1,6})$',
    'UGC': r'^(UGC\s?)(\d{1,5})$',
    'UGCA': r'^(UGCA\s?)(\d{1,3})$',
}


def setup_logging():
    """Set up the loguru logging configuration to log both to console and to a file."""
    logger.add("create_db.log", level="DEBUG",
               format="{time} {level} {message}", rotation="10 MB")
    logger.info("Logging setup complete.")


def parse_ra_dec(ra_str: str, dec_str: str) -> Tuple[Optional[float], Optional[float]]:
    if ra_str:
        ra_array = np.array([float(x) for x in ra_str.split(':')])
        ra_rad = np.radians(np.sum(ra_array * [15, 1 / 4, 1 / 240]))
        logger.debug(f"Parsed RA: {ra_str} to radians: {ra_rad}")
    else:
        ra_rad = None

    if dec_str:
        dec_array = np.array([float(x) for x in dec_str.split(':')])
        sign = -1 if np.signbit(dec_array[0]) else 1
        dec_rad = np.radians(np.sum(dec_array * [1, sign / 60, sign / 3600]))
        logger.debug(f"Parsed Dec: {dec_str} to radians: {dec_rad}")
    else:
        dec_rad = None

    return ra_rad, dec_rad


def insert_object_identifiers(cursor: sqlite3.Cursor, name: str, identifier: str) -> None:
    for cat, pat in PATTERNS.items():
        if match := re.match(pat, identifier):
            match cat:
                case 'NGC|IC':
                    if match.group(3):
                        if match.group(4):
                            objectname = f"{match.group(1).strip()}{int(match.group(2)):04d} {match.group(4)}{int(match.group(5)):02d}"
                        else:
                            objectname = f"{match.group(1).strip()}{int(match.group(2)):04d}{match.group(3).strip()}"
                    else:
                        objectname = f"{match.group(1).strip()}{int(match.group(2)):04d}"
                case 'ESO':
                    objectname = f"{match.group(1).strip()}{int(match.group(2)):03d}-{int(match.group(3)):03d}"
                case 'Harvard':
                    objectname = f"{match.group(1).strip()}{int(match.group(2)):02d}"
                case 'UGC':
                    objectname = f"{match.group(1).strip()}{int(match.group(2)):05d}"
                case 'PGC':
                    objectname = f"PGC{int(match.group(2)):06d}"
                case 'MCG':
                    objectname = f"{match.group(1).strip()}{match.group(2).strip()}"
                case _:
                    objectname = f"{match.group(1).strip()}{int(match.group(2)):03d}"

            logger.debug(
                f"Inserting identifier: {objectname} for object: {name}")
            cursor.execute(
                'INSERT INTO objIdentifiers(name, identifier) VALUES(?, ?)', (name, objectname))


def create_database(db_path: str, filenames: List[str]) -> None:
    """Creates the SQLite database from CSV files."""
    try:
        with sqlite3.connect(db_path) as db:
            cursor = db.cursor()

            logger.info("Creating tables in database...")
            # Create objects types table
            cursor.execute('DROP TABLE IF EXISTS objTypes')
            cursor.execute('''CREATE TABLE IF NOT EXISTS objTypes(
                                type TEXT PRIMARY KEY NOT NULL,
                                typedesc TEXT NOT NULL)''')
            cursor.executemany(
                'INSERT INTO objTypes VALUES(?, ?)', objectTypes.items())

            # Create main objects table
            cursor.execute('DROP TABLE IF EXISTS objects')
            cursor.execute('''CREATE TABLE IF NOT EXISTS objects(
                                id INTEGER PRIMARY KEY NOT NULL,
                                name TEXT NOT NULL UNIQUE,
                                type TEXT NOT NULL,
                                ra REAL,
                                dec REAL,
                                const TEXT,
                                majax REAL,
                                minax REAL,
                                pa INTEGER,
                                bmag REAL,
                                vmag REAL,
                                jmag REAL,
                                hmag REAL,
                                kmag REAL,
                                sbrightn REAL,
                                hubble TEXT,
                                parallax REAL,
                                pmra REAL,
                                pmdec REAL,
                                radvel INTEGER,
                                redshift REAL,
                                cstarumag REAL,
                                cstarbmag REAL,
                                cstarvmag REAL,
                                messier TEXT,
                                ngc TEXT,
                                ic TEXT,
                                cstarnames TEXT,
                                identifiers TEXT,
                                commonnames TEXT,
                                nednotes TEXT,
                                ongcnotes TEXT,
                                notngc BOOL DEFAULT FALSE)''')

            # Create object identifiers table
            cursor.execute('DROP TABLE IF EXISTS objIdentifiers')
            cursor.execute('''CREATE TABLE IF NOT EXISTS objIdentifiers(
                                id INTEGER PRIMARY KEY NOT NULL,
                                name TEXT NOT NULL,
                                identifier TEXT NOT NULL UNIQUE)''')

            columns_maybe_null: List[str] = [
                'MajAx', 'MinAx', 'PosAng', 'B-Mag', 'V-Mag', 'J-Mag', 'H-Mag', 'K-Mag',
                'SurfBr', 'Pax', 'Pm-RA', 'Pm-Dec', 'RadVel', 'Redshift', 'Cstar U-Mag',
                'Cstar B-Mag', 'Cstar V-Mag'
            ]

            for filename in filenames:
                notngc = filename != 'NGC.csv'
                logger.info(f"Processing file: {filename}")

                with open(filename, 'r', encoding="utf-8") as csvFile:
                    reader = csv.DictReader(csvFile, delimiter=';')

                    for line in reader:
                        for column in columns_maybe_null:
                            if line[column] == '':
                                line[column] = None

                        ra_rad, dec_rad = parse_ra_dec(line['RA'], line['Dec'])

                        cursor.execute('''INSERT INTO objects(name, type, ra, dec, const, majax, minax, pa, bmag, vmag,
                                                               jmag, hmag, kmag, sbrightn, hubble, parallax, pmra,
                                                               pmdec, radvel, redshift, cstarumag, cstarbmag, cstarvmag,
                                                                                                                              messier, ngc, ic, cstarnames, identifiers, commonnames,
                                                               nednotes, ongcnotes, notngc)
                                       VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)''',
                                       (line['Name'], line['Type'], ra_rad, dec_rad, line['Const'],
                                        line['MajAx'], line['MinAx'], line['PosAng'], line['B-Mag'],
                                        line['V-Mag'], line['J-Mag'], line['H-Mag'], line['K-Mag'],
                                        line['SurfBr'], line['Hubble'], line['Pax'], line['Pm-RA'],
                                        line['Pm-Dec'], line['RadVel'], line['Redshift'],
                                        line['Cstar U-Mag'], line['Cstar B-Mag'], line['Cstar V-Mag'],
                                        line['M'], line['NGC'], line['IC'], line['Cstar Names'],
                                        line['Identifiers'], line['Common names'], line['NED notes'],
                                        line['OpenNGC notes'], notngc))

                        logger.debug(f"Inserted object: {line['Name']}")

                        # Insert primary identifier (name) into objIdentifiers table
                        cursor.execute('INSERT INTO objIdentifiers(name, identifier) VALUES(?, ?)',
                                       (line['Name'], line['Name'].upper()))

                        # Insert other identifiers into objIdentifiers table
                        for identifier in line['Identifiers'].split(','):
                            insert_object_identifiers(
                                cursor, line['Name'], identifier)

            # Create index on objIdentifiers table for faster searches
            cursor.execute(
                'CREATE UNIQUE INDEX "idx_identifiers" ON "objIdentifiers" ("identifier");')
            logger.info("Index created on objIdentifiers table.")

            db.commit()
            logger.info(f"Database created successfully at {db_path}")

    except Exception as e:
        db.rollback()
        logger.error(f"Error occurred: {e}")
        raise e


def main():
    parser = argparse.ArgumentParser(
        description="Creates an SQLite database from OpenNGC CSV files.")
    parser.add_argument('--db_path', type=str, default=default_db_path,
                        help="Output SQLite database file path.")
    parser.add_argument('--csv_files', type=str, nargs='+',
                        required=True, help="List of input CSV files.")
    args = parser.parse_args()

    # Set up logging
    setup_logging()

    # Create the database
    create_database(args.db_path, args.csv_files)


if __name__ == "__main__":
    main()
