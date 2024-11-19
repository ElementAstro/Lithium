from pathlib import Path
from typing import Any, Dict, Optional, Tuple
from dataclasses import dataclass
from loguru import logger
from astropy.io import fits
import numpy as np
import cv2
import argparse
import sys
import concurrent.futures


# Configure Loguru logger
logger.remove()  # Remove the default logger
logger.add(sys.stderr, level="INFO", format="{time} {level} {message}")


@dataclass
class CalibrationParams:
    """Data class to store calibration parameters."""
    wavelength: float          # Wavelength in nanometers
    aperture: float            # Aperture diameter in millimeters
    obstruction: float         # Obstruction diameter in millimeters
    filter_width: float        # Filter bandwidth in nanometers
    transmissivity: float      # Transmissivity
    gain: float                # Gain
    quantum_efficiency: float  # Quantum efficiency
    extinction: float          # Extinction coefficient
    exposure_time: float       # Exposure time in seconds


def compute_flx2dn(params: CalibrationParams) -> float:
    """
    Compute the flux-to-DN conversion factor (FLX2DN).

    :param params: Calibration parameters.
    :return: Flux-to-DN conversion factor.
    """
    logger.debug("Starting FLX2DN computation.")
    try:
        c = 3.0e8  # Speed of light in m/s
        h = 6.626e-34  # Planck's constant in J·s
        wavelength_m = params.wavelength * 1e-9  # Convert nanometers to meters

        aperture_area = np.pi * \
            ((params.aperture**2 - params.obstruction**2) / 4)
        FLX2DN = (
            params.exposure_time *
            aperture_area *
            params.filter_width *
            params.transmissivity *
            params.gain *
            params.quantum_efficiency *
            (1 - params.extinction) *
            (wavelength_m / (c * h))
        )
        logger.info(f"Computed FLX2DN: {FLX2DN}")
        return FLX2DN
    except Exception as e:
        logger.error(f"Error computing FLX2DN: {e}")
        raise RuntimeError("Failed to compute FLX2DN.")


def flux_calibration(
    image: np.ndarray,
    params: CalibrationParams,
    response_function: Optional[np.ndarray] = None
) -> Tuple[np.ndarray, float, float, float]:
    """
    Perform flux calibration on an astronomical image.

    :param image: Input image as a numpy array.
    :param params: Calibration parameters.
    :param response_function: Optional instrument response function as a numpy array.
    :return: Tuple containing the calibrated and rescaled image, FLXMIN, FLXRANGE, FLX2DN.
    """
    logger.debug("Starting flux calibration process.")
    try:
        if response_function is not None:
            logger.debug("Applying instrument response correction.")
            image = instrument_response_correction(image, response_function)

        FLX2DN = compute_flx2dn(params)
        calibrated_image = image / FLX2DN
        logger.debug("Applied FLX2DN to the image.")

        calibrated_image = background_noise_correction(calibrated_image)
        logger.debug("Applied background noise correction.")

        # Rescale image to range [0, 1]
        FLXMIN = np.min(calibrated_image)
        FLXRANGE = np.max(calibrated_image) - FLXMIN
        rescaled_image = (calibrated_image - FLXMIN) / FLXRANGE
        logger.info("Rescaled calibrated image to [0, 1] range.")

        return rescaled_image, FLXMIN, FLXRANGE, FLX2DN
    except Exception as e:
        logger.error(f"Flux calibration failed: {e}")
        raise RuntimeError("Flux calibration process failed.")


