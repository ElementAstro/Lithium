from astropy.coordinates import EarthLocation, AltAz, SkyCoord
import datetime
from astroplan import Observer, FixedTarget, moon_illumination
from astropy.time import Time
import astropy.units as u
import numpy as np
from dataclasses import dataclass
from loguru import logger


@dataclass
class ObservationResult:
    altitude: float
    azimuth: float
    highest_altitude: float | None
    available_shoot_time: float | None
    is_above_horizon: bool


def calculate_current_alt(observation_start_time: datetime.datetime, observer_location: EarthLocation, ra: float, dec: float) -> ObservationResult:
    """
    Calculate the current altitude and azimuth of a celestial object.

    :param observation_start_time: The start time of the observation.
    :param observer_location: The location of the observer on Earth.
    :param ra: Right Ascension of the celestial object in degrees.
    :param dec: Declination of the celestial object in degrees.
    :return: An ObservationResult containing the altitude, azimuth, highest altitude, available shoot time, and whether the object is above the horizon.
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


def calculate_highest_alt(observation_start_time: datetime.datetime, observer_location: EarthLocation, ra: float, dec: float) -> float | None:
    """
    Calculate the highest altitude of a celestial object during the night.

    :param observation_start_time: The start time of the observation.
    :param observer_location: The location of the observer on Earth.
    :param ra: Right Ascension of the celestial object in degrees.
    :param dec: Declination of the celestial object in degrees.
    :return: The highest altitude in degrees, or None if the object is not observable.
    """
    logger.debug(
        f"Calculating highest altitude for RA: {ra}, Dec: {dec} during the night of {observation_start_time}")

    try:
        target = FixedTarget(coord=SkyCoord(
            ra, dec, unit='deg', frame='icrs'), name="Target")
        observer = Observer(location=observer_location)
        observation_time = Time(observation_start_time)

        # Calculate the beginning and end of the night
        begin_night = observer.twilight_evening_astronomical(
            observation_time, which='nearest')
        end_night = observer.twilight_morning_astronomical(
            observation_time, which='next')

        if begin_night < end_night:
            # Calculate the time of the object's meridian transit (highest point in the sky)
            midnight = observer.target_meridian_transit_time(
                observation_time, target, which='nearest')

            if begin_night < midnight < end_night:
                max_altitude = observer.altaz(midnight, target).alt
            else:
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


def calculate_available_shoot_time(observation_start_time: datetime.datetime, observer_location: EarthLocation, ra: float, dec: float) -> float | None:
    """
    Calculate the available shooting time for a celestial object during the night.

    :param observation_start_time: The start time of the observation.
    :param observer_location: The location of the observer on Earth.
    :param ra: Right Ascension of the celestial object in degrees.
    :param dec: Declination of the celestial object in degrees.
    :return: The available shooting time in hours, or None if the object is not observable.
    """
    logger.debug(
        f"Calculating available shooting time for RA: {ra}, Dec: {dec} during the night of {observation_start_time}")

    try:
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

        times = begin_of_night + delta_t * \
            np.arange(0, 1, step_size * 60.0 / delta_t.sec)
        altitudes = observer.altaz(times, target).alt

        # Determine the times when the object is above 50 degrees altitude
        observable_time_indices = altitudes > 50 * u.deg
        observable_times = times[observable_time_indices]

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

    :param observation_time: The time of the observation.
    :return: The moon phase as a percentage, or -1 if an error occurs.
    """
    logger.debug(f"Calculating moon phase for {observation_time}")

    try:
        moon_phase = moon_illumination(Time(observation_time))
        logger.info(f"Calculated moon phase: {moon_phase * 100:.2f}%")
        return moon_phase * 100  # Convert to percentage
    except Exception as e:
        logger.error(f"Error in calculating moon phase: {e}")
        return -1


def evaluate_observation_conditions(observer_location: EarthLocation, observation_time: datetime.datetime, ra: float, dec: float) -> dict:
    """
    Evaluate the observation conditions for a celestial object.

    :param observer_location: The location of the observer on Earth.
    :param observation_time: The time of the observation.
    :param ra: Right Ascension of the celestial object in degrees.
    :param dec: Declination of the celestial object in degrees.
    :return: A dictionary containing the current altitude, azimuth, highest altitude, available shoot time, whether the object is above the horizon, moon phase, and an overall score.
    """
    logger.debug(
        f"Evaluating observation conditions for RA: {ra}, Dec: {dec} at {observation_time}")

    try:
        result = calculate_current_alt(
            observation_time, observer_location, ra, dec)
        moon_phase_percent = calculate_moon_phase(observation_time)

        # Initialize score
        score = 100

        # Adjust score based on altitude
        if result.altitude is not None and result.altitude < 30:
            score -= 20  # Lower score if altitude is less than 30 degrees

        # Adjust score based on moon phase
        if moon_phase_percent > 50:
            score -= 30  # Lower score if the moon is more than 50% illuminated

        # Ensure the score is not negative
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


def plan_multiple_observations(observer_location: EarthLocation, start_time: datetime.datetime, ra_dec_list: list, days: int = 1) -> dict:
    """
    Plan observations for multiple celestial objects over a specified number of days.

    :param observer_location: The location of the observer on Earth.
    :param start_time: The start time of the observation period.
    :param ra_dec_list: A list of tuples containing the Right Ascension and Declination of the celestial objects.
    :param days: The number of days over which to plan the observations.
    :return: A dictionary containing the observation schedule.
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


# Example usage:
if __name__ == "__main__":
    try:
        location = EarthLocation(
            lat=34.0522*u.deg, lon=-118.2437*u.deg, height=71*u.m)
        observation_time = datetime.datetime.now()
        ra, dec = 10.684, 41.269  # Example coordinates (RA, Dec in degrees)

        result = calculate_current_alt(observation_time, location, ra, dec)
        print(
            f"Current Altitude: {result.altitude} degrees, Azimuth: {result.azimuth} degrees")
        print(f"Highest Altitude: {result.highest_altitude} degrees")
        print(f"Available Shooting Time: {result.available_shoot_time} hours")
        print(f"Is Above Horizon: {result.is_above_horizon}")

        # Plan observations for the next 3 days
        ra_dec_list = [(10.684, 41.269), (83.822, -5.391)]
        schedule = plan_multiple_observations(
            location, observation_time, ra_dec_list, days=3)
        print(schedule)
    except Exception as e:
        logger.error(f"Error in main execution: {e}")
