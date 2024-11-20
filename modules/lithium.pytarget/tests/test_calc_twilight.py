import pytest
import datetime
from astroplan import Observer
from astropy.time import Time
from astropy.coordinates import EarthLocation
from zoneinfo import ZoneInfo
from modules.lithium.pytarget.target.calc_twilight import calculate_twilight

# FILE: modules/lithium.pytarget/target/test_calc_twilight.py


@pytest.fixture
def observer_location():
    return EarthLocation(lat=34.0522, lon=-118.2437, height=0)  # Los Angeles, CA

@pytest.fixture
def observation_time():
    return Time(datetime.datetime(2023, 10, 1, 22, 0, 0, tzinfo=datetime.timezone.utc))  # 10 PM UTC on October 1, 2023

@pytest.fixture
def time_offset():
    return ZoneInfo("America/Los_Angeles")

@pytest.fixture
def observer(observer_location):
    return Observer(location=observer_location)

def test_calculate_twilight_valid(observer, observation_time, time_offset):
    twilight_times = calculate_twilight(observer, observation_time, time_offset)

    assert 'evening' in twilight_times
    assert 'morning' in twilight_times
    assert 'sun_set_time' in twilight_times['evening']
    assert 'evening_civil_time' in twilight_times['evening']
    assert 'evening_nautical_time' in twilight_times['evening']
    assert 'evening_astro_time' in twilight_times['evening']
    assert 'sun_rise_time' in twilight_times['morning']
    assert 'morning_civil_time' in twilight_times['morning']
    assert 'morning_nautical_time' in twilight_times['morning']
    assert 'morning_astro_time' in twilight_times['morning']

def test_calculate_twilight_invalid_time(observer, time_offset):
    invalid_time = Time(datetime.datetime(2023, 10, 1, 22, 0, 0))  # No timezone info

    twilight_times = calculate_twilight(observer, invalid_time, time_offset)

    assert twilight_times == {}

def test_calculate_twilight_invalid_observer(observation_time, time_offset):
    invalid_observer = None

    twilight_times = calculate_twilight(invalid_observer, observation_time, time_offset)

    assert twilight_times == {}

def test_calculate_twilight_invalid_time_offset(observer, observation_time):
    invalid_time_offset = None

    twilight_times = calculate_twilight(observer, observation_time, invalid_time_offset)

    assert twilight_times == {}