def save_to_fits(
    image: np.ndarray,
    filename: str,
    FLXMIN: float,
    FLXRANGE: float,
    FLX2DN: float,
    header_info: Optional[Dict[str, Any]] = None
) -> None:
    """
    Save the calibrated image to a FITS file with necessary header information.

    :param image: Calibrated image as a numpy array.
    :param filename: Output FITS filename.
    :param FLXMIN: Minimum flux value used for rescaling.
    :param FLXRANGE: Flux range used for rescaling.
    :param FLX2DN: Flux-to-DN conversion factor.
    :param header_info: Optional additional header information.
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
        logger.info(f"Calibrated image successfully saved to {filename}")
    except Exception as e:
        logger.error(f"Failed to save calibrated image to FITS: {e}")
        raise IOError("Failed to save to FITS file.")


def apply_flat_field_correction(image: np.ndarray, flat_field: np.ndarray) -> np.ndarray:
    """
    Apply flat-field correction to the image.

    :param image: Input image as a numpy array.
    :param flat_field: Flat-field image as a numpy array.
    :return: Corrected image.
    """
    logger.debug("Applying flat-field correction.")
    try:
        if image.shape != flat_field.shape:
            logger.error("Image and flat-field image shapes do not match.")
            raise ValueError(
                "Image and flat-field image must have the same shape.")
        corrected_image = image / flat_field
        logger.info("Successfully applied flat-field correction.")
        return corrected_image
    except Exception as e:
        logger.error(f"Flat-field correction failed: {e}")
        raise RuntimeError("Flat-field correction failed.")


def apply_dark_frame_subtraction(image: np.ndarray, dark_frame: np.ndarray) -> np.ndarray:
    """
    Apply dark frame subtraction to the image.

    :param image: Input image as a numpy array.
    :param dark_frame: Dark frame image as a numpy array.
    :return: Corrected image.
    """
    logger.debug("Applying dark frame subtraction.")
    try:
        if image.shape != dark_frame.shape:
            logger.error("Image and dark frame image shapes do not match.")
            raise ValueError(
                "Image and dark frame image must have the same shape.")
        corrected_image = image - dark_frame
        logger.info("Successfully applied dark frame subtraction.")
        return corrected_image
    except Exception as e:
        logger.error(f"Dark frame subtraction failed: {e}")
        raise RuntimeError("Dark frame subtraction failed.")


def instrument_response_correction(image: np.ndarray, response_function: np.ndarray) -> np.ndarray:
    """
    Apply instrument response correction to the image.

    :param image: Input image as a numpy array.
    :param response_function: Instrument response function as a numpy array.
    :return: Corrected image.
    """
    logger.debug("Applying instrument response correction.")
    try:
        if image.shape != response_function.shape:
            logger.error("Image and response function shapes do not match.")
            raise ValueError(
                "Image and response function must have the same shape.")
        corrected_image = image * response_function
        logger.info("Successfully applied instrument response correction.")
        return corrected_image
    except Exception as e:
        logger.error(f"Instrument response correction failed: {e}")
        raise RuntimeError("Instrument response correction failed.")


def background_noise_correction(image: np.ndarray) -> np.ndarray:
    """
    Apply background noise correction to the image.

    :param image: Input image as a numpy array.
    :return: Corrected image.
    """
    logger.debug("Applying background noise correction.")
    try:
        median = np.median(image)
        corrected_image = image - median
        logger.info("Successfully applied background noise correction.")
        return corrected_image
    except Exception as e:
        logger.error(f"Background noise correction failed: {e}")
        raise RuntimeError("Background noise correction failed.")


def parse_args() -> argparse.Namespace:
    """
    Parse command-line arguments.

    :return: Parsed arguments namespace.
    """
    parser = argparse.ArgumentParser(
        description="Perform flux calibration on astronomical images."
    )
    subparsers = parser.add_subparsers(
        dest='command', required=True, help='Sub-command help'
    )

    # Subcommand: calibrate
    parser_calibrate = subparsers.add_parser(
        'calibrate', help='Calibrate a single image.'
    )
    parser_calibrate.add_argument(
        '--input', type=Path, required=True, help='Path to the input image.'
    )
    parser_calibrate.add_argument(
        '--output', type=Path, required=True, help='Path to save the calibrated FITS file.'
    )
    parser_calibrate.add_argument(
        '--params', type=Path, required=True, help='Path to calibration parameters file.'
    )
    parser_calibrate.add_argument(
        '--response', type=Path, help='Path to the instrument response function image.'
    )
    parser_calibrate.add_argument(
        '--save_bg_noise', action='store_true', help='Save background noise information.'
    )

    # Subcommand: batch_calibrate
    parser_batch = subparsers.add_parser(
        'batch_calibrate', help='Batch calibrate multiple images in a directory.'
    )
    parser_batch.add_argument(
        '--input_dir', type=Path, required=True, help='Path to the input images directory.'
    )
    parser_batch.add_argument(
        '--output_dir', type=Path, required=True, help='Path to save the calibrated FITS files.'
    )
    parser_batch.add_argument(
        '--params_dir', type=Path, required=True, help='Path to the calibration parameters directory.'
    )
    parser_batch.add_argument(
        '--response_dir', type=Path, help='Path to the instrument response functions directory.'
    )
    parser_batch.add_argument(
        '--save_bg_noise', action='store_true', help='Save background noise information.'
    )

    return parser.parse_args()


def load_calibration_params(params_path: Path) -> CalibrationParams:
    """
    Load calibration parameters from a file.

    :param params_path: Path to the calibration parameters file.
    :return: CalibrationParams object.
    """
    logger.debug(f"Loading calibration parameters from {params_path}")
    try:
        # Assuming the parameter file is a simple text file with key=value per line
        params_dict = {}
        with params_path.open('r') as f:
            for line in f:
                if line.strip() and not line.startswith('#'):
                    key, value = line.strip().split('=')
                    params_dict[key.strip()] = float(value.strip())

        params = CalibrationParams(
            wavelength=params_dict['wavelength'],
            aperture=params_dict['aperture'],
            obstruction=params_dict['obstruction'],
            filter_width=params_dict['filter_width'],
            transmissivity=params_dict['transmissivity'],
            gain=params_dict['gain'],
            quantum_efficiency=params_dict['quantum_efficiency'],
            extinction=params_dict['extinction'],
            exposure_time=params_dict['exposure_time']
        )
        logger.info(f"Successfully loaded calibration parameters: {params}")
        return params
    except Exception as e:
        logger.error(f"Failed to load calibration parameters: {e}")
        raise IOError("Failed to load calibration parameters.")


def calibrate_image(
    image_path: Path,
    output_path: Path,
    params: CalibrationParams,
    response_path: Optional[Path] = None,
    save_bg_noise: bool = False
) -> None:
    """
    Calibrate a single image and save the result.

    :param image_path: Path to the input image.
    :param output_path: Path to save the calibrated FITS file.
    :param params: CalibrationParams object.
    :param response_path: Optional path to the instrument response function image.
    :param save_bg_noise: Whether to save background noise information.
    """
    logger.info(f"Starting calibration for image: {image_path}")
    try:
        image = cv2.imread(str(image_path), cv2.IMREAD_GRAYSCALE)
        if image is None:
            logger.error(f"Failed to read image: {image_path}")
            raise IOError(f"Failed to read image: {image_path}")

        response_function = None
        if response_path:
            response_function = cv2.imread(
                str(response_path), cv2.IMREAD_GRAYSCALE)
            if response_function is None:
                logger.error(
                    f"Failed to read instrument response function image: {response_path}")
                raise IOError(
                    f"Failed to read instrument response function image: {response_path}")

        calibrated_image, FLXMIN, FLXRANGE, FLX2DN = flux_calibration(
            image, params, response_function
        )

        save_to_fits(
            calibrated_image,
            str(output_path),
            FLXMIN,
            FLXRANGE,
            FLX2DN
        )

        if save_bg_noise:
            logger.debug(
                f"Saving background noise information: FLXMIN={FLXMIN}, FLXRANGE={FLXRANGE}")
            # Example: Save background noise information to a separate text file
            bg_noise_path = output_path.with_suffix('.bg_noise.txt')
            with bg_noise_path.open('w') as f:
                f.write(f"FLXMIN={FLXMIN}\nFLXRANGE={FLXRANGE}\n")
            logger.info(
                f"Background noise information saved to {bg_noise_path}")

    except Exception as e:
        logger.error(f"Calibration failed for image {image_path}: {e}")
        raise RuntimeError(f"Calibration failed for image {image_path}.")


def batch_calibrate(
    input_dir: Path,
    output_dir: Path,
    params_dir: Path,
    response_dir: Optional[Path] = None,
    save_bg_noise: bool = False
) -> None:
    """
    Batch calibrate all images in a directory.

    :param input_dir: Path to the input images directory.
    :param output_dir: Path to save the calibrated FITS files.
    :param params_dir: Path to the calibration parameters directory.
    :param response_dir: Optional path to the instrument response functions directory.
    :param save_bg_noise: Whether to save background noise information.
    """
    logger.info(f"Starting batch calibration from {input_dir} to {output_dir}")
    try:
        if not input_dir.exists() or not input_dir.is_dir():
            logger.error(
                f"Input directory does not exist or is not a directory: {input_dir}")
            raise NotADirectoryError(
                f"Input directory does not exist or is not a directory: {input_dir}")

        output_dir.mkdir(parents=True, exist_ok=True)

        image_files = list(input_dir.glob('*.*'))
        if not image_files:
            logger.warning(
                f"No image files found in input directory: {input_dir}")
            return

        def process_single_image(image_path: Path):
            basename = image_path.stem
            params_path = params_dir / f"{basename}.txt"
            if not params_path.exists():
                logger.warning(
                    f"Calibration parameters file not found: {params_path}")
                return

            output_path = output_dir / f"{basename}.fits"
            response_path = None
            if response_dir:
                response_path = response_dir / f"{basename}_response.fits"
                if not response_path.exists():
                    logger.warning(
                        f"Instrument response function file not found: {response_path}")
                    response_path = None

            try:
                params = load_calibration_params(params_path)
                calibrate_image(
                    image_path=image_path,
                    output_path=output_path,
                    params=params,
                    response_path=response_path,
                    save_bg_noise=save_bg_noise
                )
            except Exception as e:
                logger.error(f"Error processing image {image_path}: {e}")

        with concurrent.futures.ThreadPoolExecutor() as executor:
            executor.map(process_single_image, image_files)

        logger.info("Batch calibration completed successfully.")
    except Exception as e:
        logger.error(f"Batch calibration failed: {e}")
        raise RuntimeError("Batch calibration process failed.")


def main():
    """
    Main function to parse command-line arguments and execute corresponding operations.
    """
    args = parse_args()

    if args.command == 'calibrate':
        # Calibrate a single image
        try:
            params = load_calibration_params(args.params)
            calibrate_image(
                image_path=args.input,
                output_path=args.output,
                params=params,
                response_path=args.response,
                save_bg_noise=args.save_bg_noise
            )
        except Exception as e:
            logger.exception(f"Single image calibration failed: {e}")
            sys.exit(1)

    elif args.command == 'batch_calibrate':
        # Batch calibrate multiple images
        try:
            batch_calibrate(
                input_dir=args.input_dir,
                output_dir=args.output_dir,
                params_dir=args.params_dir,
                response_dir=args.response_dir,
                save_bg_noise=args.save_bg_noise
            )
        except Exception as e:
            logger.exception(f"Batch calibration failed: {e}")
            sys.exit(1)
    else:
        logger.error(f"Unknown command: {args.command}")
        sys.exit(1)


if __name__ == "__main__":
    main()
