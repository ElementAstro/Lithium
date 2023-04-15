
import asyncio
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
import traceback


async def solve(image: str, ra=None, dec=None, radius=None, downsample=None, depth=None, scale_low=None, scale_high=None, width=None, height=None,
                scale_units=None, overwrite=True, no_plot=True, verify=False, debug=False, timeout=30, resort=False, _continue=False, no_tweak=False) -> dict:
    """
        Solve the given image by astrometry and return the ra and dec if the solve is successful
        Args:
            image : str # the full path to the image
            ra : str # the ra hour angle
            dec : str # the dec hour angle
            radius : float # the radius of the search region
            downsample : int # the downsample , default is 1
            depth : list # a list of depth , like [20,30,40]
            scale_low : float # lowest of the scale
            scale_high : float # highest of the scale
            width : int # the width of the image
            height : int # the height of the image
            scale_units : str # the scale unit , default is "degwidth"
            overwrite : bool # whether to overwrite the existing image
            no_plot : bool # whether to plot the solve results
            verify : bool # whether to verify the existing wcs file
            debug : bool # whether to print debug information
            timeout : int # the number of seconds to wait before giving up
            resort : bool # whether to resort the brightness of the image
            _continue : bool # whether to continue the old operation
            no_tweak : bool # whether to correct the old wcs file

        Returns:{
            "ra" : None,
            "dec" : None,
            "fov_x" : None,
            "fov_y" : None,
            "rotation" : None,
            "message" : None
        }
        if faced error, then message will not be none and display basic error message.
        if correctly executed, message will be none
    """
    ret_struct = {
        "ra": None,
        "dec": None,
        "fov_x": None,
        "fov_y": None,
        "rotation": None,
        "message": None
    }

    if image is None or not isinstance(image, (str, Path)):  # (, np.ndarray)
        ret_struct['message'] = 'wrong image file type'
        return ret_struct
    if type(image) == str:
        image = Path(image)
    if not image.exists():
        ret_struct['message'] = 'file does not exist'
        return ret_struct

    command = ["solve-field", str(image)]

    if ra is not None and isinstance(ra, str):
        command.extend(["--ra", ra])
    if dec is not None and isinstance(dec, str):
        command.extend(["--dec", dec])
    if radius is not None and isinstance(radius, float):
        command.extend(["--radius", str(radius)])
    if downsample is not None and isinstance(downsample, int):
        command.extend(["--downsample", str(downsample)])
    if depth is not None and isinstance(depth, list):
        command.extend(
            ["--depth", str(depth).replace("[", "").replace("]", "").replace(" ", "")])
    if scale_low is not None and isinstance(scale_low, float):
        command.extend(["--scale-low", str(scale_low)])
    if scale_high is not None and isinstance(scale_high, float):
        command.extend(["--scale-high", str(scale_high)])
    if width is not None and isinstance(width, int):
        command.extend(["--width", str(width)])
    if height is not None and isinstance(height, int):
        command.extend(["--height", str(height)])
    if scale_units is not None and isinstance(scale_units, str):
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

    # change command as it is called by asyncio subprocess
    command = ' '.join(command)

    try:
        astap = await asyncio.subprocess.create_subprocess_shell(command, stdin=asyncio.subprocess.PIPE,
                                                                 stdout=asyncio.subprocess.PIPE)
        std_out, std_error = await asyncio.wait_for(astap.communicate(), timeout=timeout)
        output_ = std_out.decode().split("\n")

        for item in output_:
            if item.find("Field center: (RA H:M:S, Dec D:M:S) = ") != -1:
                ra_dec = item.replace("Field center: (RA H:M:S, Dec D:M:S) = ", "").replace(
                    "(", "").replace(").", "")
                ret_struct["ra"], ret_struct["dec"] = ra_dec.split(",")

            if item.find("Field size: ") != -1:
                fov_ = item.replace("Field size: ", "").replace(" ", "")
                ret_struct["fov_x"], ret_struct["fov_y"] = fov_.split("x")

            if item.find("Field rotation angle: up is ") != -1:
                ret_struct["rotation"] = item.replace(
                    "Field rotation angle: up is ", "").replace(" degrees E of N", "")

        if ret_struct['ra'] is None or ret_struct['dec'] is None:
            ret_struct['message'] = 'Solve failed'
            return ret_struct

        return ret_struct
    except TimeoutError:
        ret_struct['message'] = 'Solve timeout'
        print(f'Solve Timeout with input {image}, {ra}, {dec}, {radius}')
    except:
        traceback.print_exc()
        ret_struct['message'] = 'unpredictable error'

    return ret_struct

asyncio.run(solve("test.jpg"))
