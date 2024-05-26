# coding=utf-8

"""
Copyright(c) 2022-2023 Max Qian  <lightapt.com>
"""

import asyncio
from pathlib import Path
from typing import Union, Optional, Dict
import traceback
import logging
import subprocess
from dataclasses import dataclass
from astropy.io import fits
from astropy.coordinates import Angle
import astropy.units as u

logger = logging.getLogger(__name__)

class SolverError(Exception):
    """Custom exception for solver errors."""

async def solve(
    image: Union[str, Path],
    ra: Optional[float] = None,
    dec: Optional[float] = None,
    radius: Optional[Union[float, str]] = None,
    fov: Optional[float] = None,
    downsample: Optional[int] = None,
    debug: bool = False,
    update: bool = False,
    max_number_star: int = 500,
    tolerance: float = 0.007,
    wcs: bool = True,
    timeout: int = 60
) -> Dict[str, Optional[Union[float, str]]]:
    """
    Solve the given image with the parameters.

    Args:
        image (Union[str, Path]): Path to the image file or just the image.
        ra (Optional[float]): Right ascension in degrees.
        dec (Optional[float]): Declination in degrees.
        radius (Optional[Union[float, str]]): Search radius.
        fov (Optional[float]): Field of view.
        downsample (Optional[int]): Downsample factor (1, 2, 4, 0 means auto).
        debug (bool): If True, enable debug mode for more output.
        update (bool): If True, write a new FITS image if the image type is not FITS.
        max_number_star (int): Maximum number of stars to consider.
        tolerance (float): Tolerance for star matching.
        wcs (bool): If True, write a WCS file like astrometry.
        timeout (int): Timeout in seconds.

    Returns:
        Dict[str, Optional[Union[float, str]]]: Solution results.
    """
    ret_struct = {
        'ra': None,
        'dec': None,
        'fov': None,
        'message': None,
        'rota_x': None,
        'rota_y': None,
    }

    if not isinstance(image, (str, Path)):
        ret_struct['message'] = 'Invalid image file type'
        return ret_struct
    image = Path(image)

    if not image.exists():
        ret_struct['message'] = 'File does not exist'
        return ret_struct

    command = ["astap_cli"]

    # Check arguments
    if ra is not None:
        command.extend(['-ra', str(ra)])
    if dec is not None:
        command.extend(['-spd', str(dec + 90)])
    if radius is not None:
        command.extend(['-r', str(radius)])
    if fov is not None:
        command.extend(['-fov', str(fov)])
    if downsample is not None:
        command.extend(['-z', str(downsample)])
    if debug:
        command.append('-debug')
    if update:
        command.append('-update')
    if wcs:
        command.append('-wcs')

    # Add a limit of the max number of stars
    command.extend(['-s', str(max_number_star)])
    # Tolerance
    command.extend(['-t', str(tolerance)])
    command.extend(["-f", str(image)])

    command_str = ' '.join(command)
    logger.debug(f"Command line: {command_str}")

    try:
        astap = await asyncio.subprocess.create_subprocess_shell(
            command_str,
            stdin=asyncio.subprocess.PIPE,
            stdout=asyncio.subprocess.PIPE
        )
        stdout, stderr = await asyncio.wait_for(astap.communicate(), timeout=timeout)
    except asyncio.TimeoutError:
        ret_struct['message'] = 'Solve timeout'
        logger.error(f'Solve timeout with input {image}, {ra}, {dec}, {fov}')
        return ret_struct
    except Exception:
        logger.error(traceback.format_exc())
        ret_struct['message'] = 'Unexpected error'
        return ret_struct

    output_lines = stdout.decode().splitlines()

    for line in output_lines:
        if "Solution found:" in line:
            ra_dec = line.replace("Solution found: ", "").replace("  ", " ").replace(":", "")
            ra_h, ra_m, ra_s, dec_h, dec_m, dec_s = ra_dec.split(" ")
            ret_struct['ra'] = f"{ra_h}:{ra_m}:{ra_s}"
            ret_struct['dec'] = f"{dec_h}:{dec_m}:{dec_s}"
        elif "Set FOV=" in line:
            fov_value = line[line.index("Set FOV=") + 8:].split("d")[0]
            ret_struct['fov'] = fov_value

    if ret_struct['ra'] is None or ret_struct['dec'] is None:
        ret_struct['message'] = 'Solve failed'
        return ret_struct

    if wcs:
        # Read the WCS file to get the rotation information
        def cut(text: str, chunk_size: int) -> list[str]:
            return [text[i:i + chunk_size] for i in range(0, len(text), chunk_size)]

        wcs_path = image.with_suffix(".wcs")
        if not wcs_path.exists():
            ret_struct['message'] = 'No WCS file found'
            return ret_struct

        with wcs_path.open(encoding="utf-8") as file:
            wcs_data = cut(file.read(), 80)
            for line in wcs_data:
                if "CROTA1  =" in line:
                    ret_struct['rota_x'] = line.split("/")[0].split("=")[1].strip()
                if "CROTA2  =" in line:
                    ret_struct['rota_y'] = line.split("/")[0].split("=")[1].strip()

    return ret_struct

