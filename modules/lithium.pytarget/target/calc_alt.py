import datetime
from astropy.coordinates import EarthLocation, AltAz, SkyCoord
from astroplan import Observer, FixedTarget, moon_illumination
from astropy.time import Time
import astropy.units as u
import numpy as np
from dataclasses import dataclass
from loguru import logger
import argparse
from typing import List, Tuple, Dict, Optional


@dataclass
class ObservationResult:
    altitude: Optional[float]
    azimuth: Optional[float]
    highest_altitude: Optional[float]
    available_shoot_time: Optional[float]
    is_above_horizon: bool


def calculate_current_alt(observation_start_time: datetime.datetime, observer_location: EarthLocation, ra: float, dec: float) -> ObservationResult:
    """
    Calculate the current altitude and azimuth of a celestial object given its RA and Dec.

    Parameters:
    observation_start_time (datetime.datetime): The start time of the observation.
    observer_location (EarthLocation): The location of the observer.
    ra (float): Right Ascension of the celestial object in degrees.
    dec (float): Declination of the celestial object in degrees.

    Returns:
    ObservationResult: A dataclass containing the altitude, azimuth, highest altitude, available shoot time, and whether the object is above the horizon.
    """
    logger.debug(
        f"Calculating current altitude and azimuth for RA: {ra}, Dec: {dec} at {observation_start_time}")

    try:
        # Create a SkyCoord object for the celestial object
        sky_obj = SkyCoord(ra, dec, unit='deg', frame='icrs')
        observation_time_astropy = Time(observation_start_time)
        altaz_frame = AltAz(obstime=observation_time_astropy,
                            location=observer_location)
        sky_obj_altaz = sky_obj.transform_to(altaz_frame)

        # Check if the object is above the horizon
        is_above_horizon = sky_obj_altaz.alt > 0 * u.deg

        # Calculate the highest altitude and available shoot time
        highest_alt = calculate_highest_alt(
            observation_start_time, observer_location, ra, dec)
        available_shoot_time = calculate_available_shoot_time(
            observation_start_time, observer_location, ra, dec)

        logger.info(
            f"Calculated altitude: {sky_obj_altaz.alt.value}°, azimuth: {sky_obj_altaz.az.value}°, highest altitude: {highest_alt}, available shoot time: {available_shoot_time} hours, is above horizon: {bool(is_above_horizon)}")
        return ObservationResult(
            altitude=sky_obj_altaz.alt.value,
            azimuth=sky_obj_altaz.az.value,
            highest_altitude=highest_alt,
            available_shoot_time=available_shoot_time,
            is_above_horizon=bool(is_above_horizon)
        )
    except Exception as e:
        logger.error(f"Error in calculating current altitude: {e}")
        return ObservationResult(altitude=None, azimuth=None, highest_altitude=None, available_shoot_time=None, is_above_horizon=False)


def calculate_highest_alt(observation_start_time: datetime.datetime, observer_location: EarthLocation, ra: float, dec: float) -> Optional[float]:
    """
    Calculate the highest altitude of a celestial object during the night.

    Parameters:
    observation_start_time (datetime.datetime): The start time of the observation.
    observer_location (EarthLocation): The location of the observer.
    ra (float): Right Ascension of the celestial object in degrees.
    dec (float): Declination of the celestial object in degrees.

    Returns:
    Optional[float]: The highest altitude of the celestial object in degrees, or None if not observable.
    """
    logger.debug(
        f"Calculating highest altitude for RA: {ra}, Dec: {dec} during the night of {observation_start_time}")

    try:
        # Create a FixedTarget object for the celestial object
        target = FixedTarget(coord=SkyCoord(
            ra, dec, unit='deg', frame='icrs'), name="Target")
        observer = Observer(location=observer_location)
        observation_time = Time(observation_start_time)

        # Calculate the start and end of the night
        begin_night = observer.twilight_evening_astronomical(
            observation_time, which='nearest')
        end_night = observer.twilight_morning_astronomical(
            observation_time, which='next')

        if begin_night < end_night:
            # Calculate the time of the object's meridian transit
            midnight = observer.target_meridian_transit_time(
                observation_time, target, which='nearest')

            if begin_night < midnight < end_night:
                max_altitude = observer.altaz(midnight, target).alt
            else:
                # Calculate the altitudes at the start and end of the night
                nighttime_altitudes = [observer.altaz(time, target).alt for time in [
                    begin_night, end_night]]
                max_altitude = max(nighttime_altitudes)
            logger.info(f"Calculated highest altitude: {max_altitude.value}°")
            return max_altitude.value
        else:
            logger.warning("The celestial object is not observable tonight.")
            return None
    except Exception as e:
        logger.error(f"Error in calculating highest altitude: {e}")
        return None


