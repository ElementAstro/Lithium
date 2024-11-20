import sys
from astroplan import Observer
from astropy.time import Time
from astropy.coordinates import Angle, EarthLocation
from datetime import datetime, timedelta
from zoneinfo import ZoneInfo
from typing import Dict, Union, Optional
from loguru import logger
import argparse


def setup_logging():
    """Set up the loguru logging configuration to log both to console and to a file."""
    logger.add("twilight_calculation.log", level="DEBUG",
               format="{time} {level} {message}", rotation="10 MB")
    logger.info("Logging setup complete.")


def calculate_twilight(observer: Observer, time: Time, time_offset: ZoneInfo) -> Dict[str, Dict[str, str]]:
    """
    Calculate twilight times for a given observer and time.

    Parameters:
        observer (Observer): The observer's location.
        time (Time): The reference time for calculating twilight.
        time_offset (ZoneInfo): Timezone offset for local time conversion.

    Returns:
        Dict[str, Dict[str, str]]: Dictionary with evening and morning twilight times.
    """
    try:
        logger.debug(
            f"Calculating twilight times for time: {time}, location: {observer.location}")

        # Calculate the UTC offset for local time conversion
        t_delta = timedelta(hours=time_offset.utcoffset(
            datetime.now()).total_seconds() / 3600)

        # Check if it's currently night time
        is_night = observer.is_night(time)
        logger.debug(
            f"Is it currently night time? {'Yes' if is_night else 'No'}")

        # Determine twilight times based on whether it is night
        if is_night:
            sun_set_time = observer.sun_set_time(time, which='previous')
            twilight_civil_time = observer.twilight_evening_civil(
                time, which='previous')
            twilight_nautical_time = observer.twilight_evening_nautical(
                time, which='previous')
            twilight_astro_time = observer.twilight_evening_astronomical(
                time, which='previous')
        else:
            sun_set_time = observer.sun_set_time(time, which='next')
            twilight_civil_time = observer.twilight_evening_civil(
                time, which='next')
            twilight_nautical_time = observer.twilight_evening_nautical(
                time, which='next')
            twilight_astro_time = observer.twilight_evening_astronomical(
                time, which='next')

        sun_rise_time = observer.sun_rise_time(time, which='next')
        morning_civil_time = observer.twilight_morning_civil(
            time, which='next')
        morning_nautical_time = observer.twilight_morning_nautical(
            time, which='next')
        morning_astro_time = observer.twilight_morning_astronomical(
            time, which='next')

        # Convert all times to local time and format
        def format_time(t): return (
            t.datetime + t_delta).strftime('%Y-%m-%d %H:%M:%S')

        twilight_times = {
            'evening': {
                'sun_set_time': format_time(sun_set_time),
                'evening_civil_time': format_time(twilight_civil_time),
                'evening_nautical_time': format_time(twilight_nautical_time),
                'evening_astro_time': format_time(twilight_astro_time),
            },
            'morning': {
                'sun_rise_time': format_time(sun_rise_time),
                'morning_civil_time': format_time(morning_civil_time),
                'morning_nautical_time': format_time(morning_nautical_time),
                'morning_astro_time': format_time(morning_astro_time),
            }
        }

        logger.info(
            f"Twilight times calculated successfully: {twilight_times}")
        return twilight_times
    except Exception as e:
        logger.error(f"Error in calculating twilight times: {e}")
        return {}


def find_sun_altitude_time(observer: Observer, target_angle: Union[float, Angle], time: Time, time_offset: ZoneInfo) -> Optional[str]:
    """
    Find the time when the Sun reaches a specific altitude angle.

    Parameters:
        observer (Observer): The observer's location.
        target_angle (Union[float, Angle]): The target altitude angle of the Sun in degrees or as an Angle object.
        time (Time): The reference time for calculating the altitude.
        time_offset (ZoneInfo): Timezone offset for local time conversion.

    Returns:
        Optional[str]: The time (as a formatted string) when the Sun reaches the specified altitude, or None if not found.
    """
    try:
        logger.debug(
            f"Finding sun altitude time for target angle: {target_angle}, time: {time}, location: {observer.location}")

        # Convert the angle to an Angle object if necessary
        if isinstance(target_angle, (int, float)):
            target_angle = Angle(target_angle, unit='deg')

        # Find the time when the Sun reaches the target altitude
        sun_altitude_time = observer.target_altaz(time, target='sun', grid_times=[
                                                  time], which='nearest', alt=target_angle)

        if sun_altitude_time:
            t_delta = timedelta(hours=time_offset.utcoffset(
                datetime.now()).total_seconds() / 3600)
            formatted_time = (sun_altitude_time.datetime +
                              t_delta).strftime('%Y-%m-%d %H:%M:%S')
            logger.info(f"Sun altitude time found: {formatted_time}")
            return formatted_time
        else:
            logger.warning("Sun altitude time not found.")
            return None
    except Exception as e:
        logger.error(f"Error in finding sun altitude time: {e}")
        return None


