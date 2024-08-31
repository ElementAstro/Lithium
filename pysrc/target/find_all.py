from astropy.coordinates import EarthLocation
import pandas as pd
from pathlib import Path
import numpy as np
from astropy.coordinates import AltAz, SkyCoord
from astropy.time import Time
import datetime
import re
from pydantic import BaseModel, field_validator
from typing import List, Optional, Dict, Tuple
from loguru import logger

from calc_alt import calculate_star_info

# File paths
this_file_path = Path(__file__).resolve().parent
full_list_file = this_file_path / 'TargetListAll.csv'

too_big_search_string = [
    'Minkowski', 'M', 'NGC', 'PGC', 'Sh', 'UGC', 'vdB', 'IC'
]


class DSO(BaseModel):
    """Data model for Deep Sky Object (DSO)."""
    name: str
    ra: Optional[float] = None  # 赤经 (0 <= ra <= 360)
    dec: Optional[float] = None  # 赤纬 (-90 <= dec <= 90)
    alias: Optional[str] = None
    altitude_curve: Optional[List[float]] = None
    azimuth_curve: Optional[List[float]] = None

    @field_validator('ra', mode='before')
    def validate_ra(cls, v, info):
        """Validate that RA is within the range 0 to 360 degrees."""
        if v is not None and not (0 <= v <= 360):
            raise ValueError(
                f"{info.field_name.capitalize()} value {v} out of range (0 <= value <= 360).")
        return v

    @field_validator('dec', mode='before')
    def validate_dec(cls, v, info):
        """Validate that Dec is within the range -90 to 90 degrees."""
        if v is not None and not (-90 <= v <= 90):
            raise ValueError(
                f"{info.field_name.capitalize()} value {v} out of range (-90 <= value <= 90).")
        return v


def setup_logging():
    """
    Set up the loguru logging configuration to log both to console and to a file.
    """
    logger.add("dso_search.log", level="DEBUG",
               format="{time} {level} {message}", rotation="10 MB")
    logger.info("Logging setup complete.")


def read_csv(file_path: Path) -> pd.DataFrame:
    """Read the CSV file and handle potential errors."""
    try:
        logger.info(f"Reading CSV file from {file_path}")
        df = pd.read_csv(file_path)
        df = df.replace({np.nan: None})
        return df
    except Exception as e:
        logger.error(f"Error reading the CSV file at {file_path}: {e}")
        raise FileNotFoundError(
            f"Error reading the CSV file at {file_path}: {e}")


def sort_search_result(search_result: List[DSO]) -> List[DSO]:
    """Sort the search results based on priority (NGC, M, IC first)."""
    logger.debug("Sorting search results based on priority.")
    first_prior = [res for res in search_result if any(
        prefix in res.name for prefix in ('NGC', 'M ', 'IC'))]
    second_prior = [res for res in search_result if res not in first_prior]
    return first_prior + second_prior


def search_DSO(to_search_name: str, observer_location: EarthLocation, date_time_string: datetime.datetime) -> List[DSO]:
    logger.info(
        f"Starting DSO search for {to_search_name} at {date_time_string}")
    df = read_csv(full_list_file)

    results = None
    if to_search_name.isdigit():
        pattern = rf"\b0*{to_search_name}\b"
        results = df[df['name'].str.contains(pattern, na=False, regex=True)]
    elif to_search_name.isalpha():
        if len(to_search_name) > 1 and to_search_name not in too_big_search_string:
            results = df[(df['name'].str.contains(to_search_name, na=False)) |
                         (df['alias'].str.contains(to_search_name, case=False, na=False))]
    else:
        match = re.match(r"([a-z]+)([0-9]+)", to_search_name, re.I)
        if match:
            letters, digit = match.groups()
            letters = letters.lower()
            pattern = rf"\b0*{digit}\b"
            mask_letter = df['name'].str.lower(
            ).str.contains(letters, na=False)
            mask_digit = df['name'].str.contains(pattern, na=False, regex=True)
            results = df[mask_letter & mask_digit]

    if results is not None and not results.empty:
        result_dicts = results.to_dict('records')
        dsos = [DSO(**res) for res in result_dicts]
        dsos = sort_search_result(dsos)

        for dso in dsos:
            if dso.ra and dso.dec:
                dso.altitude_curve, dso.azimuth_curve = calculate_alt_and_az_curve(
                    dso.ra, dso.dec, observer_location, date_time_string)
                logger.debug(f"Calculated Alt/Az for {dso.name}")

        logger.info(f"Found {len(dsos)} matching DSOs.")
        return dsos
    else:
        logger.info("No matching DSOs found.")
        return []


def calculate_alt_and_az_curve(ra: float, dec: float, observer_location: EarthLocation,
                               date_time_string: datetime.datetime) -> Tuple[List[float], List[float]]:
    """Calculate the altitude and azimuth curve for given coordinates."""
    logger.debug(
        f"Calculating altitude and azimuth curve for RA: {ra}, Dec: {dec}")
    try:
        # Convert datetime to astropy Time object
        time = Time(date_time_string)

        # Create SkyCoord object
        sky_coord = SkyCoord(ra=ra, dec=dec, unit="deg")

        # Calculate AltAz object for each hour in the given datetime
        altaz_frame = AltAz(obstime=time, location=observer_location)
        altaz = sky_coord.transform_to(altaz_frame)

        # Extract altitude and azimuth
        altitude_curve = altaz.alt.deg
        azimuth_curve = altaz.az.deg

        logger.debug(f"Altitude: {altitude_curve}, Azimuth: {azimuth_curve}")

        return altitude_curve, azimuth_curve
    except Exception as e:
        logger.error(f"Error calculating altitude and azimuth curve: {e}")
        return [], []


def batch_search_DSO(names: List[str], observer_location: EarthLocation, date_time_string: datetime.datetime) -> Dict[str, List[DSO]]:
    """Search multiple DSOs in batch mode."""
    logger.info(f"Starting batch search for {len(names)} DSOs.")
    results = {}
    for name in names:
        results[name] = search_DSO(name, observer_location, date_time_string)
    logger.info("Batch search completed.")
    return results


# 调用示例
if __name__ == "__main__":
    # 设置日志
    setup_logging()

    # 定义观测者位置和时间
    observer_location = EarthLocation(
        lat=34.0522, lon=-118.2437, height=100)  # 例如洛杉矶
    date_time_string = datetime.datetime(
        2024, 8, 29, 22, 0, 0)  # 2024年8月29日22点

    # 单一DSO搜索
    dso_name = "M42"
    dso_results = search_DSO(dso_name, observer_location, date_time_string)
    for dso in dso_results:
        print(
            f"Name: {dso.name}, Altitude Curve: {dso.altitude_curve}, Azimuth Curve: {dso.azimuth_curve}")