# coding=utf-8

"""
Copyright(c) 2022-2023 Max Qian  <lightapt.com>
"""



logger = logging.getLogger(__name__)

@dataclass
class SolveResult:
    """Class to store the result of the ASTAP solve operation."""
    ra: Optional[float] = None
    dec: Optional[float] = None
    rotation: Optional[float] = None
    focal_length: Optional[float] = None
    message: Optional[str] = None

def astap_solve_blocking(
    image: Union[str, Path],
    ra: Optional[float] = None,
    dec: Optional[float] = None,
    radius: Optional[float] = None,
    fov: Optional[float] = None,
    downsample: int = 4,
    debug: bool = False,
    update: bool = True,
    wcs: bool = False,
    timeout: int = 60
) -> Dict[str, Optional[Union[float, str]]]:
    """
    Solve the given image with the specified parameters using ASTAP.

    Args:
        image (Union[str, Path]): Path to the image file.
        ra (Optional[float]): Current RA in degrees (converted to hours for ASTAP).
        dec (Optional[float]): Current Dec in degrees.
        radius (Optional[float]): Search radius.
        fov (Optional[float]): Field of view.
        downsample (int): Downsample factor (1, 2, 4, 0 means auto).
        debug (bool): Enable debug mode.
        update (bool): Write a new FITS image if the image type is not FITS.
        wcs (bool): Write a WCS file like astrometry.
        timeout (int): Timeout in seconds.

    Returns:
        Dict[str, Optional[Union[float, str]]]: Solve results.
    """
    result = SolveResult(message='Solve failed')

    if not isinstance(image, (str, Path)):
        result.message = "Invalid image file type"
        return result.__dict__
    image = Path(image)

    if not image.exists():
        result.message = "File does not exist"
        return result.__dict__

    command = ["/usr/local/bin/astap_cli"]

    # Add optional parameters to the command
    if ra is not None:
        command.extend(['-ra', str(ra / 360 * 24)])
    if dec is not None:
        command.extend(['-spd', str(dec + 90)])
    if radius is not None:
        command.extend(['-r', str(radius)])
    if fov is not None:
        command.extend(['-fov', str(fov)])
    command.extend(['-z', str(downsample)])
    if debug:
        command.append('-debug')
    if update:
        command.append('-update')
    if wcs:
        command.append('-wcs')

    command.extend(["-f", str(image)])

    command_str = ' '.join(command)
    logger.info(f"Command line: {command_str}")

    try:
        astap = subprocess.Popen(command_str, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        stdout, stderr = astap.communicate(timeout=timeout)
    except subprocess.TimeoutExpired:
        result.message = 'Solve timeout'
        logger.error(f"Solve timeout with input {image}, {ra}, {dec}, {fov}")
        return result.__dict__
    except Exception:
        logger.error(traceback.format_exc())
        result.message = 'Unexpected error'
        return result.__dict__

    output_lines = stdout.decode().splitlines()
    solve_success = any("Solution found:" in line for line in output_lines)

    if not solve_success:
        logger.error(f"STDERR: {stderr.decode()}")
        logger.error(f"Solve failed with input {image}, {ra}, {dec}, {fov}, output: {output_lines}")
        return result.__dict__

    logger.info("Solution found!")

    try:
        with fits.open(image) as fit_file:
            header = fit_file[0].header

            result.message = 'Solve success'
            result.ra = header.get('CRVAL1')
            result.dec = header.get('CRVAL2')
            rotation = header.get('CROTA1')
            result.rotation = rotation

            x_pixel_arcsec = Angle(header['CDELT1'] * u.degree).arcsec
            y_pixel_arcsec = Angle(header['CDELT2'] * u.degree).arcsec
            x_pixel_size = header.get('XPIXSZ')
            y_pixel_size = header.get('YPIXSZ')

            if x_pixel_size and y_pixel_size:
                x_focal_length = x_pixel_size / x_pixel_arcsec * 206.625
                y_focal_length = y_pixel_size / y_pixel_arcsec * 206.625
                result.focal_length = (x_focal_length + y_focal_length) / 2  # in mm
    except Exception:
        logger.error(f"Error retrieving solved result for file {image}!")
        logger.error(traceback.format_exc())
        result.message = 'Solve result retrieval failed'

    return result.__dict__
