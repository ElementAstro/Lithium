import pytest
import datetime
from astropy.coordinates import EarthLocation
from zoneinfo import ZoneInfo
from modules.lithium.pytarget.target.find_named import get_light_star_list, SimpleLightStarInfo, CardinalDirection

# FILE: modules/lithium.pytarget/target/test_find_named.py


@pytest.fixture
def observer_location():
    return EarthLocation(lat=34.0522, lon=-118.2437, height=0)  # Los Angeles, CA

@pytest.fixture
def observation_time():
    return datetime.datetime(2023, 10, 1, 22, 0, 0, tzinfo=ZoneInfo("UTC"))  # 10 PM UTC on October 1, 2023

def test_get_light_star_list_valid(observer_location, observation_time):
    max_magnitude = 2
    result = get_light_star_list(observer_location, observation_time, max_magnitude)

    assert isinstance(result, list)
    assert all(isinstance(star, dict) for star in result)
    assert len(result) > 0

def test_get_light_star_list_no_results(observer_location, observation_time):
    max_magnitude = 0  # No stars should match this magnitude
    result = get_light_star_list(observer_location, observation_time, max_magnitude)

    assert isinstance(result, list)
    assert len(result) == 0

def test_get_light_star_list_with_filters(observer_location, observation_time):
    max_magnitude = 2
    range_filter = [CardinalDirection.NORTH.value]
    constellation_filter = "And"
    constellation_zh_filter = "仙女座"
    name_filter = "Andromeda"
    show_name_filter = "仙女座"

    result = get_light_star_list(observer_location, observation_time, max_magnitude, range_filter,
                                 constellation_filter, constellation_zh_filter, name_filter, show_name_filter)

    assert isinstance(result, list)
    assert all(isinstance(star, dict) for star in result)
    assert len(result) > 0

def test_get_light_star_list_invalid_range_filter(observer_location, observation_time):
    max_magnitude = 2
    range_filter = ["invalid_direction"]

    result = get_light_star_list(observer_location, observation_time, max_magnitude, range_filter)

    assert isinstance(result, list)
    assert len(result) > 0  # Should ignore the invalid filter and return results

def test_get_light_star_list_edge_case(observer_location, observation_time):
    max_magnitude = 4  # Edge case for maximum magnitude
    result = get_light_star_list(observer_location, observation_time, max_magnitude)

    assert isinstance(result, list)
    assert all(isinstance(star, dict) for star in result)
    assert len(result) > 0

def test_calculate_light_star_info(observer_location, observation_time):
    star = SimpleLightStarInfo(
        name="Test Star", show_name="测试星", ra=10.684, dec=41.269, constellation="And", constellation_zh="仙女座", magnitude=2.0
    )
    star.update_alt_az(45.0, 90.0)

    assert star.alt == 45.0
    assert star.az == 90.0
    assert star.sky == "east"