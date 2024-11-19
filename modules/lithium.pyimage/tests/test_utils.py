import pytest
import numpy as np
from pathlib import Path
from astropy.io import fits
from ..utils import read_fits_header
from ..core import CalibrationParams

# FILE: modules/lithium.pyimage/image/fluxcalibration/test_utils.py


@pytest.fixture
def setup_fits_file(tmp_path):
    # Create a temporary directory with a test FITS file
    fits_path = tmp_path / "test_image.fits"
    data = np.zeros((100, 100), dtype=np.float32)
    hdu = fits.PrimaryHDU(data)
    hdr = hdu.header
    hdr['WAVELEN'] = 500.0
    hdr['TRANSMIS'] = 0.8
    hdr['FILTWDTH'] = 100.0
    hdr['APERTURE'] = 200.0
    hdr['OBSTRUCT'] = 50.0
    hdr['EXPTIME'] = 60.0
    hdr['EXTINCT'] = 0.1
    hdr['GAIN'] = 1.5
    hdr['QUANTEFF'] = 0.9
    hdu.writeto(fits_path)
    return fits_path


def test_read_fits_header_valid(setup_fits_file):
    params = read_fits_header(str(setup_fits_file))
    assert params.wavelength == 500.0
    assert params.transmissivity == 0.8
    assert params.filter_width == 100.0
    assert params.aperture == 200.0
    assert params.obstruction == 50.0
    assert params.exposure_time == 60.0
    assert params.extinction == 0.1
    assert params.gain == 1.5
    assert params.quantum_efficiency == 0.9


def test_read_fits_header_missing_keys(tmp_path):
    fits_path = tmp_path / "test_image_missing_keys.fits"
    data = np.zeros((100, 100), dtype=np.float32)
    hdu = fits.PrimaryHDU(data)
    hdu.writeto(fits_path)
    params = read_fits_header(str(fits_path))
    assert params.wavelength == 550  # Default value
    assert params.transmissivity == 0.8  # Default value
    assert params.filter_width == 100  # Default value
    assert params.aperture == 200  # Default value
    assert params.obstruction == 50  # Default value
    assert params.exposure_time == 60  # Default value
    assert params.extinction == 0.1  # Default value
    assert params.gain == 1.5  # Default value
    assert params.quantum_efficiency == 0.9  # Default value


def test_read_fits_header_invalid_file(tmp_path):
    invalid_fits_path = tmp_path / "invalid_image.fits"
    with open(invalid_fits_path, 'w') as f:
        f.write("This is not a valid FITS file.")
    with pytest.raises(OSError):
        read_fits_header(str(invalid_fits_path))


def test_read_fits_header_non_existent_file():
    non_existent_path = "non_existent_file.fits"
    with pytest.raises(OSError):
        read_fits_header(non_existent_path)
