from astropy.coordinates import EarthLocation, SkyCoord, AltAz
from astropy.time import Time
import astropy.units as u
from astroplan import Observer
from datetime import datetime, timedelta, time, timezone
from typing import List, Tuple, Optional
from loguru import logger

# 配置loguru以打印详细的日志信息
logger.add("debug.log", level="DEBUG",
           format="{time} {level} {message}", rotation="10 MB")


def calculate_azimuth_altitude(observation_datetime: datetime, observer_location: EarthLocation,
                               star_ra: float, star_dec: float) -> Tuple[Optional[float], Optional[float]]:
    """
    Calculate the azimuth and altitude of a star given the observation time and observer's location.

    :param observation_datetime: The date and time of the observation.
    :param observer_location: The location of the observer on Earth.
    :param star_ra: Right Ascension of the star in degrees.
    :param star_dec: Declination of the star in degrees.
    :return: A tuple containing the azimuth and altitude in degrees, or (None, None) if an error occurs.
    """
    try:
        logger.debug(
            f"Calculating azimuth and altitude for RA: {star_ra}, Dec: {star_dec} at {observation_datetime}")

        if observation_datetime.tzinfo is None:
            raise ValueError("observation_datetime must be timezone-aware")

        star_coord = SkyCoord(ra=star_ra, dec=star_dec,
                              unit="deg", frame="icrs")
        altaz_frame = AltAz(obstime=Time(
            observation_datetime), location=observer_location)
        star_altaz = star_coord.transform_to(altaz_frame)

        azimuth = star_altaz.az.deg
        altitude = star_altaz.alt.deg

        logger.info(
            f"Calculated azimuth: {azimuth:.2f}°, altitude: {altitude:.2f}° for star at RA: {star_ra}, Dec: {star_dec}")
        return azimuth, altitude

    except Exception as e:
        logger.error(f"Error in calculating azimuth and altitude: {e}")
        return None, None


def calculate_star_info(observation_start_time: datetime, observer_location: EarthLocation,
                        ra: float, dec: float, time_span: int = 20,
                        observation_hours: Tuple[int, int] = (16, 8)) -> List[Tuple[str, float, float]]:
    """
    Calculate azimuth and altitude of a star over a night from the specified start time to the end time next day 
    at specified intervals.

    :param observation_start_time: The date and time when observation starts.
    :param observer_location: The location of the observer on Earth.
    :param ra: Right Ascension of the star in degrees.
    :param dec: Declination of the star in degrees.
    :param time_span: The time interval in minutes between calculations (default is 20 minutes).
    :param observation_hours: Tuple indicating the start and end hours of the observation period.
                              Defaults to (16, 8), meaning from 16:00 to 08:00 next day.
    :return: A list of tuples, each containing the time string, azimuth, and altitude.
    """
    try:
        logger.debug(
            f"Calculating star information from {observation_start_time} for RA: {ra}, Dec: {dec}")

        if observation_start_time.tzinfo is None:
            raise ValueError("observation_start_time must be timezone-aware")

        start_hour, end_hour = observation_hours
        start_time = datetime.combine(observation_start_time.date(), time(start_hour, 0),
                                      tzinfo=observation_start_time.tzinfo)
        end_time = datetime.combine((observation_start_time + timedelta(days=1)).date(),
                                    time(end_hour, 0), tzinfo=observation_start_time.tzinfo)

        star_info = []

        while start_time < end_time:
            az, alt = calculate_azimuth_altitude(
                start_time, observer_location, ra, dec)
            if az is not None and alt is not None:
                star_info.append(
                    (start_time.strftime('%Y-%m-%d %H:%M:%S'), az, alt))
                logger.debug(
                    f"Time: {start_time.strftime('%Y-%m-%d %H:%M:%S')}, Azimuth: {az:.2f}°, Altitude: {alt:.2f}°")

            start_time += timedelta(minutes=time_span)

        logger.info(
            f"Calculated star information for {len(star_info)} time points.")
        return star_info

    except Exception as e:
        logger.error(f"Error in calculating star information: {e}")
        return []


def calculate_rise_set_times(observer_location: EarthLocation, ra: float, dec: float,
                             date: datetime, horizon: float = 0) -> Tuple[Optional[str], Optional[str]]:
    """
    Calculate the rise and set times of a star for a given date and observer's location.

    :param observer_location: The location of the observer on Earth.
    :param ra: Right Ascension of the star in degrees.
    :param dec: Declination of the star in degrees.
    :param date: The date for which to calculate the rise and set times.
    :param horizon: The altitude of the horizon in degrees (default is 0, representing the true horizon).
    :return: A tuple containing the rise and set times as strings, or (None, None) if an error occurs.
    """
    try:
        logger.debug(
            f"Calculating rise and set times for RA: {ra}, Dec: {dec} on {date}")

        star_coord = SkyCoord(ra=ra, dec=dec, unit="deg", frame="icrs")
        observer = Observer(location=observer_location, timezone=date.tzinfo)

        rise_time = observer.target_rise_time(
            Time(date), star_coord, which='nearest', horizon=horizon*u.deg)
        set_time = observer.target_set_time(
            Time(date), star_coord, which='nearest', horizon=horizon*u.deg)

        rise_str = rise_time.datetime.strftime(
            '%Y-%m-%d %H:%M:%S') if rise_time else None
        set_str = set_time.datetime.strftime(
            '%Y-%m-%d %H:%M:%S') if set_time else None

        logger.info(
            f"Calculated rise time: {rise_str}, set time: {set_str} for star at RA: {ra}, Dec: {dec}")
        return rise_str, set_str

    except Exception as e:
        logger.error(f"Error in calculating rise and set times: {e}")
        return None, None


# Example Usage
if __name__ == "__main__":
    observer_location = EarthLocation(
        lat=30.2525, lon=120.0400, height=0)  # Hangzhou, China
    observation_start_time = datetime(
        2023, 9, 3, 19, 0, 0, tzinfo=timezone.utc)
    ra = 10.684583333333332  # RA of the star
    dec = 41.26888888888889  # Dec of the star

    star_info = calculate_star_info(
        observation_start_time, observer_location, ra, dec)
    for info in star_info:
        date_string, az, alt = info
        print(f"Time: {date_string}, Azimuth: {az:.2f}°, Altitude: {alt:.2f}°")

    # Calculate rise and set times
    rise_time, set_time = calculate_rise_set_times(
        observer_location, ra, dec, observation_start_time)
    print(f"Rise time: {rise_time}, Set time: {set_time}")
