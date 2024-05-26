# coding=utf-8

"""
Copyright(c) 2022-2023 Max Qian  <lightapt.com>
"""

import asyncio
from pathlib import Path
import traceback
from typing import Union, Optional, List, Dict
import logging

logger = logging.getLogger(__name__)

class SolverError(Exception):
    """Custom exception for solver errors."""


async def solve(
    image: Union[str, Path],
    ra: Optional[str] = None,
    dec: Optional[str] = None,
    radius: Optional[float] = None,
    downsample: Optional[int] = None,
    depth: Optional[List[int]] = None,
    scale_low: Optional[float] = None,
    scale_high: Optional[float] = None,
    width: Optional[int] = None,
    height: Optional[int] = None,
    scale_units: Optional[str] = "degwidth",
    overwrite: bool = True,
    no_plot: bool = True,
    verify: bool = False,
    debug: bool = False,
    timeout: int = 30,
    resort: bool = False,
    _continue: bool = False,
    no_tweak: bool = False
) -> Dict[str, Optional[Union[str, float]]]:
    """
    Solve the given image using astrometry and return the RA and Dec if the solve is successful.

    Args:
        image (Union[str, Path]): Full path to the image.
        ra (Optional[str]): RA hour angle.
        dec (Optional[str]): Dec hour angle.
        radius (Optional[float]): Radius of the search region.
        downsample (Optional[int]): Downsample factor (default is 1).
        depth (Optional[List[int]]): List of depth levels (e.g., [20, 30, 40]).
        scale_low (Optional[float]): Lowest scale value.
        scale_high (Optional[float]): Highest scale value.
        width (Optional[int]): Image width.
        height (Optional[int]): Image height.
        scale_units (Optional[str]): Scale unit (default is "degwidth").
        overwrite (bool): Whether to overwrite existing files.
        no_plot (bool): Whether to plot the solve results.
        verify (bool): Whether to verify the existing WCS file.
        debug (bool): Whether to print debug information.
        timeout (int): Number of seconds to wait before giving up.
        resort (bool): Whether to resort the brightness of the image.
        _continue (bool): Whether to continue the old operation.
        no_tweak (bool): Whether to correct the old WCS file.

    Returns:
        Dict[str, Optional[Union[str, float]]]: Solution results.
    """
    ret_struct = {
        "ra": None,
        "dec": None,
        "fov_x": None,
        "fov_y": None,
        "rotation": None,
        "message": None
    }

    if not isinstance(image, (str, Path)):
        ret_struct["message"] = "Invalid image file type"
        return ret_struct
    image = Path(image)

    if not image.exists():
        ret_struct["message"] = "File does not exist"
        return ret_struct

    command = ["/usr/astrometry/bin/solve-field", str(image)]

    # Add optional parameters to the command
    if ra is not None:
        command.extend(["--ra", ra])
    if dec is not None:
        command.extend(["--dec", dec])
    if radius is not None:
        command.extend(["--radius", str(radius)])
    if downsample is not None:
        command.extend(["--downsample", str(downsample)])
    if depth is not None:
        command.extend(["--depth", ",".join(map(str, depth))])
    if scale_low is not None:
        command.extend(["--scale-low", str(scale_low)])
    if scale_high is not None:
        command.extend(["--scale-high", str(scale_high)])
    if width is not None:
        command.extend(["--width", str(width)])
    if height is not None:
        command.extend(["--height", str(height)])
    if scale_units is not None:
        command.extend(["--scale-units", scale_units])
    if overwrite:
        command.append("--overwrite")
    if no_plot:
        command.append("--no-plot")
    if verify:
        command.append("--verify")
    if resort:
        command.append("--resort")
    if _continue:
        command.append("--continue")
    if no_tweak:
        command.append("--no-tweak")
    if debug:
        command.append("--verbose")

    command_str = " ".join(command)
    logger.debug(f"Command line: {command_str}")

    try:
        astrometry = await asyncio.subprocess.create_subprocess_shell(
            command_str,
            stdin=asyncio.subprocess.PIPE,
            stdout=asyncio.subprocess.PIPE
        )
        stdout, stderr = await asyncio.wait_for(astrometry.communicate(), timeout=timeout)
    except asyncio.TimeoutError:
        ret_struct["message"] = "Solve timeout"
        logger.error(f"Solve timeout with input {image}, {ra}, {dec}, {radius}")
        return ret_struct
    except Exception:
        logger.error(traceback.format_exc())
        ret_struct["message"] = "Unexpected error"
        return ret_struct

    output_lines = stdout.decode().splitlines()

    for line in output_lines:
        if "Field center: (RA H:M:S, Dec D:M:S) =" in line:
            ra_dec = line.replace("Field center: (RA H:M:S, Dec D:M:S) = ", "").replace("(", "").replace(").", "")
            ret_struct["ra"], ret_struct["dec"] = ra_dec.split(",")
        elif "Field size:" in line:
            fov_x, fov_y = line.replace("Field size:", "").strip().split("x")
            ret_struct["fov_x"], ret_struct["fov_y"] = fov_x, fov_y
        elif "Field rotation angle: up is" in line:
            ret_struct["rotation"] = line.replace("Field rotation angle: up is ", "").replace(" degrees E of N", "")

    if ret_struct["ra"] is None or ret_struct["dec"] is None:
        ret_struct["message"] = "Solve failed"
        return ret_struct

    return ret_struct
