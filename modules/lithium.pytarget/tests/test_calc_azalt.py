import pytest
import datetime
from astropy.coordinates import EarthLocation
from modules.lithium.pytarget.target.calc_azalt import calculate_azimuth_altitude, calculate_star_info, calculate_rise_set_times

# FILE: modules/lithium.pytarget/target/test_calc_azalt.py


@pytest.fixture
def observer_location():
    return EarthLocation(lat=34.0522, lon=-118.2437, height=0)  # Los Angeles, CA

@pytest.fixture
def observation_datetime():
    return datetime.datetime(2023, 10, 1, 22, 0, 0, tzinfo=datetime.timezone.utc)  # 10 PM UTC on October 1, 2023

def test_calculate_azimuth_altitude_valid(observer_location, observation_datetime):
    ra = 10.684  # Right Ascension in degrees
    dec = 41.269  # Declination in degrees

    azimuth, altitude = calculate_azimuth_altitude(observation_datetime, observer_location, ra, dec)

    assert azimuth is not None
    assert altitude is not None

def test_calculate_azimuth_altitude_invalid_datetime(observer_location):
    ra = 10.684  # Right Ascension in degrees
    dec = 41.269  # Declination in degrees
    invalid_datetime = datetime.datetime(2023, 10, 1, 22, 0, 0)  # No timezone info

    azimuth, altitude = calculate_azimuth_altitude(invalid_datetime, observer_location, ra, dec)

    assert azimuth is None
    assert altitude is None

def test_calculate_star_info_valid(observer_location, observation_datetime):
    ra = 10.684  # Right Ascension in degrees
    dec = 41.269  # Declination in degrees

    star_info = calculate_star_info(observation_datetime, observer_location, ra, dec)

    assert len(star_info) > 0
    for entry in star_info:
        assert len(entry) == 3
        assert isinstance(entry[0], str)
        assert isinstance(entry[1], float)
        assert isinstance(entry[2], float)

def test_calculate_star_info_invalid_datetime(observer_location):
    ra = 10.684  # Right Ascension in degrees
    dec = 41.269  # Declination in degrees
    invalid_datetime = datetime.datetime(2023, 10, 1, 22, 0, 0)  # No timezone info

    star_info = calculate_star_info(invalid_datetime, observer_location, ra, dec)

    assert len(star_info) == 0

def test_calculate_rise_set_times_valid(observer_location, observation_datetime):
    ra = 10.684  # Right Ascension in degrees
    dec = 41.269  # Declination in degrees

    rise_time, set_time = calculate_rise_set_times(observer_location, ra, dec, observation_datetime)

    assert rise_time is not None
    assert set_time is not None

def test_calculate_rise_set_times_invalid_datetime(observer_location):
    ra = 10.684  # Right Ascension in degrees
    dec = 41.269  # Declination in degrees
    invalid_datetime = datetime.datetime(2023, 10, 1, 22, 0, 0)  # No timezone info

    rise_time, set_time = calculate_rise_set_times(observer_location, ra, dec, invalid_datetime)

    assert rise_time is None
    assert set_time is None