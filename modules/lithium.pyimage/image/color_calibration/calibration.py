import cv2
import numpy as np
from typing import Tuple, Optional, List
from dataclasses import dataclass, field
from pathlib import Path
from loguru import logger


@dataclass
class ColorCalibrationConfig:
    """Configuration parameters for color calibration."""
    gamma: float = 1.0  # Gamma correction value
    # 'gray_world', 'white_patch', 'learning_based'
    white_balance_method: str = 'gray_world'
    calibration_save_path: Optional[Path] = None


class ColorCalibration:
    """Handles color calibration tasks for astronomical images."""

    def __init__(self, image: np.ndarray, config: Optional[ColorCalibrationConfig] = None) -> None:
        """
        Initialize the ColorCalibration class.

        :param image: Input RGB image
        :param config: Color calibration configuration
        """
        self.image = image
        self.config = config or ColorCalibrationConfig()

        # Configure logging
        logger.remove()
        logger.add(
            "color_calibration.log",
            rotation="10 MB",
            retention="10 days",
            level="DEBUG",
            format="{time} | {level} | {message}"
        )
        logger.debug(
            "ColorCalibration initialized with configuration: {}", self.config)

    def gamma_correction(self) -> np.ndarray:
        """
        Apply gamma correction to the image.

        :return: Gamma corrected image
        """
        logger.info("Starting gamma correction with gamma value: {}",
                    self.config.gamma)
        try:
            inv_gamma = 1.0 / self.config.gamma
            table = np.array([((i / 255.0) ** inv_gamma) *
                             255 for i in np.arange(256)]).astype("uint8")
            corrected_image = cv2.LUT(self.image, table)
            logger.debug("Gamma correction completed.")
            return corrected_image
        except Exception as e:
            logger.error("Error during gamma correction: {}", e)
            raise

    def apply_white_balance(self) -> np.ndarray:
        """
        Apply white balance to the image based on the configured method.

        :return: White balanced image
        """
        method = self.config.white_balance_method.lower()
        logger.info("Starting white balance with method: {}", method)
        if method == 'gray_world':
            return self.gray_world_white_balance()
        elif method == 'white_patch':
            return self.white_patch_white_balance()
        elif method == 'learning_based':
            return self.learning_based_white_balance()
        else:
            logger.error("Unsupported white balance method: {}", method)
            raise ValueError(f"Unsupported white balance method: {method}")

    def gray_world_white_balance(self) -> np.ndarray:
        """
        Apply gray world white balance algorithm.

        :return: White balanced image
        """
        logger.debug("Applying gray world white balance.")
        try:
            mean_values = np.mean(self.image, axis=(0, 1))
            gray_value = np.mean(mean_values)
            factors = gray_value / mean_values
            balanced_image = self.apply_color_factors(factors)
            logger.debug("Gray world white balance completed.")
            return balanced_image
        except Exception as e:
            logger.error("Error during gray world white balance: {}", e)
            raise

    def white_patch_white_balance(self) -> np.ndarray:
        """
        Apply white patch white balance algorithm.

        :return: White balanced image
        """
        logger.debug("Applying white patch white balance.")
        try:
            max_values = np.max(self.image, axis=(0, 1))
            factors = 255.0 / max_values
            balanced_image = self.apply_color_factors(factors)
            logger.debug("White patch white balance completed.")
            return balanced_image
        except Exception as e:
            logger.error("Error during white patch white balance: {}", e)
            raise

    def learning_based_white_balance(self) -> np.ndarray:
        """
        Apply learning-based white balance algorithm.

        :return: White balanced image
        """
        logger.debug("Applying learning-based white balance.")
        try:
            wb = cv2.xphoto.createLearningBasedWB()
            balanced_image = wb.balanceWhite(self.image)
            logger.debug("Learning-based white balance completed.")
            return balanced_image
        except Exception as e:
            logger.error("Error during learning-based white balance: {}", e)
            raise

    def apply_color_factors(self, factors: np.ndarray) -> np.ndarray:
        """
        Apply color correction factors.

        :param factors: Color correction factors
        :return: Corrected image
        """
        logger.debug("Applying color correction factors: {}", factors)
        try:
            calibrated_image = self.image.astype(np.float32)
            for i in range(3):
                calibrated_image[:, :, i] *= factors[i]
            calibrated_image = np.clip(
                calibrated_image, 0, 255).astype(np.uint8)
            logger.debug("Color correction completed.")
            return calibrated_image
        except Exception as e:
            logger.error("Error applying color correction factors: {}", e)
            raise

    def save_calibration_parameters(self) -> None:
        """
        Save calibration parameters to a file.

        """
        if self.config.calibration_save_path:
            logger.info("Saving calibration parameters to file: {}",
                        self.config.calibration_save_path)
            try:
                calibration_data = {
                    'gamma': self.config.gamma,
                    'white_balance_method': self.config.white_balance_method
                }
                np.save(self.config.calibration_save_path, calibration_data)
                logger.debug("Calibration parameters saved successfully.")
            except Exception as e:
                logger.error("Error saving calibration parameters: {}", e)
                raise

    def load_calibration_parameters(self) -> None:
        """
        Load calibration parameters from a file.

        """
        if self.config.calibration_save_path and self.config.calibration_save_path.exists():
            logger.info("Loading calibration parameters from file: {}",
                        self.config.calibration_save_path)
            try:
                calibration_data = np.load(
                    self.config.calibration_save_path, allow_pickle=True).item()
                self.config.gamma = calibration_data.get(
                    'gamma', self.config.gamma)
                self.config.white_balance_method = calibration_data.get(
                    'white_balance_method', self.config.white_balance_method)
                logger.debug("Calibration parameters loaded successfully.")
            except Exception as e:
                logger.error("Error loading calibration parameters: {}", e)
                raise

    def batch_process(self, image_list: List[np.ndarray]) -> List[np.ndarray]:
        """
        Batch process a list of images.

        :param image_list: List of images to process
        :return: List of processed images
        """
        logger.info("Starting batch processing of {} images.", len(image_list))
        processed_images = []
        try:
            for idx, img in enumerate(image_list):
                logger.debug("Processing image {}.", idx + 1)
                self.image = img
                corrected_image = self.apply_white_balance()
                corrected_image = self.gamma_correction()
                processed_images.append(corrected_image)
            logger.info("Batch processing completed.")
            return processed_images
        except Exception as e:
            logger.error("Error during batch processing: {}", e)
            raise

    def adjust_saturation(self, saturation_scale: float = 1.0) -> np.ndarray:
        """
        Adjust the saturation of the image.

        :param saturation_scale: Saturation adjustment factor
        :return: Image with adjusted saturation
        """
        logger.info("Adjusting image saturation with scale: {}",
                    saturation_scale)
        try:
            hsv_image = cv2.cvtColor(
                self.image, cv2.COLOR_BGR2HSV).astype(np.float32)
            hsv_image[:, :, 1] *= saturation_scale
            hsv_image[:, :, 1] = np.clip(hsv_image[:, :, 1], 0, 255)
            adjusted_image = cv2.cvtColor(
                hsv_image.astype(np.uint8), cv2.COLOR_HSV2BGR)
            logger.debug("Saturation adjustment completed.")
            return adjusted_image
        except Exception as e:
            logger.error("Error adjusting saturation: {}", e)
            raise

    def adjust_brightness(self, brightness_offset: int = 0) -> np.ndarray:
        """
        Adjust the brightness of the image.

        :param brightness_offset: Brightness offset value
        :return: Image with adjusted brightness
        """
        logger.info("Adjusting image brightness with offset: {}",
                    brightness_offset)
        try:
            adjusted_image = cv2.convertScaleAbs(
                self.image, alpha=1, beta=brightness_offset)
            logger.debug("Brightness adjustment completed.")
            return adjusted_image
        except Exception as e:
            logger.error("Error adjusting brightness: {}", e)
            raise


# Example usage
if __name__ == "__main__":
    # Load image
    image_path = "path/to/image.jpg"
    image = cv2.imread(image_path)
    if image is None:
        logger.error("Unable to load image: {}", image_path)
        exit(1)

    # Configure color calibration parameters
    calibration_config = ColorCalibrationConfig(
        gamma=2.2,
        white_balance_method='gray_world',
        calibration_save_path=Path("calibration_params.npy")
    )

    # Initialize color calibration object
    color_calibrator = ColorCalibration(image=image, config=calibration_config)

    # Apply white balance
    balanced_image = color_calibrator.apply_white_balance()

    # Apply gamma correction
    corrected_image = color_calibrator.gamma_correction()

    # Adjust saturation
    adjusted_image = color_calibrator.adjust_saturation(saturation_scale=1.2)

    # Adjust brightness
    bright_image = color_calibrator.adjust_brightness(brightness_offset=10)

    # Save calibration parameters
    color_calibrator.save_calibration_parameters()

    # Display results
    cv2.imshow("Original Image", image)
    cv2.imshow("Corrected Image", corrected_image)
    cv2.waitKey(0)
    cv2.destroyAllWindows()