def calculate_available_shoot_time(observation_start_time: datetime.datetime, observer_location: EarthLocation, ra: float, dec: float) -> Optional[float]:
    """
    Calculate the available shooting time for a celestial object during the night.

    Parameters:
    observation_start_time (datetime.datetime): The start time of the observation.
    observer_location (EarthLocation): The location of the observer.
    ra (float): Right Ascension of the celestial object in degrees.
    dec (float): Declination of the celestial object in degrees.

    Returns:
    Optional[float]: The available shooting time in hours, or None if not observable.
    """
    logger.debug(
        f"Calculating available shooting time for RA: {ra}, Dec: {dec} during the night of {observation_start_time}")

    try:
        # Create a FixedTarget object for the celestial object
        target = FixedTarget(coord=SkyCoord(
            ra, dec, unit=u.deg), name="Target")
        observer = Observer(location=observer_location)
        observation_time = Time(observation_start_time)
        begin_of_night = observer.twilight_evening_astronomical(
            observation_time, which='nearest')
        end_of_night = observer.twilight_morning_astronomical(
            observation_time, which='next')

        step_size = 15  # minutes
        delta_t = end_of_night - begin_of_night
        if delta_t.sec <= 0:
            logger.warning(
                "Invalid time period: Begin of night is after the end of night.")
            return None

        # Generate time intervals throughout the night
        times = begin_of_night + delta_t * \
            np.arange(0, 1, step_size * 60.0 / delta_t.sec)
        altitudes = observer.altaz(times, target).alt

        # Determine the times when the object is above 50 degrees altitude
        observable_time_indices = altitudes > 50 * u.deg
        observable_times = times[observable_time_indices]

        # Estimate the total observable time in hours
        observable_time_estimate = len(observable_times) * step_size / 60
        logger.info(
            f"Calculated available shooting time: {observable_time_estimate} hours")
        return observable_time_estimate
    except Exception as e:
        logger.error(f"Error in calculating available shooting time: {e}")
        return None


def calculate_moon_phase(observation_time: datetime.datetime) -> float:
    """
    Calculate the moon phase as a percentage of illumination.

    Parameters:
    observation_time (datetime.datetime): The time of the observation.

    Returns:
    float: The moon phase as a percentage of illumination.
    """
    logger.debug(f"Calculating moon phase for {observation_time}")

    try:
        moon_phase = moon_illumination(Time(observation_time))
        logger.info(f"Calculated moon phase: {moon_phase * 100:.2f}%")
        return moon_phase * 100  # Convert to percentage
    except Exception as e:
        logger.error(f"Error in calculating moon phase: {e}")
        return -1


