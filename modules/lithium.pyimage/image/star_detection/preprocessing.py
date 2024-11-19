import cv2
import numpy as np
from astropy.io import fits
from typing import List, Tuple, Optional, Union
from pathlib import Path
from dataclasses import dataclass, field
from loguru import logger
import matplotlib.pyplot as plt
import json


@dataclass
class PreprocessingConfig:
    """
    Configuration settings for image preprocessing.
    """
    median_filter_size: int = 3
    wavelet_levels: int = 3
    binarization_threshold: int = 128
    save_preprocessed: bool = False
    preprocessed_save_path: Optional[Path] = None
    save_wavelet: bool = False
    wavelet_save_path: Optional[Path] = None


class Preprocessor:
    def __init__(self, config: Optional[PreprocessingConfig] = None) -> None:
        """
        Initialize the Preprocessor with optional configuration.

        :param config: PreprocessingConfig object containing settings.
        """
        self.config = config or PreprocessingConfig()

        # Configure Loguru logger
        logger.remove()  # Remove default logger
        logger.add("preprocessing.log", rotation="10 MB", retention="10 days",
                   level="DEBUG", format="{time:YYYY-MM-DD HH:mm:ss} | {level} | {message}")
        logger.debug("Initialized Preprocessor with default configuration.")

    def load_fits_image(self, file_path: Union[str, Path]) -> np.ndarray:
        """
        Load a FITS image from the specified file path.

        :param file_path: Path to the FITS file.
        :return: Image data as a numpy array.
        """
        logger.info(f"Loading FITS image from {file_path}")
        try:
            with fits.open(str(file_path)) as hdul:
                image_data = hdul[0].data
            logger.debug("FITS image loaded successfully.")
            return image_data
        except Exception as e:
            logger.error(f"Failed to load FITS image: {e}")
            raise

    def preprocess_fits_image(self, image_data: np.ndarray) -> np.ndarray:
        """
        Preprocess FITS image by normalizing to the 0-255 range.

        :param image_data: Raw image data from the FITS file.
        :return: Preprocessed image data as a numpy array.
        """
        logger.info("Preprocessing FITS image.")
        try:
            image_data = np.nan_to_num(image_data)
            image_data = image_data.astype(np.float64)
            image_data -= np.min(image_data)
            image_data /= np.max(image_data)
            image_data *= 255
            preprocessed_image = image_data.astype(np.uint8)
            logger.debug("FITS image preprocessed successfully.")
            return preprocessed_image
        except Exception as e:
            logger.error(f"Error during FITS image preprocessing: {e}")
            raise

    def load_image(self, file_path: Union[str, Path]) -> np.ndarray:
        """
        Load an image from the specified file path. Supports FITS and standard image formats.

        :param file_path: Path to the image file.
        :return: Loaded image as a numpy array.
        """
        logger.info(f"Loading image from {file_path}")
        try:
            if str(file_path).lower().endswith('.fits'):
                image_data = self.load_fits_image(file_path)
                if image_data.ndim == 2:
                    return self.preprocess_fits_image(image_data)
                elif image_data.ndim == 3:
                    channels = [self.preprocess_fits_image(
                        image_data[..., i]) for i in range(image_data.shape[2])]
                    preprocessed_image = cv2.merge(channels)
                    logger.debug(
                        "Multichannel FITS image preprocessed successfully.")
                    return preprocessed_image
                else:
                    logger.error("Unsupported FITS image dimensions.")
                    raise ValueError("Unsupported FITS image dimensions.")
            else:
                image = cv2.imread(str(file_path), cv2.IMREAD_UNCHANGED)
                if image is None:
                    logger.error(f"Unable to load image file: {file_path}")
                    raise ValueError(f"Unable to load image file: {file_path}")
                logger.debug("Standard image loaded successfully.")
                return image
        except Exception as e:
            logger.error(f"Error loading image: {e}")
            raise

    def apply_median_filter(self, image: np.ndarray) -> np.ndarray:
        """
        Apply median filtering to the image.

        :param image: Input image.
        :return: Filtered image.
        """
        logger.info("Applying median filter.")
        try:
            filtered_image = cv2.medianBlur(
                image, self.config.median_filter_size)
            logger.debug(
                f"Median filter applied with kernel size {self.config.median_filter_size}.")
            return filtered_image
        except Exception as e:
            logger.error(f"Error applying median filter: {e}")
            raise

    def wavelet_transform(self, image: np.ndarray) -> List[np.ndarray]:
        """
        Perform wavelet transform using a Laplacian pyramid.

        :param image: Input image.
        :return: List of wavelet transformed images at each level.
        """
        logger.info("Performing wavelet transform.")
        try:
            pyramid = []
            current_image = image.copy()
            for level in range(self.config.wavelet_levels):
                down = cv2.pyrDown(current_image)
                up = cv2.pyrUp(down, current_image.shape[:2])

                # Resize up to match the original image size
                up = cv2.resize(
                    up, (current_image.shape[1], current_image.shape[0]))

                # Calculate the difference to get the detail layer
                layer = cv2.subtract(current_image, up)
                pyramid.append(layer)
                logger.debug(f"Wavelet level {level + 1} computed.")
                current_image = down

            pyramid.append(current_image)  # Add the final low-resolution image
            logger.debug("Wavelet transform completed successfully.")
            if self.config.save_wavelet and self.config.wavelet_save_path:
                self.save_wavelet_pyramid(
                    pyramid, self.config.wavelet_save_path)
            return pyramid
        except Exception as e:
            logger.error(f"Error during wavelet transform: {e}")
            raise

    def inverse_wavelet_transform(self, pyramid: List[np.ndarray]) -> np.ndarray:
        """
        Reconstruct the image from its wavelet pyramid representation.

        :param pyramid: List of wavelet transformed images at each level.
        :return: Reconstructed image.
        """
        logger.info("Performing inverse wavelet transform.")
        try:
            image = pyramid.pop()
            while pyramid:
                up = cv2.pyrUp(image, pyramid[-1].shape[:2])

                # Resize up to match the size of the current level
                up = cv2.resize(
                    up, (pyramid[-1].shape[1], pyramid[-1].shape[0]))

                # Add the detail layer to reconstruct the image
                image = cv2.add(up, pyramid.pop())
                logger.debug("Wavelet level reconstructed.")
            logger.debug("Inverse wavelet transform completed successfully.")
            return image
        except Exception as e:
            logger.error(f"Error during inverse wavelet transform: {e}")
            raise

    def background_subtraction(self, image: np.ndarray, background: np.ndarray) -> np.ndarray:
        """
        Subtract the background from the image using the provided background image.

        :param image: Original image.
        :param background: Background image to subtract.
        :return: Image with background subtracted.
        """
        logger.info("Performing background subtraction.")
        try:
            background_resized = cv2.resize(
                background, (image.shape[1], image.shape[0]))
            result = cv2.subtract(image, background_resized)
            result[result < 0] = 0
            logger.debug("Background subtracted successfully.")
            return result
        except Exception as e:
            logger.error(f"Error during background subtraction: {e}")
            raise

    def binarize(self, image: np.ndarray) -> np.ndarray:
        """
        Binarize the image using a fixed threshold from configuration.

        :param image: Input image.
        :return: Binarized image.
        """
        logger.info("Binarizing image.")
        try:
            _, binary_image = cv2.threshold(
                image, self.config.binarization_threshold, 255, cv2.THRESH_BINARY)
            logger.debug(
                f"Image binarized with threshold {self.config.binarization_threshold}.")
            return binary_image
        except Exception as e:
            logger.error(f"Error during binarization: {e}")
            raise

    def detect_stars(self, binary_image: np.ndarray) -> Tuple[List[Tuple[int, int]], List[Tuple[Tuple[int, int], float, float]]]:
        """
        Detect stars in a binary image by finding contours.

        :param binary_image: Binarized image.
        :return: Tuple containing a list of star centers and a list of star properties (center, area, perimeter).
        """
        logger.info("Detecting stars in binary image.")
        try:
            contours, _ = cv2.findContours(
                binary_image, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
            star_centers = []
            star_properties = []

            for contour in contours:
                M = cv2.moments(contour)
                if M['m00'] != 0:
                    center = (int(M['m10'] / M['m00']),
                              int(M['m01'] / M['m00']))
                    star_centers.append(center)
                    area = cv2.contourArea(contour)
                    perimeter = cv2.arcLength(contour, True)
                    star_properties.append((center, area, perimeter))
                    logger.debug(
                        f"Detected star at {center} with area {area} and perimeter {perimeter}.")

            logger.info(f"Total stars detected: {len(star_centers)}.")
            return star_centers, star_properties
        except Exception as e:
            logger.error(f"Error during star detection: {e}")
            raise

    def save_preprocessed_image(self, image: np.ndarray, save_path: Union[str, Path]) -> None:
        """
        Save the preprocessed image to the specified path.

        :param image: Preprocessed image.
        :param save_path: Path to save the image.
        """
        logger.info(f"Saving preprocessed image to {save_path}.")
        try:
            cv2.imwrite(str(save_path), image)
            logger.debug("Preprocessed image saved successfully.")
        except Exception as e:
            logger.error(f"Failed to save preprocessed image: {e}")
            raise

    def save_wavelet_pyramid(self, pyramid: List[np.ndarray], save_path: Union[str, Path]) -> None:
        """
        Save the wavelet pyramid images to the specified directory.

        :param pyramid: List of wavelet transformed images at each level.
        :param save_path: Directory path to save the wavelet images.
        """
        logger.info(f"Saving wavelet pyramid to {save_path}.")
        try:
            save_path = Path(save_path)
            save_path.mkdir(parents=True, exist_ok=True)
            for idx, layer in enumerate(pyramid):
                layer_path = save_path / f"wavelet_level_{idx + 1}.png"
                cv2.imwrite(str(layer_path), layer)
                logger.debug(f"Wavelet level {idx + 1} saved to {layer_path}.")
            logger.debug("All wavelet pyramid images saved successfully.")
        except Exception as e:
            logger.error(f"Failed to save wavelet pyramid images: {e}")
            raise

    def run_preprocessing_pipeline(self, file_path: Union[str, Path]) -> Tuple[np.ndarray, List[Tuple[int, int]], List[Tuple[Tuple[int, int], float, float]]]:
        """
        Execute the full preprocessing pipeline on the provided image file.

        :param file_path: Path to the image file.
        :return: Tuple containing the final processed image, list of star centers, and list of star properties.
        """
        logger.info("Starting preprocessing pipeline.")
        try:
            image = self.load_image(file_path)
            logger.debug("Image loaded for preprocessing.")

            if self.config.save_preprocessed and self.config.preprocessed_save_path:
                self.save_preprocessed_image(
                    image, self.config.preprocessed_save_path)

            filtered_image = self.apply_median_filter(image)
            logger.debug("Median filter applied.")

            wavelet_pyramid = self.wavelet_transform(filtered_image)
            reconstructed_image = self.inverse_wavelet_transform(
                wavelet_pyramid)
            logger.debug("Wavelet transform and inverse transform completed.")

            # Placeholder for actual background
            background = np.zeros_like(reconstructed_image)
            subtracted_image = self.background_subtraction(
                reconstructed_image, background)
            logger.debug("Background subtraction completed.")

            binary_image = self.binarize(subtracted_image)
            logger.debug("Image binarization completed.")

            star_centers, star_properties = self.detect_stars(binary_image)
            logger.debug("Star detection completed.")

            logger.info("Preprocessing pipeline completed successfully.")
            return binary_image, star_centers, star_properties
        except Exception as e:
            logger.error(f"Error in preprocessing pipeline: {e}")
            raise


# Example Usage
if __name__ == "__main__":
    import sys

    # Initialize Preprocessor with custom configuration
    config = PreprocessingConfig(
        median_filter_size=5,
        wavelet_levels=4,
        binarization_threshold=100,
        save_preprocessed=True,
        preprocessed_save_path=Path("preprocessed_image.png"),
        save_wavelet=True,
        wavelet_save_path=Path("wavelet_pyramid")
    )
    preprocessor = Preprocessor(config=config)

    # Path to the input image (FITS or standard format)
    input_image_path = "path/to/image.fits"

    try:
        # Run preprocessing pipeline
        binary_img, centers, properties = preprocessor.run_preprocessing_pipeline(
            input_image_path)
        print(f"Detected {len(centers)} stars.")

        # Optionally, visualize binary image
        plt.imshow(binary_img, cmap='gray')
        plt.title('Binarized Image')
        plt.show()

    except Exception as e:
        print(f"An error occurred during preprocessing: {e}")
