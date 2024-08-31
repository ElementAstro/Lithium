import pytest
from astropy.coordinates import EarthLocation
import datetime


@pytest.fixture
def observer_location():
    return EarthLocation(lat=34.0522*u.deg, lon=-118.2437*u.deg, height=71*u.m)


@pytest.fixture
def observation_time():
    return datetime.datetime.now()


@pytest.fixture
def target_coordinates():
    return 10.684, 41.269  # Example coordinates (RA, Dec in degrees)


def test_calculate_current_alt(observer_location, observation_time, target_coordinates):
    ra, dec = target_coordinates
    result = calculate_current_alt(
        observation_time, observer_location, ra, dec)

    assert result.altitude is not None
    assert result.azimuth is not None
    assert isinstance(result.is_above_horizon, bool)


def test_calculate_highest_alt(observer_location, observation_time, target_coordinates):
    ra, dec = target_coordinates
    highest_alt = calculate_highest_alt(
        observation_time, observer_location, ra, dec)

    assert highest_alt is not None
    assert highest_alt >= 0


def test_calculate_available_shoot_time(observer_location, observation_time, target_coordinates):
    ra, dec = target_coordinates
    available_shoot_time = calculate_available_shoot_time(
        observation_time, observer_location, ra, dec)

    assert available_shoot_time is not None
    assert available_shoot_time >= 0
