import argparse
import random
import string
import json
import csv
import uuid
from datetime import datetime, timedelta
from typing import Any, Dict, List, Optional
from dataclasses import dataclass, field
from loguru import logger
from rich.console import Console
from rich.table import Table
from rich.progress import Progress
from pathlib import Path

console = Console()

# Configure Loguru to handle logging with specified settings
logger.add("fuzz.log", rotation="1 MB", level="DEBUG",
           backtrace=True, diagnose=True)


@dataclass
class FieldInfo:
    """
    Data class to store information about each field in the schema.

    Attributes:
        type (str): The type of the field (e.g., int, float, string).
        min (Optional[int]): The minimum value for numerical fields.
        max (Optional[int]): The maximum value for numerical fields.
        precision (Optional[int]): The precision for float values.
        length (Optional[int]): The length of generated strings.
        start_date (Optional[str]): The start date for date fields.
        end_date (Optional[str]): The end date for date fields.
        choices (Optional[List[Any]]): A list of choices for choice fields.
    """
    type: str
    min: Optional[int] = 0
    max: Optional[int] = 100
    precision: Optional[int] = 2
    length: Optional[int] = 8
    start_date: Optional[str] = "2000-01-01"
    end_date: Optional[str] = "2030-01-01"
    choices: Optional[List[Any]] = field(default_factory=list)


def random_int(min_val: int = 0, max_val: int = 100) -> int:
    """
    Generate a random integer between min_val and max_val.

    Args:
        min_val (int): The minimum integer value.
        max_val (int): The maximum integer value.

    Returns:
        int: A randomly generated integer within the specified range.
    """
    value = random.randint(min_val, max_val)
    logger.debug(f"Generated integer: {value} between {min_val} and {max_val}")
    return value


def random_float(min_val: float = 0.0, max_val: float = 100.0, precision: int = 2) -> float:
    """
    Generate a random float between min_val and max_val with specified precision.

    Args:
        min_val (float): The minimum float value.
        max_val (float): The maximum float value.
        precision (int): The number of decimal places.

    Returns:
        float: A randomly generated float within the specified range and precision.
    """
    value = round(random.uniform(min_val, max_val), precision)
    logger.debug(
        f"Generated float: {value} between {min_val} and {max_val} with precision {precision}")
    return value


def random_string(length: int = 8) -> str:
    """
    Generate a random string of specified length.

    Args:
        length (int): The length of the generated string.

    Returns:
        str: A randomly generated alphanumeric string.
    """
    value = ''.join(random.choices(
        string.ascii_letters + string.digits, k=length))
    logger.debug(f"Generated string: {value} with length {length}")
    return value


def random_email() -> str:
    """
    Generate a random email address.

    Returns:
        str: A randomly generated email address.
    """
    domains = ["example.com", "test.com", "email.com"]
    value = f"{random_string(5)}@{random.choice(domains)}"
    logger.debug(f"Generated email: {value}")
    return value


def random_phone() -> str:
    """
    Generate a random phone number in the format +XX-XXX-XXXX.

    Returns:
        str: A randomly generated phone number.
    """
    value = f"+{random.randint(1, 99)}-{random.randint(100, 999)}-{random.randint(1000, 9999)}"
    logger.debug(f"Generated phone: {value}")
    return value


def random_uuid_str() -> str:
    """
    Generate a random UUID string.

    Returns:
        str: A randomly generated UUID.
    """
    value = str(uuid.uuid4())
    logger.debug(f"Generated UUID: {value}")
    return value


def random_date(start_date: str = "2000-01-01", end_date: str = "2030-01-01") -> str:
    """
    Generate a random date between start_date and end_date.

    Args:
        start_date (str): The start date in YYYY-MM-DD format.
        end_date (str): The end date in YYYY-MM-DD format.

    Returns:
        str: A randomly generated date string in YYYY-MM-DD format.
    """
    start = datetime.strptime(start_date, "%Y-%m-%d")
    end = datetime.strptime(end_date, "%Y-%m-%d")
    delta = end - start
    random_days = random.randint(0, delta.days)
    value = (start + timedelta(days=random_days)).strftime("%Y-%m-%d")
    logger.debug(
        f"Generated date: {value} between {start_date} and {end_date}")
    return value


