import pytest
import datetime
from astropy.coordinates import EarthLocation
from modules.lithium.pytarget.target.calc_alt import calculate_current_alt, ObservationResult

# FILE: modules/lithium.pytarget/target/test_calc_alt.py


@pytest.fixture
def observer_location():
    return EarthLocation(lat=34.0522, lon=-118.2437, height=0)  # Los Angeles, CA

@pytest.fixture
def observation_start_time():
    return datetime.datetime(2023, 10, 1, 22, 0, 0)  # 10 PM on October 1, 2023

def test_calculate_current_alt_valid(observer_location, observation_start_time):
    ra = 10.684  # Right Ascension in degrees
    dec = 41.269  # Declination in degrees

    result = calculate_current_alt(observation_start_time, observer_location, ra, dec)

    assert isinstance(result, ObservationResult)
    assert result.altitude is not None
    assert result.azimuth is not None
    assert result.highest_altitude is not None
    assert result.available_shoot_time is not None
    assert isinstance(result.is_above_horizon, bool)

def test_calculate_current_alt_below_horizon(observer_location, observation_start_time):
    ra = 180.0  # Right Ascension in degrees
    dec = -90.0  # Declination in degrees (below horizon)

    result = calculate_current_alt(observation_start_time, observer_location, ra, dec)

    assert isinstance(result, ObservationResult)
    assert result.altitude is not None
    assert result.azimuth is not None
    assert result.highest_altitude is None
    assert result.available_shoot_time is None
    assert result.is_above_horizon is False

def test_calculate_current_alt_invalid_ra_dec(observer_location, observation_start_time):
    ra = 400.0  # Invalid Right Ascension
    dec = 100.0  # Invalid Declination

    result = calculate_current_alt(observation_start_time, observer_location, ra, dec)

    assert isinstance(result, ObservationResult)
    assert result.altitude is None
    assert result.azimuth is None
    assert result.highest_altitude is None
    assert result.available_shoot_time is None
    assert result.is_above_horizon is False

def test_calculate_current_alt_edge_case(observer_location, observation_start_time):
    ra = 0.0  # Right Ascension at edge
    dec = 0.0  # Declination at edge

    result = calculate_current_alt(observation_start_time, observer_location, ra, dec)

    assert isinstance(result, ObservationResult)
    assert result.altitude is not None
    assert result.azimuth is not None
    assert result.highest_altitude is not None
    assert result.available_shoot_time is not None
    assert isinstance(result.is_above_horizon, bool)