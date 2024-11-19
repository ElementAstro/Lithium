import cv2
import numpy as np
from typing import Optional, List, Tuple, Union
from pathlib import Path
from loguru import logger
from .utils import save_image, load_image


@dataclass
class HistogramConfig:
    """
    Configuration parameters for histogram-based image processing.
    """
    clip_shadow: float = 0.01
    clip_highlight: float = 0.01
    target_median: int = 128
    method: str = 'gamma'  # 'gamma', 'logarithmic', 'mtf'
    apply_clahe: bool = False
    clahe_clip_limit: float = 2.0
    clahe_tile_grid_size: Tuple[int, int] = (8, 8)
    apply_noise_reduction: bool = False
    noise_reduction_method: str = 'median'  # 'median', 'gaussian'
    apply_sharpening: bool = False
    sharpening_strength: float = 1.0
    batch_process: bool = False
    file_list: Optional[List[Union[str, Path]]] = None
    output_directory: Optional[Union[str, Path]] = None


class HistogramProcessor:
    """
    Handles automated histogram transformations and enhancements for images.
    """

    def __init__(self, config: Optional[HistogramConfig] = None) -> None:
        """
        Initialize HistogramProcessor with configuration.

        :param config: HistogramConfig object containing processing settings.
        """
        self.config = config or HistogramConfig()
        logger.debug(
            f"HistogramProcessor initialized with config: {self.config}")

    def histogram_clipping(self, image: np.ndarray) -> np.ndarray:
        """
        Clip the histogram of the image based on shadow and highlight percentages.

        :param image: Input image.
        :return: Clipped image.
        """
        logger.debug("Starting histogram clipping.")
        flat = image.flatten()
        low_val = np.percentile(flat, self.config.clip_shadow * 100)
        high_val = np.percentile(flat, 100 - self.config.clip_highlight * 100)
        clipped_image = np.clip(image, low_val, high_val).astype(np.uint8)
        logger.debug("Histogram clipping completed.")
        return clipped_image

    def gamma_transformation(self, image: np.ndarray) -> np.ndarray:
        """
        Apply gamma transformation to the image.

        :param image: Input image.
        :return: Gamma transformed image.
        """
        logger.debug("Starting gamma transformation.")
        try:
            mean_val = np.median(image)
            if mean_val == 0:
                logger.warning(
                    "Median value is zero during gamma transformation.")
                return image
            gamma = np.log(self.config.target_median / 255.0) / \
                np.log(mean_val / 255.0)
            gamma_corrected = np.array(
                255 * (image / 255.0) ** gamma, dtype='uint8')
            logger.debug("Gamma transformation completed.")
            return gamma_corrected
        except Exception as e:
            logger.error(f"Error during gamma transformation: {e}")
            raise

    def logarithmic_transformation(self, image: np.ndarray) -> np.ndarray:
        """
        Apply logarithmic transformation to the image.

        :param image: Input image.
        :return: Logarithmically transformed image.
        """
        logger.debug("Starting logarithmic transformation.")
        try:
            c = 255 / np.log(1 + np.max(image))
            log_transformed = np.array(c * np.log(1 + image), dtype='uint8')
            logger.debug("Logarithmic transformation completed.")
            return log_transformed
        except Exception as e:
            logger.error(f"Error during logarithmic transformation: {e}")
            raise

    def mtf_transformation(self, image: np.ndarray) -> np.ndarray:
        """
        Apply MTF transformation to the image.

        :param image: Input image.
        :return: MTF transformed image.
        """
        logger.debug("Starting MTF transformation.")
        try:
            mean_val = np.median(image)
            if mean_val == 0:
                logger.warning(
                    "Median value is zero during MTF transformation.")
                return image
            mtf = self.config.target_median / mean_val
            mtf_transformed = np.clip(image * mtf, 0, 255).astype(np.uint8)
            logger.debug("MTF transformation completed.")
            return mtf_transformed
        except Exception as e:
            logger.error(f"Error during MTF transformation: {e}")
            raise

    def apply_clahe_method(self, image: np.ndarray) -> np.ndarray:
        """
        Apply CLAHE to the image.

        :param image: Input image.
        :return: CLAHE applied image.
        """
        logger.debug("Starting CLAHE.")
        try:
            clahe = cv2.createCLAHE(
                clipLimit=self.config.clahe_clip_limit,
                tileGridSize=self.config.clahe_tile_grid_size
            )
            if len(image.shape) == 2:
                clahe_applied = clahe.apply(image)
            else:
                channels = cv2.split(image)
                clahe_applied = cv2.merge([clahe.apply(ch) for ch in channels])
            logger.debug("CLAHE completed.")
            return clahe_applied
        except Exception as e:
            logger.error(f"Error during CLAHE application: {e}")
            raise

    def noise_reduction(self, image: np.ndarray) -> np.ndarray:
        """
        Apply noise reduction to the image.

        :param image: Input image.
        :return: Noise reduced image.
        """
        logger.debug("Starting noise reduction.")
        try:
            if self.config.noise_reduction_method == 'median':
                reduced = cv2.medianBlur(image, 3)
            elif self.config.noise_reduction_method == 'gaussian':
                reduced = cv2.GaussianBlur(image, (3, 3), 0)
            else:
                logger.error(
                    f"Unsupported noise reduction method: {self.config.noise_reduction_method}")
                raise ValueError(
                    f"Unsupported noise reduction method: {self.config.noise_reduction_method}")
            logger.debug("Noise reduction completed.")
            return reduced
        except Exception as e:
            logger.error(f"Error during noise reduction: {e}")
            raise

    def sharpen_image(self, image: np.ndarray) -> np.ndarray:
        """
        Sharpen the image.

        :param image: Input image.
        :return: Sharpened image.
        """
        logger.debug("Starting image sharpening.")
        try:
            kernel = np.array([[-1, -1, -1],
                               [-1, 9 + self.config.sharpening_strength, -1],
                               [-1, -1, -1]])
            sharpened = cv2.filter2D(image, -1, kernel)
            logger.debug("Image sharpening completed.")
            return sharpened
        except Exception as e:
            logger.error(f"Error during image sharpening: {e}")
            raise

    def process_single_image(self, image: np.ndarray) -> np.ndarray:
        """
        Process a single image with the configured transformations.

        :param image: Input image.
        :return: Processed image.
        """
        logger.info("Processing a single image.")
        try:
            if self.config.apply_noise_reduction:
                image = self.noise_reduction(image)

            image = self.histogram_clipping(image)

            if self.config.method == 'gamma':
                image = self.gamma_transformation(image)
            elif self.config.method == 'logarithmic':
                image = self.logarithmic_transformation(image)
            elif self.config.method == 'mtf':
                image = self.mtf_transformation(image)
            else:
                logger.error(f"Invalid method specified: {self.config.method}")
                raise ValueError(
                    f"Invalid method specified: {self.config.method}")

            if self.config.apply_clahe:
                image = self.apply_clahe_method(image)

            if self.config.apply_sharpening:
                image = self.sharpen_image(image)

            logger.info("Image processing completed successfully.")
            return image
        except Exception as e:
            logger.error(f"Error during image processing: {e}")
            raise

    def process_batch_images(self, file_list: List[Union[str, Path]]) -> List[np.ndarray]:
        """
        Process a batch of images.

        :param file_list: List of image file paths.
        :return: List of processed images.
        """
        logger.info(f"Starting batch processing of {len(file_list)} images.")
        processed_images = []
        for idx, file_path in enumerate(file_list, start=1):
            logger.info(
                f"Processing image {idx}/{len(file_list)}: {file_path}")
            image = load_image(file_path, grayscale=False)
            if image is None:
                logger.warning(
                    f"Skipping image due to load failure: {file_path}")
                continue
            processed_image = self.process_single_image(image)
            processed_images.append(processed_image)
            if self.config.output_directory:
                output_path = Path(self.config.output_directory) / \
                    f"processed_{Path(file_path).name}"
                save_image(output_path, processed_image)
        logger.info("Batch processing completed.")
        return processed_images

    def process(self, image: Optional[np.ndarray] = None) -> Optional[Union[np.ndarray, List[np.ndarray]]]:
        """
        Process images based on the configuration.

        :param image: Single image to process. Required if batch_process is False.
        :return: Processed single image or list of processed images.
        """
        if self.config.batch_process:
            if not self.config.file_list:
                logger.error(
                    "File list must be provided for batch processing.")
                raise ValueError(
                    "File list must be provided for batch processing.")
            return self.process_batch_images(self.config.file_list)
        else:
            if image is None:
                logger.error("Image must be provided for single processing.")
                raise ValueError(
                    "Image must be provided for single processing.")
            return self.process_single_image(image)