def random_choice(choices: List[Any]) -> Any:
    """
    Randomly select an element from a list of choices.

    Args:
        choices (List[Any]): A list of possible choices.

    Returns:
        Any: A randomly selected element from the choices list.
    """
    value = random.choice(choices)
    logger.debug(f"Selected choice: {value} from {choices}")
    return value


def random_bool() -> bool:
    """
    Generate a random boolean value.

    Returns:
        bool: A randomly generated boolean value (True or False).
    """
    value = random.choice([True, False])
    logger.debug(f"Generated boolean: {value}")
    return value


def random_ip() -> str:
    """
    Generate a random IPv4 address.

    Returns:
        str: A randomly generated IPv4 address.
    """
    value = '.'.join(str(random.randint(0, 255)) for _ in range(4))
    logger.debug(f"Generated IP: {value}")
    return value


def random_url() -> str:
    """
    Generate a random URL.

    Returns:
        str: A randomly generated URL.
    """
    protocols = ["http", "https"]
    domains = ["example.com", "test.com", "mysite.com"]
    value = f"{random.choice(protocols)}://{random_string(5)}.{random.choice(domains)}"
    logger.debug(f"Generated URL: {value}")
    return value


def random_address() -> str:
    """
    Generate a random address.

    Returns:
        str: A randomly generated address.
    """
    streets = ["Main St", "Second St", "Third St", "Fourth St"]
    cities = ["New York", "Los Angeles", "Chicago", "Houston"]
    value = f"{random.randint(100, 9999)} {random.choice(streets)}, {random.choice(cities)}"
    logger.debug(f"Generated address: {value}")
    return value


def random_company() -> str:
    """
    Generate a random company name.

    Returns:
        str: A randomly generated company name.
    """
    prefixes = ["Tech", "Info", "Data", "Global"]
    suffixes = ["Solutions", "Systems", "Corp", "Industries"]
    value = f"{random.choice(prefixes)} {random.choice(suffixes)}"
    logger.debug(f"Generated company name: {value}")
    return value


def generate_data(schema: Dict[str, FieldInfo], num_records: int) -> List[Dict[str, Any]]:
    """
    Generate data based on the provided schema and number of records.

    Args:
        schema (Dict[str, FieldInfo]): A dictionary defining the schema for data generation.
        num_records (int): The number of records to generate.

    Returns:
        List[Dict[str, Any]]: A list of dictionaries representing the generated data records.
    """
    data = []
    with Progress() as progress:
        task = progress.add_task("Generating data...", total=num_records)
        for _ in range(num_records):
            record = {}
            for field, field_info in schema.items():
                field_type = field_info.type
                if field_type == "int":
                    record[field] = random_int(field_info.min, field_info.max)
                elif field_type == "float":
                    record[field] = random_float(
                        field_info.min, field_info.max, field_info.precision)
                elif field_type == "string":
                    record[field] = random_string(field_info.length)
                elif field_type == "email":
                    record[field] = random_email()
                elif field_type == "phone":
                    record[field] = random_phone()
                elif field_type == "uuid":
                    record[field] = random_uuid_str()
                elif field_type == "date":
                    record[field] = random_date(
                        field_info.start_date, field_info.end_date)
                elif field_type == "choice":
                    record[field] = random_choice(field_info.choices)
                elif field_type == "bool":
                    record[field] = random_bool()
                elif field_type == "ip":
                    record[field] = random_ip()
                elif field_type == "url":
                    record[field] = random_url()
                elif field_type == "address":
                    record[field] = random_address()
                elif field_type == "company":
                    record[field] = random_company()
                else:
                    logger.error(f"Unsupported field type: {field_type}")
                    raise ValueError(f"Unsupported field type: {field_type}")
            data.append(record)
            progress.advance(task)
    return data


