from astropy.coordinates import EarthLocation, SkyCoord, AltAz
from astropy.time import Time
import astropy.units as u
from astroplan import Observer
from datetime import datetime, timedelta, time, timezone
from typing import List, Tuple, Optional
from loguru import logger
import matplotlib.pyplot as plt
import csv
import sys
import argparse

# Configure loguru to print detailed log information
logger.add("debug.log", level="DEBUG",
           format="{time} {level} {message}", rotation="10 MB")


def calculate_azimuth_altitude(observation_datetime: datetime, observer_location: EarthLocation,
                               star_ra: float, star_dec: float) -> Tuple[Optional[float], Optional[float]]:
    """
    Calculate the azimuth and altitude of a celestial object.

    :param observation_datetime: The date and time of the observation (must be timezone-aware).
    :param observer_location: The location of the observer on Earth.
    :param star_ra: Right Ascension of the celestial object in degrees.
    :param star_dec: Declination of the celestial object in degrees.
    :return: A tuple containing the azimuth and altitude, or (None, None) if an error occurs.
    """
    try:
        if observation_datetime.tzinfo is None:
            raise ValueError("observation_datetime must be timezone-aware")

        # Create a SkyCoord object for the celestial object
        sky_obj = SkyCoord(ra=star_ra * u.deg,
                           dec=star_dec * u.deg, frame='icrs')
        altaz_frame = AltAz(obstime=Time(
            observation_datetime), location=observer_location)
        altaz = sky_obj.transform_to(altaz_frame)

        azimuth = altaz.az.degree
        altitude = altaz.alt.degree

        logger.info(
            f"Calculation complete - Azimuth: {azimuth:.2f}°, Altitude: {altitude:.2f}°")
        return azimuth, altitude

    except ValueError as ve:
        logger.error(f"Value error: {ve}")
        return None, None
    except Exception as e:
        logger.error(f"Error calculating azimuth and altitude: {e}")
        return None, None


def calculate_star_info(observation_start_time: datetime, observer_location: EarthLocation,
                        ra: float, dec: float, time_span: int = 20,
                        observation_hours: Tuple[int, int] = (16, 8)) -> List[Tuple[str, float, float]]:
    """
    Calculate the azimuth and altitude of a celestial object over a period of time.

    :param observation_start_time: The start date and time of the observation (must be timezone-aware).
    :param observer_location: The location of the observer on Earth.
    :param ra: Right Ascension of the celestial object in degrees.
    :param dec: Declination of the celestial object in degrees.
    :param time_span: The time interval for calculations in minutes.
    :param observation_hours: The start and end hours of the observation period.
    :return: A list of tuples containing the time string, azimuth, and altitude.
    """
    try:
        if observation_start_time.tzinfo is None:
            raise ValueError("observation_start_time must be timezone-aware")

        start_hour, end_hour = observation_hours
        start_time = datetime.combine(observation_start_time.date(), time(
            start_hour, 0), tzinfo=observation_start_time.tzinfo)
        end_time = datetime.combine((observation_start_time + timedelta(
            days=1)).date(), time(end_hour, 0), tzinfo=observation_start_time.tzinfo)

        total_minutes = int((end_time - start_time).total_seconds() / 60)
        time_points = [start_time + timedelta(minutes=i)
                       for i in range(0, total_minutes + 1, time_span)]

        star_info = []
        for current_time in time_points:
            az, alt = calculate_azimuth_altitude(
                current_time, observer_location, ra, dec)
            if az is not None and alt is not None:
                star_info.append(
                    (current_time.strftime('%Y-%m-%d %H:%M:%S'), az, alt))

        logger.info(
            f"Calculation complete, {len(star_info)} data points generated.")
        return star_info

    except ValueError as ve:
        logger.error(f"Value error: {ve}")
        return []
    except Exception as e:
        logger.error(f"Error calculating star information: {e}")
        return []


