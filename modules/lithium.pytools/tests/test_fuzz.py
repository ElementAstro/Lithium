import pytest
from fuzz import generate_data, FieldInfo

def test_generate_data_simple_schema():
    schema = {
        "name": FieldInfo(type="string", length=10),
        "age": FieldInfo(type="int", min=18, max=99)
    }
    num_records = 5
    data = generate_data(schema, num_records)
    
    assert len(data) == num_records
    for record in data:
        assert "name" in record
        assert "age" in record
        assert isinstance(record["name"], str)
        assert isinstance(record["age"], int)
        assert 18 <= record["age"] <= 99

def test_generate_data_various_field_types():
    schema = {
        "name": FieldInfo(type="string", length=10),
        "age": FieldInfo(type="int", min=18, max=99),
        "email": FieldInfo(type="email"),
        "phone": FieldInfo(type="phone"),
        "uuid": FieldInfo(type="uuid"),
        "date": FieldInfo(type="date", start_date="2020-01-01", end_date="2020-12-31"),
        "choice": FieldInfo(type="choice", choices=["A", "B", "C"]),
        "bool": FieldInfo(type="bool"),
        "ip": FieldInfo(type="ip"),
        "url": FieldInfo(type="url"),
        "address": FieldInfo(type="address"),
        "company": FieldInfo(type="company")
    }
    num_records = 3
    data = generate_data(schema, num_records)
    
    assert len(data) == num_records
    for record in data:
        assert "name" in record
        assert "age" in record
        assert "email" in record
        assert "phone" in record
        assert "uuid" in record
        assert "date" in record
        assert "choice" in record
        assert "bool" in record
        assert "ip" in record
        assert "url" in record
        assert "address" in record
        assert "company" in record

        assert isinstance(record["name"], str)
        assert isinstance(record["age"], int)
        assert isinstance(record["email"], str)
        assert isinstance(record["phone"], str)
        assert isinstance(record["uuid"], str)
        assert isinstance(record["date"], str)
        assert record["choice"] in ["A", "B", "C"]
        assert isinstance(record["bool"], bool)
        assert isinstance(record["ip"], str)
        assert isinstance(record["url"], str)
        assert isinstance(record["address"], str)
        assert isinstance(record["company"], str)

def test_generate_data_empty_schema():
    schema = {}
    num_records = 5
    data = generate_data(schema, num_records)
    
    assert len(data) == num_records
    for record in data:
        assert record == {}

def test_generate_data_zero_records():
    schema = {
        "name": FieldInfo(type="string", length=10),
        "age": FieldInfo(type="int", min=18, max=99)
    }
    num_records = 0
    data = generate_data(schema, num_records)
    
    assert len(data) == 0

def test_generate_data_invalid_schema():
    schema = {
        "name": FieldInfo(type="unknown")
    }
    num_records = 5
    with pytest.raises(ValueError):
        generate_data(schema, num_records)