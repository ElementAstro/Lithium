import numpy as np
from astropy.stats import sigma_clipped_stats
from loguru import logger
from astropy.io import fits
from .core import CalibrationParams


def instrument_response_correction(image: np.ndarray, response_function: np.ndarray) -> np.ndarray:
    """
    Apply instrument response correction to the image.

    :param image: Input image (numpy array).
    :param response_function: Instrument response function (numpy array of the same shape as the image).
    :return: Corrected image.
    """
    logger.debug("Applying instrument response correction.")
    try:
        if image.shape != response_function.shape:
            logger.error("Image and response function shapes do not match.")
            raise ValueError(
                "Image and response function must have the same shape.")
        corrected_image = image / response_function
        logger.info("Instrument response correction applied successfully.")
        return corrected_image
    except Exception as e:
        logger.error(f"Instrument response correction failed: {e}")
        raise RuntimeError("Instrument response correction failed.")


def background_noise_correction(image: np.ndarray, sigma: float = 3.0) -> np.ndarray:
    """
    Estimate and subtract the background noise from the image.

    :param image: Input image (numpy array).
    :param sigma: Sigma level for sigma-clipping. Default is 3.0.
    :return: Image with background noise subtracted.
    """
    logger.debug(f"Applying background noise correction with sigma={sigma}.")
    try:
        mean, median, std = sigma_clipped_stats(image, sigma=sigma)
        corrected_image = image - median
        logger.info(
            f"Background noise correction applied successfully. Median: {median}, Std: {std}")
        return corrected_image
    except Exception as e:
        logger.error(f"Background noise correction failed: {e}")
        raise RuntimeError("Background noise correction failed.")


def read_fits_header(file_path: str) -> CalibrationParams:
    """
    Reads the FITS header and returns the necessary calibration parameters.

    :param file_path: Path to the FITS file.
    :return: CalibrationParams object containing calibration parameters.
    """
    logger.debug(f"Reading FITS header from file: {file_path}")
    try:
        with fits.open(file_path) as hdul:
            header = hdul[0].header
            params = CalibrationParams(
                wavelength=header.get('WAVELEN', 550),  # nm
                transmissivity=header.get('TRANSMIS', 0.8),
                filter_width=header.get('FILTWDTH', 100),  # nm
                aperture=header.get('APERTURE', 200),  # mm
                obstruction=header.get('OBSTRUCT', 50),  # mm
                exposure_time=header.get('EXPTIME', 60),  # seconds
                extinction=header.get('EXTINCT', 0.1),
                gain=header.get('GAIN', 1.5),  # e-/ADU
                quantum_efficiency=header.get('QUANTEFF', 0.9),
            )
            logger.info(
                "Calibration parameters extracted successfully from FITS header.")
            return params
    except Exception as e:
        logger.error(f"Failed to read FITS header: {e}")
        raise IOError("Reading FITS header failed.")
