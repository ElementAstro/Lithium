import pytest
import datetime
from astropy.coordinates import EarthLocation
from modules.lithium.pytarget.target.find_all import search_DSO, batch_search_DSO, DSO

# FILE: modules/lithium.pytarget/target/test_find_all.py


@pytest.fixture
def observer_location():
    return EarthLocation(lat=34.0522, lon=-118.2437, height=0)  # Los Angeles, CA

@pytest.fixture
def date_time_string():
    return datetime.datetime(2023, 10, 1, 22, 0, 0)  # 10 PM on October 1, 2023

def test_search_DSO_valid(observer_location, date_time_string):
    to_search_name = "M31"  # Andromeda Galaxy
    results = search_DSO(to_search_name, observer_location, date_time_string)

    assert isinstance(results, list)
    assert all(isinstance(dso, DSO) for dso in results)
    assert len(results) > 0

def test_search_DSO_no_results(observer_location, date_time_string):
    to_search_name = "NonExistentObject"
    results = search_DSO(to_search_name, observer_location, date_time_string)

    assert isinstance(results, list)
    assert len(results) == 0

def test_search_DSO_with_filters(observer_location, date_time_string):
    to_search_name = "M31"  # Andromeda Galaxy
    magnitude_range = (3.0, 5.0)
    ra_range = (0.0, 50.0)
    dec_range = (30.0, 50.0)
    alias = "Andromeda"

    results = search_DSO(to_search_name, observer_location, date_time_string,
                         magnitude_range=magnitude_range, ra_range=ra_range, dec_range=dec_range, alias=alias)

    assert isinstance(results, list)
    assert all(isinstance(dso, DSO) for dso in results)
    assert len(results) > 0

def test_batch_search_DSO_valid(observer_location, date_time_string):
    names = ["M31", "M42"]  # Andromeda Galaxy and Orion Nebula
    results = batch_search_DSO(names, observer_location, date_time_string)

    assert isinstance(results, dict)
    assert all(isinstance(name, str) for name in results.keys())
    assert all(isinstance(dsos, list) for dsos in results.values())
    assert all(all(isinstance(dso, DSO) for dso in dsos) for dsos in results.values())

def test_batch_search_DSO_no_results(observer_location, date_time_string):
    names = ["NonExistentObject1", "NonExistentObject2"]
    results = batch_search_DSO(names, observer_location, date_time_string)

    assert isinstance(results, dict)
    assert all(isinstance(name, str) for name in results.keys())
    assert all(isinstance(dsos, list) for dsos in results.values())
    assert all(len(dsos) == 0 for dsos in results.values())

def test_batch_search_DSO_with_filters(observer_location, date_time_string):
    names = ["M31", "M42"]  # Andromeda Galaxy and Orion Nebula
    magnitude_range = (3.0, 5.0)
    ra_range = (0.0, 50.0)
    dec_range = (30.0, 50.0)
    alias = "Andromeda"

    results = batch_search_DSO(names, observer_location, date_time_string,
                               magnitude_range=magnitude_range, ra_range=ra_range, dec_range=dec_range, alias=alias)

    assert isinstance(results, dict)
    assert all(isinstance(name, str) for name in results.keys())
    assert all(isinstance(dsos, list) for dsos in results.values())
    assert all(all(isinstance(dso, DSO) for dso in dsos) for dsos in results.values())
    assert len(results["M31"]) > 0
    assert len(results["M42"]) > 0