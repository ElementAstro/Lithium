import asyncio
import traceback

import numpy as np
from pathlib import Path
#from ..misc import #support_logger
from astropy.io import fits
from astropy import units as u
from astropy.coordinates import Angle


async def astap_solve(image: str, ra: float = None, dec: float = None, radius: float = None, fov: float = None,
                      downsample: int = None, debug: bool = False, update: bool = True, _wcs: bool = False,
                      timeout: int = 60) -> dict:
    ret_struct = {
        'ra': None,
        'dec': None,
        'rotation': None,
        'focal_length': None,
        'message': None,
    }
    if not isinstance(image, (str, Path)):
        ret_struct['message'] = 'wrong image file type'
        return ret_struct
    image = Path(image)
    if not image.exists():
        ret_struct['message'] = 'file does not exist'
        return ret_struct

    command = ["astap"]

    for arg_name, arg_value in {
        "ra": ra,
        "dec": dec + 90 if dec is not None else None,
        "radius": radius,
        "fov": fov,
        "downsample": downsample,
    }.items():
        if arg_value is not None:
            command.extend(["-" + arg_name, str(arg_value)])

    if debug:
        command.append("-debug")
    if update:
        command.append("-update")
    if _wcs:
        command.append("-wcs")

    command.extend(["-f", str(image)])

    command_str = " ".join(command)
    #support_logger.info(f"Command line: {command_str}")

    try:
        astap = await asyncio.subprocess.create_subprocess_shell(
            command_str,
            stdin=asyncio.subprocess.PIPE,
            stdout=asyncio.subprocess.PIPE
        )
        std_out, std_err = await asyncio.wait_for(astap.communicate(), timeout=timeout)
    except TimeoutError:
        ret_struct['message'] = 'Solve timeout'
        #support_logger.error(f"Solve Timeout with input {image}, {ra}, {dec}, {fov}")
        return ret_struct
    except:
        #support_logger.error(traceback.format_exc())
        ret_struct['message'] = 'unpredictable error'
        return ret_struct

    output_lines = std_out.decode().split("\n")
    if any(["Solution found:" in line for line in output_lines]):
        ret_struct['message'] = 'Solve success'

        try:
            fits_header = fits.getheader(image)

            ret_struct['ra'] = fits_header['CRVAL1']
            ret_struct['dec'] = fits_header['CRVAL2']
            ret_struct['rotation'] = fits_header.get('CROTA1')
            x_pixel_size_um = fits_header.get('XPIXSZ')
            y_pixel_size_um = fits_header.get('YPIXSZ')

            x_pixel_degree = Angle(fits_header['CDELT1'], u.deg)
            y_pixel_degree = Angle(fits_header['CDELT2'], u.deg)

            # Calculate focal length and fov.
            if x_pixel_size_um and y_pixel_size_um:
                x_focal_length_mm = x_pixel_size_um / x_pixel_degree.arcsec * 206.625
                y_focal_length_mm = y_pixel_size_um / y_pixel_degree.arcsec * 206.625
                avg_focal_length_mm = (x_focal_length_mm + y_focal_length_mm) / 2
                ret_struct['focal_length'] = avg_focal_length_mm
                ret_struct['fov'] = Angle(206.265 / avg_focal_length_mm, u.deg).deg
        except:
            #support_logger.error(f'Error in getting solved result for file {image}!')
            #support_logger.error(traceback.format_exc())
            ret_struct['message'] = 'Solve Result get failed'
    else:
        ret_struct['message'] = 'Solve failed'

    return ret_struct