def save_to_file(data: List[Dict[str, Any]], output_format: str, output_file: Path, delimiter: str = ',', encoding: str = 'utf-8') -> None:
    """
    Save the generated data to a file in the specified format.

    Args:
        data (List[Dict[str, Any]]): The data to be saved.
        output_format (str): The format to save the data in ('json', 'csv', 'text').
        output_file (Path): The path to the output file.
        delimiter (str, optional): The delimiter to use for CSV files. Defaults to ','.
        encoding (str, optional): The file encoding. Defaults to 'utf-8'.

    Raises:
        ValueError: If an unsupported output format is specified.
    """
    output_file = Path(output_file)
    try:
        if output_format == "json":
            with output_file.open("w", encoding=encoding) as f:
                json.dump(data, f, indent=4)
        elif output_format == "csv":
            with output_file.open("w", newline='', encoding=encoding) as f:
                writer = csv.DictWriter(
                    f, fieldnames=data[0].keys(), delimiter=delimiter)
                writer.writeheader()
                writer.writerows(data)
        elif output_format == "text":
            with output_file.open("w", encoding=encoding) as f:
                for record in data:
                    f.write(str(record) + "\n")
        else:
            logger.error(f"Unsupported output format: {output_format}")
            raise ValueError(f"Unsupported output format: {output_format}")
        logger.info(f"Data saved to {output_file} in {output_format} format.")
    except Exception as e:
        logger.error(f"Failed to save data: {e}")
        raise


def display_data(data: List[Dict[str, Any]]) -> None:
    """
    Display the generated data in a table format using Rich.

    Args:
        data (List[Dict[str, Any]]): The data to be displayed.
    """
    if data:
        table = Table(title="Generated Data", show_lines=True)
        for key in data[0].keys():
            table.add_column(key, style="cyan", no_wrap=True)
        for item in data:
            table.add_row(*[str(value) for value in item.values()])
        console.print(table)
    else:
        logger.info("No data to display.")


def parse_schema(schema_string: str) -> Dict[str, FieldInfo]:
    """
    Parse the schema string into a dictionary of FieldInfo objects.

    Args:
        schema_string (str): The schema in JSON string format.

    Returns:
        Dict[str, FieldInfo]: A dictionary mapping field names to their FieldInfo.

    Raises:
        ValueError: If the schema string is invalid or missing required keys.
    """
    try:
        raw_schema = json.loads(schema_string)
        schema = {}
        for field, field_info in raw_schema.items():
            if not isinstance(field_info, dict):
                logger.error(f"Field {field} should be a JSON object.")
                raise ValueError(f"Field {field} should be a JSON object.")
            if "type" not in field_info:
                logger.error(f"Field {field} is missing a 'type'.")
                raise ValueError(f"Field {field} is missing a 'type'.")
            schema[field] = FieldInfo(**field_info)
        logger.debug(f"Parsed schema: {schema}")
        return schema
    except json.JSONDecodeError as e:
        logger.error(f"Invalid JSON schema: {e}")
        raise ValueError(f"Invalid JSON schema: {e}")


def main() -> None:
    """
    Main function to parse command-line arguments and generate data accordingly.
    """
    parser = argparse.ArgumentParser(description="Random Data Generator Tool")
    parser.add_argument(
        "-n", "--num_records", type=int, required=True, help="Number of records to generate"
    )
    parser.add_argument(
        "-s",
        "--schema",
        type=str,
        required=True,
        help='Schema as a JSON string (e.g., \'{"name": {"type": "string"}, "age": {"type": "int"}}\')',
    )
    parser.add_argument(
        "-f",
        "--format",
        choices=["json", "csv", "text"],
        required=True,
        help="Output format",
    )
    parser.add_argument(
        "-o", "--output", type=str, required=True, help="Output file name"
    )
    parser.add_argument(
        "--display", action="store_true", help="Display the generated data in the terminal"
    )
    parser.add_argument(
        "--delimiter", type=str, default=',', help="Delimiter for CSV files"
    )
    parser.add_argument(
        "--encoding", type=str, default='utf-8', help="File encoding"
    )

    args = parser.parse_args()

    try:
        # Parse the schema from the provided JSON string
        schema = parse_schema(args.schema)
        # Generate the data based on the schema and number of records
        data = generate_data(schema, args.num_records)
        # Save the generated data to the specified file and format
        save_to_file(data, args.format, args.output,
                     args.delimiter, args.encoding)
        logger.success(
            f"Generated {args.num_records} records and saved to {args.output} in {args.format} format."
        )
        # If display flag is set, display the data in the terminal
        if args.display:
            display_data(data)
    except Exception as e:
        # Log and display any errors that occur during the process
        logger.error(f"Error: {e}")
        console.print(f"[red]Error: {e}[/red]")


if __name__ == "__main__":
    main()