def calculate_golden_hour(observer: Observer, time: Time, time_offset: ZoneInfo) -> Dict[str, Optional[str]]:
    """
    Calculate the start and end times for the golden hour (just after sunrise and just before sunset).

    Parameters:
        observer (Observer): The observer's location.
        time (Time): The reference time for calculating golden hour.
        time_offset (ZoneInfo): Timezone offset for local time conversion.

    Returns:
        Dict[str, Optional[str]]: Dictionary containing the start and end times of the morning and evening golden hours.
    """
    try:
        logger.debug(
            f"Calculating golden hour for time: {time}, location: {observer.location}")

        # Convert the angle to an Angle object (approximately -6 degrees for golden hour)
        golden_hour_angle = Angle(-6, unit='deg')

        # Morning golden hour
        morning_start = observer.target_altaz(time, target='sun', grid_times=[
                                              time], which='previous', alt=golden_hour_angle)
        morning_end = observer.sun_rise_time(time, which='next')

        # Evening golden hour
        evening_start = observer.sun_set_time(time, which='next')
        evening_end = observer.target_altaz(time, target='sun', grid_times=[
                                            time], which='next', alt=golden_hour_angle)

        # Convert to local time and format
        t_delta = timedelta(hours=time_offset.utcoffset(
            datetime.now()).total_seconds() / 3600)

        def format_time(t): return (
            t.datetime + t_delta).strftime('%Y-%m-%d %H:%M:%S') if t else None

        golden_hour_times = {
            'morning_golden_hour_start': format_time(morning_start),
            'morning_golden_hour_end': format_time(morning_end),
            'evening_golden_hour_start': format_time(evening_start),
            'evening_golden_hour_end': format_time(evening_end),
        }

        logger.info(
            f"Golden hour times calculated successfully: {golden_hour_times}")
        return golden_hour_times
    except Exception as e:
        logger.error(f"Error in calculating golden hour: {e}")
        return {}


def main():
    parser = argparse.ArgumentParser(
        description="Calculate twilight and golden hour times for a given location and time.")
    parser.add_argument('--lat', type=float, required=True,
                        help='Latitude of the observer in degrees')
    parser.add_argument('--lon', type=float, required=True,
                        help='Longitude of the observer in degrees')
    parser.add_argument('--height', type=float, default=0,
                        help='Height of the observer above sea level in meters')
    parser.add_argument('--date', type=str, required=True,
                        help='Date and time of the observation (format: YYYY-MM-DD HH:MM:SS)')
    parser.add_argument('--timezone', type=str, required=True,
                        help='Timezone of the observation (e.g., America/Los_Angeles)')

    args = parser.parse_args()

    # Set up logging
    setup_logging()

    try:
        observer_location = EarthLocation(
            lat=args.lat * u.deg, lon=args.lon * u.deg, height=args.height * u.m)
        observer = Observer(location=observer_location)
        observation_time = Time(datetime.strptime(
            args.date, '%Y-%m-%d %H:%M:%S'))
        time_offset = ZoneInfo(args.timezone)

        # Calculate twilight times
        twilight_times = calculate_twilight(
            observer, observation_time, time_offset)
        print("Twilight times:", twilight_times)

        # Find when the Sun reaches a specific altitude (-6 degrees is often used for civil twilight)
        sun_altitude_time = find_sun_altitude_time(
            observer, -6, observation_time, time_offset)
        print("Sun altitude time (-6 degrees):", sun_altitude_time)

        # Calculate golden hour times
        golden_hour_times = calculate_golden_hour(
            observer, observation_time, time_offset)
        print("Golden hour times:", golden_hour_times)

    except Exception as e:
        logger.error(f"Error in main execution: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
