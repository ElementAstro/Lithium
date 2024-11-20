import re
import pytest
import datetime
from modules.lithium.pytarget.target.planet import Moon

# FILE: modules/lithium.pytarget/target/test_planet.py


@pytest.fixture
def moon():
    # San Francisco, CA
    return Moon(lat="37.7749", lon="-122.4194", elevation=10)


def test_get_next_full(moon):
    next_full = moon.get_next_full
    assert isinstance(next_full, datetime.date)


def test_get_next_new(moon):
    next_new = moon.get_next_new
    assert isinstance(next_new, datetime.date)


def test_get_previous_full(moon):
    previous_full = moon.get_previous_full
    assert isinstance(previous_full, datetime.date)


def test_get_previous_new(moon):
    previous_new = moon.get_previous_new
    assert isinstance(previous_new, datetime.date)


def test_get_next_last_quarter(moon):
    next_last_quarter = moon.get_next_last_quarter
    assert isinstance(next_last_quarter, datetime.date)


def test_get_next_first_quarter(moon):
    next_first_quarter = moon.get_next_first_quarter
    assert isinstance(next_first_quarter, datetime.date)


def test_get_previous_last_quarter(moon):
    previous_last_quarter = moon.get_previous_last_quarter
    assert isinstance(previous_last_quarter, datetime.date)


def test_get_previous_first_quarter(moon):
    previous_first_quarter = moon.get_previous_first_quarter
    assert isinstance(previous_first_quarter, datetime.date)


def test_get_moon_phase(moon):
    moon_phase = moon.get_moon_phase()
    assert isinstance(moon_phase, str)
    assert moon_phase in ["Full", "New", "First Quarter", "Last Quarter",
                          "Waxing Crescent", "Waxing Gibbous", "Waning Gibbous", "Waning Crescent"]


def test_get_moon_ra(moon):
    moon_ra = moon.get_moon_ra()
    assert isinstance(moon_ra, str)


def test_get_moon_dec(moon):
    moon_dec = moon.get_moon_dec()
    assert isinstance(moon_dec, str)


def test_get_moon_az(moon):
    moon_az = moon.get_moon_az()
    assert isinstance(moon_az, str)
    assert moon_az.endswith("°")


def test_get_moon_alt(moon):
    moon_alt = moon.get_moon_alt()
    assert isinstance(moon_alt, str)
    assert moon_alt.endswith("°")


def test_get_moon_rise(moon):
    moon_rise = moon.get_moon_rise()
    assert isinstance(moon_rise, str)
    assert re.match(r"\d{2}:\d{2}:\d{2}", moon_rise) or moon_rise == "-"


def test_get_moon_set(moon):
    moon_set = moon.get_moon_set()
    assert isinstance(moon_set, str)
    assert re.match(r"\d{2}:\d{2}:\d{2}", moon_set) or moon_set == "-"


def test_get_moon_transit(moon):
    moon_transit = moon.get_moon_transit()
    assert isinstance(moon_transit, str)
    assert re.match(r"\d{2}:\d{2}:\d{2}", moon_transit) or moon_transit == "-"
