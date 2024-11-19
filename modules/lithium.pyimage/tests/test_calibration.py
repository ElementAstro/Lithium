import pytest
import numpy as np
from pathlib import Path
from ..calibration import batch_calibrate, CalibrationParams, save_to_fits

# FILE: modules/lithium.pyimage/image/fluxcalibration/test_calibration.py


@pytest.fixture
def setup_test_environment(tmp_path):
    # Create a temporary directory with test images and parameter files
    input_dir = tmp_path / "input_images"
    input_dir.mkdir()
    output_dir = tmp_path / "output_images"
    params_dir = tmp_path / "params"
    params_dir.mkdir()
    response_dir = tmp_path / "response"
    response_dir.mkdir()

    # Create dummy images
    for i in range(3):
        image = np.ones((100, 100), dtype=np.uint8) * (i * 85)
        image_path = input_dir / f"test_image_{i}.png"
        cv2.imwrite(str(image_path), image)

        # Create corresponding parameter files
        params_content = f"""
        wavelength=500.0
        aperture=100.0
        obstruction=20.0
        filter_width=10.0
        transmissivity=0.9
        gain=1.0
        quantum_efficiency=0.8
        extinction=0.1
        exposure_time=30.0
        """
        params_path = params_dir / f"test_image_{i}.txt"
        with params_path.open('w') as f:
            f.write(params_content.strip())

        # Create corresponding response function files
        response_function = np.ones((100, 100), dtype=np.uint8) * 255
        response_path = response_dir / f"test_image_{i}_response.fits"
        save_to_fits(response_function, str(response_path), 0, 1, 1)

    return input_dir, output_dir, params_dir, response_dir


def test_batch_calibrate_valid_images(setup_test_environment):
    input_dir, output_dir, params_dir, response_dir = setup_test_environment
    batch_calibrate(input_dir, output_dir, params_dir, response_dir)
    assert len(list(output_dir.glob("*.fits"))) == 3


def test_batch_calibrate_missing_params(setup_test_environment):
    input_dir, output_dir, params_dir, response_dir = setup_test_environment
    missing_params_path = params_dir / "test_image_1.txt"
    missing_params_path.unlink()
    batch_calibrate(input_dir, output_dir, params_dir, response_dir)
    assert len(list(output_dir.glob("*.fits"))) == 2


def test_batch_calibrate_missing_response(setup_test_environment):
    input_dir, output_dir, params_dir, response_dir = setup_test_environment
    missing_response_path = response_dir / "test_image_1_response.fits"
    missing_response_path.unlink()
    batch_calibrate(input_dir, output_dir, params_dir, response_dir)
    assert len(list(output_dir.glob("*.fits"))) == 3


def test_batch_calibrate_invalid_input_directory(tmp_path):
    invalid_input_dir = tmp_path / "invalid_input"
    output_dir = tmp_path / "output_images"
    params_dir = tmp_path / "params"
    response_dir = tmp_path / "response"
    with pytest.raises(NotADirectoryError):
        batch_calibrate(invalid_input_dir, output_dir,
                        params_dir, response_dir)


def test_batch_calibrate_invalid_output_directory(setup_test_environment):
    input_dir, _, params_dir, response_dir = setup_test_environment
    invalid_output_dir = Path("/invalid/output")
    with pytest.raises(OSError):
        batch_calibrate(input_dir, invalid_output_dir,
                        params_dir, response_dir)