def evaluate_observation_conditions(observer_location: EarthLocation, observation_time: datetime.datetime, ra: float, dec: float) -> Dict[str, Optional[float]]:
    """
    Evaluate the observation conditions for a celestial object.

    Parameters:
    observer_location (EarthLocation): The location of the observer.
    observation_time (datetime.datetime): The time of the observation.
    ra (float): Right Ascension of the celestial object in degrees.
    dec (float): Declination of the celestial object in degrees.

    Returns:
    Dict[str, Optional[float]]: A dictionary containing the current altitude, azimuth, highest altitude, available shoot time, whether the object is above the horizon, moon phase, and an overall score.
    """
    logger.debug(
        f"Evaluating observation conditions for RA: {ra}, Dec: {dec} at {observation_time}")

    try:
        result = calculate_current_alt(
            observation_time, observer_location, ra, dec)
        moon_phase_percent = calculate_moon_phase(observation_time)

        score = 100

        # Adjust score based on altitude and moon phase
        if result.altitude is not None and result.altitude < 30:
            score -= 20

        if moon_phase_percent > 50:
            score -= 30

        score = max(score, 0)

        logger.info(f"Evaluation complete with score: {score}")
        return {
            "current_altitude": result.altitude,
            "current_azimuth": result.azimuth,
            "highest_altitude": result.highest_altitude,
            "available_shoot_time": result.available_shoot_time,
            "is_above_horizon": result.is_above_horizon,
            "moon_phase": moon_phase_percent,
            "overall_score": score
        }
    except Exception as e:
        logger.error(f"Error in evaluating observation conditions: {e}")
        return {}


def plan_multiple_observations(observer_location: EarthLocation, start_time: datetime.datetime, ra_dec_list: List[Tuple[float, float]], days: int = 1) -> Dict[str, Dict[str, Optional[float]]]:
    """
    Plan observations for multiple celestial objects over a specified number of days.

    Parameters:
    observer_location (EarthLocation): The location of the observer.
    start_time (datetime.datetime): The start time of the observations.
    ra_dec_list (List[Tuple[float, float]]): A list of tuples containing the RA and Dec of the celestial objects.
    days (int): The number of days to plan observations for.

    Returns:
    Dict[str, Dict[str, Optional[float]]]: A dictionary containing the observation conditions for each object on each day.
    """
    logger.debug(
        f"Planning observations for multiple objects over {days} days starting from {start_time}")

    schedule = {}
    try:
        for day in range(days):
            current_time = start_time + datetime.timedelta(days=day)
            for ra, dec in ra_dec_list:
                key = f"{ra}_{dec}_{current_time.date()}"
                schedule[key] = evaluate_observation_conditions(
                    observer_location, current_time, ra, dec)
        logger.info("Multiple observation planning completed.")
        return schedule
    except Exception as e:
        logger.error(f"Error in planning multiple observations: {e}")
        return {}


def main():
    """
    Main function to parse command line arguments and calculate observation conditions.
    """
    parser = argparse.ArgumentParser(
        description="Calculate celestial object observation conditions.")
    parser.add_argument("--lat", type=float, required=True,
                        help="Latitude of the observer location.")
    parser.add_argument("--lon", type=float, required=True,
                        help="Longitude of the observer location.")
    parser.add_argument("--height", type=float, default=0,
                        help="Height of the observer location in meters.")
    parser.add_argument("--ra", type=float, required=True,
                        help="Right Ascension of the celestial object in degrees.")
    parser.add_argument("--dec", type=float, required=True,
                        help="Declination of the celestial object in degrees.")
    parser.add_argument("--days", type=int, default=1,
                        help="Number of days to plan observations for.")
    args = parser.parse_args()

    # Create an EarthLocation object for the observer's location
    location = EarthLocation(lat=args.lat * u.deg,
                             lon=args.lon * u.deg, height=args.height * u.m)
    observation_time = datetime.datetime.now()

    # Calculate the current altitude and azimuth
    result = calculate_current_alt(
        observation_time, location, args.ra, args.dec)
    print(
        f"Current Altitude: {result.altitude} degrees, Azimuth: {result.azimuth} degrees")
    print(f"Highest Altitude: {result.highest_altitude} degrees")
    print(f"Available Shooting Time: {result.available_shoot_time} hours")
    print(f"Is Above Horizon: {result.is_above_horizon}")

    # Plan observations for multiple days
    ra_dec_list = [(args.ra, args.dec)]
    schedule = plan_multiple_observations(
        location, observation_time, ra_dec_list, days=args.days)
    print(schedule)


if __name__ == "__main__":
    main()
