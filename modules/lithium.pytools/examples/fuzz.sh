# Example 1: Generate random data and save as JSON
# This example generates 10 records based on the provided schema and saves the data to a JSON file.
$ python fuzz.py --num_records 10 --schema '{"name": {"type": "string"}, "age": {"type": "int"}}' --format json --output data.json

# Example 2: Generate random data and save as CSV
# This example generates 20 records based on the provided schema and saves the data to a CSV file.
$ python fuzz.py --num_records 20 --schema '{"email": {"type": "email"}, "phone": {"type": "phone"}}' --format csv --output data.csv

# Example 3: Generate random data and save as text
# This example generates 5 records based on the provided schema and saves the data to a text file.
$ python fuzz.py --num_records 5 --schema '{"uuid": {"type": "uuid"}, "date": {"type": "date"}}' --format text --output data.txt

# Example 4: Generate random data with custom ranges and save as JSON
# This example generates 15 records with custom ranges for integer and float fields and saves the data to a JSON file.
$ python fuzz.py --num_records 15 --schema '{"score": {"type": "int", "min": 50, "max": 100}, "price": {"type": "float", "min": 10.0, "max": 50.0, "precision": 2}}' --format json --output data.json

# Example 5: Generate random data with choices and save as CSV
# This example generates 10 records with a choice field and saves the data to a CSV file.
$ python fuzz.py --num_records 10 --schema '{"status": {"type": "choice", "choices": ["active", "inactive", "pending"]}}' --format csv --output data.csv

# Example 6: Generate random data with boolean and IP fields and save as text
# This example generates 8 records with boolean and IP fields and saves the data to a text file.
$ python fuzz.py --num_records 8 --schema '{"is_active": {"type": "bool"}, "ip_address": {"type": "ip"}}' --format text --output data.txt

# Example 7: Generate random data with URL field and save as JSON
# This example generates 12 records with a URL field and saves the data to a JSON file.
$ python fuzz.py --num_records 12 --schema '{"website": {"type": "url"}}' --format json --output data.json

# Example 8: Generate random data with date range and save as CSV
# This example generates 10 records with a date field within a specified range and saves the data to a CSV file.
$ python fuzz.py --num_records 10 --schema '{"event_date": {"type": "date", "start_date": "2022-01-01", "end_date": "2023-01-01"}}' --format csv --output data.csv

# Example 9: Generate random data and display in terminal
# This example generates 5 records based on the provided schema, saves the data to a JSON file, and displays the data in the terminal.
$ python fuzz.py --num_records 5 --schema '{"name": {"type": "string"}, "email": {"type": "email"}}' --format json --output data.json --display

# Example 10: Generate random data with nested schema and save as JSON
# This example generates 10 records with a nested schema and saves the data to a JSON file.
$ python fuzz.py --num_records 10 --schema '{"user": {"type": "string"}, "profile": {"type": "choice", "choices": [{"age": {"type": "int"}, "email": {"type": "email"}}]}}' --format json --output data.json