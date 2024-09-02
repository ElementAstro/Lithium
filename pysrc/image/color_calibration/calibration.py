import cv2
import numpy as np
from typing import Tuple
from dataclasses import dataclass

@dataclass
class ColorCalibration:
    """Class to handle color calibration tasks for astronomical images."""
    image: np.ndarray

    def calculate_color_factors(self, white_reference: np.ndarray) -> np.ndarray:
        """
        Calculate the color calibration factors based on a white reference image.

        Parameters:
            white_reference (np.ndarray): The white reference region.

        Returns:
            np.ndarray: The calibration factors for the RGB channels.
        """
        mean_values = np.mean(white_reference, axis=(0, 1))
        factors = 1.0 / mean_values
        return factors

    def apply_color_calibration(self, factors: np.ndarray) -> np.ndarray:
        """
        Apply color calibration to the image using provided factors.

        Parameters:
            factors (np.ndarray): The RGB calibration factors.

        Returns:
            np.ndarray: Color calibrated image.
        """
        calibrated_image = np.zeros_like(self.image)
        for i in range(3):
            calibrated_image[:, :, i] = self.image[:, :, i] * factors[i]
        return calibrated_image

    def automatic_white_balance(self) -> np.ndarray:
        """
        Perform automatic white balance using the Gray World algorithm.

        Returns:
            np.ndarray: White balanced image.
        """
        mean_values = np.mean(self.image, axis=(0, 1))
        gray_value = np.mean(mean_values)
        factors = gray_value / mean_values
        return self.apply_color_calibration(factors)

    def match_histograms(self, reference_image: np.ndarray) -> np.ndarray:
        """
        Match the color histogram of the image to a reference image.

        Parameters:
            reference_image (np.ndarray): The reference image whose histogram is to be matched.

        Returns:
            np.ndarray: Histogram matched image.
        """
        matched_image = np.zeros_like(self.image)
        for i in range(3):
            matched_image[:, :, i] = exposure.match_histograms(self.image[:, :, i], reference_image[:, :, i], multichannel=False)
        return matched_image
