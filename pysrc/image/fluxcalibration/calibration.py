import numpy as np
from astropy.io import fits
from .core import CalibrationParams
from .utils import instrument_response_correction, background_noise_correction

def compute_flx2dn(params: CalibrationParams) -> float:
    """
    Compute the flux conversion factor (FLX2DN).
    :param params: Calibration parameters.
    :return: Flux conversion factor.
    """
    c = 3.0e8  # Speed of light in m/s
    h = 6.626e-34  # Planck's constant in J·s
    wavelength_m = params.wavelength * 1e-9  # Convert nm to meters

    aperture_area = np.pi * (params.aperture**2 - params.obstruction**2) / 4
    FLX2DN = (params.exposure_time * aperture_area * params.filter_width * 
              params.transmissivity * params.gain * params.quantum_efficiency * 
              (1 - params.extinction) * (wavelength_m / (c * h)))
    return FLX2DN

def flux_calibration(image: np.ndarray, params: CalibrationParams, 
                     response_function: Optional[np.ndarray] = None) -> np.ndarray:
    """
    Perform flux calibration on an astronomical image.
    :param image: Input image (numpy array).
    :param params: Calibration parameters.
    :param response_function: Optional instrument response function (numpy array).
    :return: Flux-calibrated and rescaled image.
    """
    if response_function is not None:
        image = instrument_response_correction(image, response_function)

    FLX2DN = compute_flx2dn(params)
    calibrated_image = image / FLX2DN
    calibrated_image = background_noise_correction(calibrated_image)

    # Rescale the image to the range [0, 1]
    min_val = np.min(calibrated_image)
    max_val = np.max(calibrated_image)
    FLXRANGE = max_val - min_val
    FLXMIN = min_val
    rescaled_image = (calibrated_image - min_val) / FLXRANGE

    return rescaled_image, FLXMIN, FLXRANGE, FLX2DN

def save_to_fits(image: np.ndarray, filename: str, FLXMIN: float, FLXRANGE: float, 
                 FLX2DN: float, header_info: dict = {}) -> None:
    """
    Save the calibrated image to a FITS file with necessary header information.
    :param image: Calibrated image (numpy array).
    :param filename: Output FITS file name.
    :param FLXMIN: Minimum flux value used for rescaling.
    :param FLXRANGE: Range of flux values used for rescaling.
    :param FLX2DN: Flux conversion factor.
    :param header_info: Dictionary of additional header information to include.
    """
    hdu = fits.PrimaryHDU(image)
    hdr = hdu.header
    hdr['FLXMIN'] = FLXMIN
    hdr['FLXRANGE'] = FLXRANGE
    hdr['FLX2DN'] = FLX2DN

    # Add additional header information
    for key, value in header_info.items():
        hdr[key] = value

    hdu.writeto(filename, overwrite=True)