def calculate_rise_set_times(observer_location: EarthLocation, ra: float, dec: float,
                             date: datetime, horizon: float = 0) -> Tuple[Optional[str], Optional[str]]:
    """
    Calculate the rise and set times of a celestial object.

    :param observer_location: The location of the observer on Earth.
    :param ra: Right Ascension of the celestial object in degrees.
    :param dec: Declination of the celestial object in degrees.
    :param date: The date for which to calculate rise and set times (must be timezone-aware).
    :param horizon: The horizon altitude in degrees.
    :return: A tuple containing the rise and set times as strings, or (None, None) if an error occurs.
    """
    try:
        if date.tzinfo is None:
            raise ValueError("date must be timezone-aware")

        star_coord = SkyCoord(ra=ra * u.deg, dec=dec * u.deg, frame='icrs')
        observer = Observer(location=observer_location, timezone=date.tzinfo)

        rise_time = observer.target_rise_time(
            Time(date), star_coord, which='nearest', horizon=horizon*u.deg)
        set_time = observer.target_set_time(
            Time(date), star_coord, which='nearest', horizon=horizon*u.deg)

        rise_str = rise_time.to_datetime().strftime(
            '%Y-%m-%d %H:%M:%S') if rise_time else None
        set_str = set_time.to_datetime().strftime(
            '%Y-%m-%d %H:%M:%S') if set_time else None

        logger.info(
            f"Calculation complete - Rise time: {rise_str}, Set time: {set_str}")
        return rise_str, set_str

    except ValueError as ve:
        logger.error(f"Value error: {ve}")
        return None, None
    except Exception as e:
        logger.error(f"Error calculating rise and set times: {e}")
        return None, None


def plot_star_info(star_info: List[Tuple[str, float, float]]):
    """
    Plot the azimuth and altitude of a celestial object over time.

    :param star_info: A list of tuples containing the time string, azimuth, and altitude.
    """
    try:
        times = [datetime.strptime(entry[0], '%Y-%m-%d %H:%M:%S')
                 for entry in star_info]
        azimuths = [entry[1] for entry in star_info]
        altitudes = [entry[2] for entry in star_info]

        plt.figure(figsize=(12, 6))
        plt.plot(times, azimuths, label='Azimuth (°)')
        plt.plot(times, altitudes, label='Altitude (°)')
        plt.xlabel('Time')
        plt.ylabel('Degrees')
        plt.title('Star Azimuth and Altitude Over Time')
        plt.legend()
        plt.grid(True)
        plt.tight_layout()
        plt.savefig('star_plot.png')
        plt.close()
        logger.info("Star information plot saved as star_plot.png")
    except Exception as e:
        logger.error(f"Error plotting star information: {e}")


def export_to_csv(star_info: List[Tuple[str, float, float]], filename: str = 'star_info.csv'):
    """
    Export the star information to a CSV file.

    :param star_info: A list of tuples containing the time string, azimuth, and altitude.
    :param filename: The name of the CSV file.
    """
    try:
        with open(filename, mode='w', newline='') as file:
            writer = csv.writer(file)
            writer.writerow(['Time', 'Azimuth (°)', 'Altitude (°)'])
            writer.writerows(star_info)
        logger.info(f"Star information exported to {filename}")
    except Exception as e:
        logger.error(f"Error exporting to CSV: {e}")


def main():
    parser = argparse.ArgumentParser(
        description="Calculate and process the azimuth and altitude of a celestial object.")
    parser.add_argument('--ra', type=float, required=True,
                        help='Right Ascension of the celestial object in degrees')
    parser.add_argument('--dec', type=float, required=True,
                        help='Declination of the celestial object in degrees')
    parser.add_argument('--lat', type=float, required=True,
                        help='Latitude of the observer in degrees')
    parser.add_argument('--lon', type=float, required=True,
                        help='Longitude of the observer in degrees')
    parser.add_argument('--height', type=float, default=0,
                        help='Height of the observer above sea level in meters')
    parser.add_argument('--date', type=str, required=True,
                        help='Start time of the observation (format: YYYY-MM-DD HH:MM:SS, timezone-aware)')
    parser.add_argument('--plot', action='store_true',
                        help='Generate a plot of the star information')
    parser.add_argument('--export', action='store_true',
                        help='Export the star information to a CSV file')

    args = parser.parse_args()

    try:
        observation_start_time = datetime.strptime(
            args.date, '%Y-%m-%d %H:%M:%S').replace(tzinfo=timezone.utc)
        location = EarthLocation(
            lat=args.lat * u.deg, lon=args.lon * u.deg, height=args.height * u.m)

        star_info = calculate_star_info(
            observation_start_time, location, args.ra, args.dec)
        for info in star_info:
            date_string, az, alt = info
            print(
                f"Time: {date_string}, Azimuth: {az:.2f}°, Altitude: {alt:.2f}°")

        if args.plot:
            plot_star_info(star_info)

        if args.export:
            export_to_csv(star_info)

        # Calculate rise and set times
        rise_time, set_time = calculate_rise_set_times(
            location, args.ra, args.dec, observation_start_time)
        print(f"Rise time: {rise_time}, Set time: {set_time}")

    except Exception as e:
        logger.error(f"Error in main execution: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
