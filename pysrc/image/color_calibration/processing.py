import cv2
import numpy as np
from skimage import exposure, filters, feature
from dataclasses import dataclass

@dataclass
class ImageProcessing:
    """Class to handle image processing tasks for astronomical images."""
    image: np.ndarray

    def apply_gamma_correction(self, gamma: float) -> np.ndarray:
        """
        Apply gamma correction to the image to adjust the contrast.
        
        Parameters:
            gamma (float): The gamma value to apply. < 1 makes the image darker, > 1 makes it brighter.

        Returns:
            np.ndarray: Gamma-corrected image.
        """
        inv_gamma = 1.0 / gamma
        table = np.array([((i / 255.0) ** inv_gamma) * 255 for i in np.arange(0, 256)]).astype("float32")
        corrected_image = cv2.LUT(self.image, table)
        return corrected_image

    def histogram_equalization(self) -> np.ndarray:
        """
        Apply histogram equalization to improve the contrast of the image.

        Returns:
            np.ndarray: Histogram equalized image.
        """
        equalized_image = np.zeros_like(self.image)
        for i in range(3):
            equalized_image[:, :, i] = exposure.equalize_hist(self.image[:, :, i])
        return equalized_image

    def denoise_image(self, h: float = 10.0) -> np.ndarray:
        """
        Apply Non-Local Means Denoising to the image.

        Parameters:
            h (float): Filter strength. Higher h removes noise more aggressively.

        Returns:
            np.ndarray: Denoised image.
        """
        denoised_image = cv2.fastNlMeansDenoisingColored(self.image, None, h, h, 7, 21)
        return denoised_image

    def detect_stars(self, min_distance: int = 10, threshold_rel: float = 0.2) -> np.ndarray:
        """
        Detect stars in the image using peak local max method.

        Parameters:
            min_distance (int): Minimum number of pixels separating peaks in a region of 2 * min_distance + 1.
            threshold_rel (float): Minimum intensity of peaks relative to the highest peak.

        Returns:
            np.ndarray: Coordinates of the detected stars.
        """
        gray_image = cv2.cvtColor((self.image * 255).astype(np.uint8), cv2.COLOR_RGB2GRAY)
        coordinates = feature.peak_local_max(gray_image, min_distance=min_distance, threshold_rel=threshold_rel)
        return coordinates
