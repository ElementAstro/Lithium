from zoneinfo import ZoneInfo
from astropy.coordinates import EarthLocation, AltAz, SkyCoord
import datetime
import pandas as pd
import astropy.units as u
from pathlib import Path
from typing import List, Dict, Any, Optional
from dataclasses import dataclass
from enum import Enum
from loguru import logger
import argparse

this_file_path = Path(__file__)
star_file = this_file_path.parent / 'NamedStars.csv'


class CardinalDirection(Enum):
    NORTH = 'north'
    EAST = 'east'
    SOUTH = 'south'
    WEST = 'west'


@dataclass
class SimpleLightStarInfo:
    name: str
    show_name: str
    ra: float
    dec: float
    constellation: str
    constellation_zh: str
    magnitude: float
    alt: Optional[float] = None
    az: Optional[float] = None
    sky: Optional[str] = ''

    def update_alt_az(self, alt: float, az: float) -> None:
        """
        Update altitude and azimuth of the star and determine its cardinal direction in the sky.

        Parameters:
            alt (float): Altitude in degrees.
            az (float): Azimuth in degrees.
        """
        self.alt = alt
        self.az = az
        direction_index = int((self.az + 45) / 90) % 4
        self.sky = list(CardinalDirection)[direction_index].value
        logger.debug(
            f"Updated {self.name} -> Alt: {alt:.2f}°, Az: {az:.2f}°, Sky: {self.sky}")


def setup_logging():
    """
    Set up the loguru logging configuration to log both to console and to a file.
    """
    logger.add("star_info.log", level="DEBUG",
               format="{time} {level} {message}", rotation="10 MB")
    logger.info("Logging setup complete.")


def read_light_star_file(max_magnitude: int = 2) -> List[SimpleLightStarInfo]:
    """
    Read the light star data from a CSV file and filter based on magnitude.

    Parameters:
        max_magnitude (int): Maximum magnitude of stars to include.

    Returns:
        List[SimpleLightStarInfo]: List of stars that match the criteria.
    """
    max_magnitude = min(max_magnitude, 4)
    try:
        logger.info(f"Reading star data from {star_file}")
        light_star_data = pd.read_csv(star_file)
    except FileNotFoundError:
        logger.error(f"Star data file not found at {star_file}")
        raise FileNotFoundError(f"Star data file not found at {star_file}")

    filtered_data = light_star_data[light_star_data['Mag'] < max_magnitude]
    logger.info(
        f"Filtered {len(filtered_data)} stars with magnitude < {max_magnitude}")

    return [
        SimpleLightStarInfo(
            name=row['Name'], show_name=row['Zh'], ra=float(row['RA']), dec=float(row['Dec']),
            constellation=row['Const'], constellation_zh=row['Const_Zh'], magnitude=row['Mag']
        )
        for _, row in filtered_data.iterrows()
    ]


def calculate_light_star_info(star: SimpleLightStarInfo, observation_location: EarthLocation,
                              observation_time: datetime.datetime) -> None:
    """
    Calculate altitude and azimuth for a given star based on observation location and time.

    Parameters:
        star (SimpleLightStarInfo): The star to calculate for.
        observation_location (EarthLocation): The observer's location.
        observation_time (datetime.datetime): The time of observation.
    """
    logger.debug(
        f"Calculating Alt/Az for {star.name} at {observation_time} from location {observation_location}")
    altaz_frame = AltAz(location=observation_location,
                        obstime=observation_time)
    star_coord = SkyCoord(ra=star.ra * u.deg, dec=star.dec * u.deg)
    star_altaz = star_coord.transform_to(altaz_frame)
    star.update_alt_az(star_altaz.alt.deg, star_altaz.az.deg)


