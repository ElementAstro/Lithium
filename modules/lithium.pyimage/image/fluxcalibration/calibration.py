from .utils import instrument_response_correction, background_noise_correction
from .core import CalibrationParams
from typing import Any, Optional, Tuple, Dict
from loguru import logger
from astropy.io import fits
import numpy as np


def compute_flx2dn(params: CalibrationParams) -> float:
    """
    Compute the flux conversion factor (FLX2DN).

    :param params: Calibration parameters.
    :return: Flux conversion factor.
    """
    logger.debug("Starting computation of FLX2DN.")
    try:
        c = 3.0e8  # Speed of light in m/s
        h = 6.626e-34  # Planck's constant in J·s
        wavelength_m = params.wavelength * 1e-9  # Convert nm to meters

        aperture_area = np.pi * \
            (params.aperture**2 - params.obstruction**2) / 4
        FLX2DN = (params.exposure_time * aperture_area * params.filter_width *
                  params.transmissivity * params.gain * params.quantum_efficiency *
                  (1 - params.extinction) * (wavelength_m / (c * h)))
        logger.info(f"Computed FLX2DN: {FLX2DN}")
        return FLX2DN
    except Exception as e:
        logger.error(f"Error computing FLX2DN: {e}")
        raise RuntimeError("Failed to compute FLX2DN.")


def flux_calibration(image: np.ndarray, params: CalibrationParams,
                     response_function: Optional[np.ndarray] = None) -> Tuple[np.ndarray, float, float, float]:
    """
    Perform flux calibration on an astronomical image.

    :param image: Input image (numpy array).
    :param params: Calibration parameters.
    :param response_function: Optional instrument response function (numpy array).
    :return: Tuple containing calibrated and rescaled image, FLXMIN, FLXRANGE, FLX2DN.
    """
    logger.debug("Starting flux calibration process.")
    try:
        if response_function is not None:
            logger.debug("Applying instrument response correction.")
            image = instrument_response_correction(image, response_function)

        FLX2DN = compute_flx2dn(params)
        calibrated_image = image / FLX2DN
        logger.debug("Applied FLX2DN to image.")

        calibrated_image = background_noise_correction(calibrated_image)
        logger.debug("Applied background noise correction.")

        # Rescale the image to the range [0, 1]
        min_val = np.min(calibrated_image)
        max_val = np.max(calibrated_image)
        FLXRANGE = max_val - min_val
        FLXMIN = min_val
        rescaled_image = (calibrated_image - min_val) / FLXRANGE
        logger.info("Rescaled calibrated image to [0, 1].")

        return rescaled_image, FLXMIN, FLXRANGE, FLX2DN
    except Exception as e:
        logger.error(f"Flux calibration failed: {e}")
        raise RuntimeError("Flux calibration process failed.")


def save_to_fits(image: np.ndarray, filename: str, FLXMIN: float, FLXRANGE: float,
                 FLX2DN: float, header_info: Optional[Dict[str, Any]] = None) -> None:
    """
    Save the calibrated image to a FITS file with necessary header information.

    :param image: Calibrated image (numpy array).
    :param filename: Output FITS file name.
    :param FLXMIN: Minimum flux value used for rescaling.
    :param FLXRANGE: Range of flux values used for rescaling.
    :param FLX2DN: Flux conversion factor.
    :param header_info: Dictionary of additional header information to include.
    """
    logger.debug(f"Saving calibrated image to FITS file: {filename}")
    try:
        hdu = fits.PrimaryHDU(image)
        hdr = hdu.header
        hdr['FLXMIN'] = (FLXMIN, 'Minimum flux value for rescaling')
        hdr['FLXRANGE'] = (FLXRANGE, 'Flux range for rescaling')
        hdr['FLX2DN'] = (FLX2DN, 'Flux conversion factor')

        # Add additional header information if provided
        if header_info:
            for key, value in header_info.items():
                hdr[key] = (value, f'Additional header key {key}')

        hdu.writeto(filename, overwrite=True)
        logger.info(f"Calibrated image saved successfully to {filename}")
    except Exception as e:
        logger.error(f"Failed to save calibrated image to FITS: {e}")
        raise IOError("Saving to FITS file failed.")


def apply_flat_field_correction(image: np.ndarray, flat_field: np.ndarray) -> np.ndarray:
    """
    Apply flat field correction to an image.

    :param image: Input image (numpy array).
    :param flat_field: Flat field image (numpy array).
    :return: Corrected image.
    """
    logger.debug("Applying flat field correction.")
    try:
        if image.shape != flat_field.shape:
            logger.error("Image and flat field shapes do not match.")
            raise ValueError("Image and flat field must have the same shape.")
        corrected_image = image / flat_field
        logger.info("Flat field correction applied successfully.")
        return corrected_image
    except Exception as e:
        logger.error(f"Flat field correction failed: {e}")
        raise RuntimeError("Flat field correction failed.")


def apply_dark_frame_subtraction(image: np.ndarray, dark_frame: np.ndarray) -> np.ndarray:
    """
    Apply dark frame subtraction to an image.

    :param image: Input image (numpy array).
    :param dark_frame: Dark frame image (numpy array).
    :return: Corrected image.
    """
    logger.debug("Applying dark frame subtraction.")
    try:
        if image.shape != dark_frame.shape:
            logger.error("Image and dark frame shapes do not match.")
            raise ValueError("Image and dark frame must have the same shape.")
        corrected_image = image - dark_frame
        logger.info("Dark frame subtraction applied successfully.")
        return corrected_image
    except Exception as e:
        logger.error(f"Dark frame subtraction failed: {e}")
        raise RuntimeError("Dark frame subtraction failed.")
