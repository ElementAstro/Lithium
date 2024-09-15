import numpy as np
from astropy.stats import sigma_clipped_stats

def instrument_response_correction(image: np.ndarray, response_function: np.ndarray) -> np.ndarray:
    """
    Apply instrument response correction to the image.
    :param image: Input image (numpy array).
    :param response_function: Instrument response function (numpy array of the same shape as the image).
    :return: Corrected image.
    """
    return image / response_function

def background_noise_correction(image: np.ndarray) -> np.ndarray:
    """
    Estimate and subtract the background noise from the image.
    :param image: Input image (numpy array).
    :return: Image with background noise subtracted.
    """
    _, median, _ = sigma_clipped_stats(image, sigma=3.0)  # Estimate background
    return image - median

def read_fits_header(file_path: str) -> dict:
    """
    Reads the FITS header and returns the necessary calibration parameters.
    :param file_path: Path to the FITS file.
    :return: Dictionary containing calibration parameters.
    """
    with fits.open(file_path) as hdul:
        header = hdul[0].header
        params = {
            'wavelength': header.get('WAVELEN', 550),  # nm
            'transmissivity': header.get('TRANSMIS', 0.8),
            'filter_width': header.get('FILTWDTH', 100),  # nm
            'aperture': header.get('APERTURE', 200),  # mm
            'obstruction': header.get('OBSTRUCT', 50),  # mm
            'exposure_time': header.get('EXPTIME', 60),  # seconds
            'extinction': header.get('EXTINCT', 0.1),
            'gain': header.get('GAIN', 1.5),  # e-/ADU
            'quantum_efficiency': header.get('QUANTEFF', 0.9),
        }
        return params