def get_light_star_list(observation_location: EarthLocation, observation_time: datetime.datetime,
                        max_magnitude: int = 2, range_filter: Optional[List[str]] = None,
                        constellation_filter: Optional[str] = None, constellation_zh_filter: Optional[str] = None,
                        name_filter: Optional[str] = None, show_name_filter: Optional[str] = None) -> List[Dict[str, Any]]:
    """
    Get a list of stars visible at a given location and time with optional filtering.

    Parameters:
        observation_location (EarthLocation): The observer's location.
        observation_time (datetime.datetime): The time of observation.
        max_magnitude (int): Maximum magnitude of stars to include.
        range_filter (Optional[List[str]]): Cardinal directions to filter the stars.
        constellation_filter (Optional[str]): Constellation to filter the stars.
        constellation_zh_filter (Optional[str]): Constellation Chinese name to filter the stars.
        name_filter (Optional[str]): Star name to filter the stars.
        show_name_filter (Optional[str]): Star Chinese name to filter the stars.

    Returns:
        List[Dict[str, Any]]: List of dictionaries containing star information.
    """
    logger.info(
        f"Getting visible stars at {observation_time} for location {observation_location}")
    range_filter = range_filter or []
    valid_directions = {d.value for d in CardinalDirection}
    if range_filter and not set(range_filter).issubset(valid_directions):
        logger.warning(
            f"Invalid range filter provided: {range_filter}. Ignoring filter.")
        range_filter = []

    light_star_list = read_light_star_file(max_magnitude)
    filtered_star_list = []

    for star in light_star_list:
        calculate_light_star_info(star, observation_location, observation_time)
        if star.alt is not None and star.alt > 20:
            if (not range_filter or star.sky in range_filter) and \
               (not constellation_filter or star.constellation == constellation_filter) and \
               (not constellation_zh_filter or star.constellation_zh == constellation_zh_filter) and \
               (not name_filter or star.name == name_filter) and \
               (not show_name_filter or star.show_name == show_name_filter):
                filtered_star_list.append(star.__dict__)
                logger.debug(
                    f"Star {star.name} is visible and added to the list.")

    logger.info(
        f"Found {len(filtered_star_list)} visible stars matching criteria.")
    return filtered_star_list


def main():
    parser = argparse.ArgumentParser(
        description="Find named stars visible from a given location and time.")
    parser.add_argument('--lat', type=float, required=True,
                        help='Latitude of the observer in degrees')
    parser.add_argument('--lon', type=float, required=True,
                        help='Longitude of the observer in degrees')
    parser.add_argument('--height', type=float, default=0,
                        help='Height of the observer above sea level in meters')
    parser.add_argument('--date', type=str, required=True,
                        help='Date and time of the observation (format: YYYY-MM-DD HH:MM:SS)')
    parser.add_argument('--max_magnitude', type=int, default=2,
                        help='Maximum magnitude of stars to include')
    parser.add_argument('--range_filter', type=str, nargs='*',
                        help='Cardinal directions to filter the stars')
    parser.add_argument('--constellation_filter', type=str,
                        help='Constellation to filter the stars')
    parser.add_argument('--constellation_zh_filter', type=str,
                        help='Constellation Chinese name to filter the stars')
    parser.add_argument('--name_filter', type=str,
                        help='Star name to filter the stars')
    parser.add_argument('--show_name_filter', type=str,
                        help='Star Chinese name to filter the stars')

    args = parser.parse_args()

    # Set up logging
    setup_logging()

    try:
        observation_location = EarthLocation(
            lat=args.lat * u.deg, lon=args.lon * u.deg, height=args.height * u.m)
        observation_time = datetime.datetime.strptime(
            args.date, '%Y-%m-%d %H:%M:%S').astimezone(ZoneInfo("UTC"))

        # Get the list of visible stars
        visible_stars = get_light_star_list(
            observation_location, observation_time, args.max_magnitude, args.range_filter,
            args.constellation_filter, args.constellation_zh_filter, args.name_filter, args.show_name_filter)

        # Print the results
        if visible_stars:
            print(
                f"{'Name':<15} {'Magnitude':<10} {'Altitude':<10} {'Azimuth':<10} {'Sky':<10}")
            print("-" * 60)
            for star in visible_stars:
                print(
                    f"{star['name']:<15} {star['magnitude']:<10} {star['alt']:<10.2f} {star['az']:<10.2f} {star['sky']:<10}")
        else:
            print("No visible stars found with the given parameters.")

    except Exception as e:
        logger.error(f"An error occurred: {e}")
        print(f"An error occurred: {e}")


if __name__ == "__main__":
    main()
