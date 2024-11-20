import numpy as np
from loguru import logger
import argparse
from typing import Tuple


def setup_logging():
    """Set up the loguru logging configuration to log both to console and to a file."""
    logger.add("fov_calculation.log", level="DEBUG",
               format="{time} {level} {message}", rotation="10 MB")
    logger.info("Logging setup complete.")


def ra_dec2fixed_xyz(ra: float, dec: float) -> np.ndarray:
    """
    Convert RA/DEC to fixed XYZ coordinates in the celestial coordinate system.

    Args:
        ra: Right ascension in degrees.
        dec: Declination in degrees.

    Returns:
        A numpy array representing the [x, y, z] coordinates.
    """
    ra_rad = np.radians(ra)
    dec_rad = np.radians(dec)

    x = np.cos(ra_rad) * np.cos(dec_rad)
    y = np.sin(ra_rad) * np.cos(dec_rad)
    z = np.sin(dec_rad)

    logger.debug(f"RA: {ra}°, Dec: {dec}° -> X: {x}, Y: {y}, Z: {z}")

    return np.array([x, y, z])


def fixed_xyz2ra_dec(vector: np.ndarray) -> Tuple[float, float]:
    """
    Convert fixed XYZ coordinates to RA/DEC.

    Args:
        vector: A numpy array with the [x, y, z] coordinates.

    Returns:
        A tuple containing RA and DEC in degrees.
    """
    x, y, z = vector

    ra = np.arctan2(y, x)
    ra = np.degrees(ra)

    r = np.sqrt(x**2 + y**2)
    dec = np.arctan2(z, r)
    dec = np.degrees(dec)

    logger.debug(f"X: {x}, Y: {y}, Z: {z} -> RA: {ra}°, Dec: {dec}°")

    return ra, dec


def calc_fov_points(x_pixels: int, x_pixel_size: float, y_pixels: int, y_pixel_size: float, focal_length: int,
                    target_ra: float, target_dec: float, camera_rotation: float) -> Tuple[Tuple[float, float],
                                                                                          Tuple[float,
                                                                                                float],
                                                                                          Tuple[float,
                                                                                                float],
                                                                                          Tuple[float, float]]:
    """
    Calculate the field of view (FOV) points in RA/DEC coordinates for a camera given its parameters.

    Args:
        x_pixels: Number of pixels in the x direction.
        x_pixel_size: Size of each pixel in the x direction in micrometers.
        y_pixels: Number of pixels in the y direction.
        y_pixel_size: Size of each pixel in the y direction in micrometers.
        focal_length: Focal length of the camera in millimeters.
        target_ra: Target right ascension in degrees.
        target_dec: Target declination in degrees.
        camera_rotation: Rotation of the camera in degrees.

    Returns:
        A tuple containing four tuples, each representing the RA/DEC coordinates of a corner of the FOV.
    """
    if focal_length <= 0:
        raise ValueError("Focal length must be a positive value.")

    logger.info(f"Calculating FOV points for target RA: {target_ra}°, Dec: {target_dec}°, "
                f"Camera rotation: {camera_rotation}°, Focal length: {focal_length}mm")

    # Convert pixel sizes to millimeters
    x_length = x_pixels * x_pixel_size / 1000
    y_length = y_pixels * y_pixel_size / 1000
    logger.debug(f"Sensor size: {x_length}mm x {y_length}mm")

    # Calculate FOV angles
    FOV_x = np.degrees(np.arctan(x_length / (2 * focal_length)) * 2)
    FOV_y = np.degrees(np.arctan(y_length / (2 * focal_length)) * 2)
    logger.debug(f"FOV angles: FOV_x = {FOV_x}°, FOV_y = {FOV_y}°")

    FOV_x = np.radians(FOV_x)
    FOV_y = np.radians(FOV_y)

    # Define corner points in the camera's field of view
    A = np.array([1, np.tan(FOV_x / 2), np.tan(FOV_y / 2)]).reshape([3, 1])
    B = np.array([1, -np.tan(FOV_x / 2), np.tan(FOV_y / 2)]).reshape([3, 1])
    C = np.array([1, -np.tan(FOV_x / 2), -np.tan(FOV_y / 2)]).reshape([3, 1])
    D = np.array([1, np.tan(FOV_x / 2), -np.tan(FOV_y / 2)]).reshape([3, 1])

    # Convert target RA/DEC and rotation to radians
    RA = np.radians(target_ra)
    DEC = np.radians(target_dec)
    THETA = np.radians(camera_rotation)

    # Rotation matrices
    Rx = np.array([[1, 0, 0],
                   [0, np.cos(THETA), -np.sin(THETA)],
                   [0, np.sin(THETA), np.cos(THETA)]])

    Ry = np.array([[np.cos(DEC), 0, np.sin(DEC)],
                   [0, 1, 0],
                   [-np.sin(DEC), 0, np.cos(DEC)]])

    Rz = np.array([[np.cos(RA), -np.sin(RA), 0],
                   [np.sin(RA), np.cos(RA), 0],
                   [0, 0, 1]])

    # Combined rotation matrix
    R = Rz @ Ry @ Rx
    logger.debug(f"Combined rotation matrix: \n{R}")

    # Apply the rotation to each corner point
    R_A = R @ A
    R_B = R @ B
    R_C = R @ C
    R_D = R @ D

    corners = (fixed_xyz2ra_dec(R_A.flatten()),
               fixed_xyz2ra_dec(R_B.flatten()),
               fixed_xyz2ra_dec(R_C.flatten()),
               fixed_xyz2ra_dec(R_D.flatten()))

    logger.info(f"Calculated FOV corners: {corners}")

    return corners


def main():
    parser = argparse.ArgumentParser(
        description="Calculate the field of view (FOV) points in RA/DEC coordinates for a camera.")
    parser.add_argument('--x_pixels', type=int, required=True,
                        help='Number of pixels in the x direction')
    parser.add_argument('--x_pixel_size', type=float, required=True,
                        help='Size of each pixel in the x direction in micrometers')
    parser.add_argument('--y_pixels', type=int, required=True,
                        help='Number of pixels in the y direction')
    parser.add_argument('--y_pixel_size', type=float, required=True,
                        help='Size of each pixel in the y direction in micrometers')
    parser.add_argument('--focal_length', type=int, required=True,
                        help='Focal length of the camera in millimeters')
    parser.add_argument('--target_ra', type=float, required=True,
                        help='Target right ascension in degrees')
    parser.add_argument('--target_dec', type=float,
                        required=True, help='Target declination in degrees')
    parser.add_argument('--camera_rotation', type=float,
                        default=0.0, help='Rotation of the camera in degrees')

    args = parser.parse_args()

    # Set up logging
    setup_logging()

    try:
        fov_points = calc_fov_points(x_pixels=args.x_pixels, x_pixel_size=args.x_pixel_size, y_pixels=args.y_pixels,
                                     y_pixel_size=args.y_pixel_size, focal_length=args.focal_length,
                                     target_ra=args.target_ra, target_dec=args.target_dec, camera_rotation=args.camera_rotation)
        for idx, point in enumerate(fov_points, start=1):
            print(f"Corner {idx}: RA = {point[0]:.6f}°, DEC = {point[1]:.6f}°")
    except Exception as e:
        logger.error(f"Error calculating FOV points: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
