import argparse
import json
from dataclasses import dataclass, field
from pathlib import Path
from typing import Dict, Optional, Tuple, List

import cv2
import numpy as np
from astropy.io import fits
from concurrent.futures import ThreadPoolExecutor
from loguru import logger
from scipy import ndimage
import yaml


@dataclass
class ImageProcessingConfig:
    """Configuration parameters for image processing."""
    remove_hot_pixels: bool = False
    denoise: bool = False
    equalize_histogram: bool = False
    apply_clahe: bool = False
    clahe_clip_limit: float = 2.0
    clahe_tile_grid_size: Tuple[int, int] = (8, 8)
    unsharp_mask: bool = False
    unsharp_amount: float = 1.5
    adjust_gamma: bool = False
    gamma_value: float = 1.0
    apply_gaussian_blur: bool = False
    gaussian_kernel_size: int = 5
    do_stretch: bool = False
    do_star_count: bool = False
    do_star_mark: bool = False
    resize_size: int = 2048
    bayer_type: Optional[str] = None
    config_file: Optional[Path] = None
    save_jpg: bool = False
    jpg_file: Optional[Path] = None
    save_star_data: bool = False
    star_file: Optional[Path] = None


class ImageProcessor:
    """Handles image processing tasks including loading, enhancing, and saving images."""

    def __init__(self, config: ImageProcessingConfig) -> None:
        """
        Initialize the ImageProcessor with the given configuration.

        :param config: ImageProcessingConfig object containing processing settings.
        """
        self.config = config

        # Configure logging with loguru
        logger.remove()
        logger.add(
            "image_processor.log",
            rotation="10 MB",
            retention="10 days",
            level="DEBUG",
            format="{time} | {level} | {message}"
        )
        logger.debug(f"ImageProcessor initialized with config: {self.config}")

    def debayer_image(self, img: np.ndarray) -> np.ndarray:
        """Convert a raw image using the specified Bayer pattern to a color image."""
        logger.debug("Starting debayering process.")
        bayer_patterns = {
            "rggb": cv2.COLOR_BAYER_RGGB2BGR,
            "gbrg": cv2.COLOR_BAYER_GBRG2BGR,
            "bggr": cv2.COLOR_BAYER_BGGR2BGR,
            "grbg": cv2.COLOR_BAYER_GRBG2BGR
        }
        pattern = self.config.bayer_type.lower() if self.config.bayer_type else 'rggb'
        converted_img = cv2.cvtColor(
            img, bayer_patterns.get(pattern, cv2.COLOR_BAYER_RGGB2BGR))
        logger.debug("Debayering completed.")
        return converted_img

    def resize_image(self, img: np.ndarray) -> np.ndarray:
        """Resize the image to the target size while maintaining aspect ratio."""
        logger.debug("Starting image resizing.")
        scale = min(self.config.resize_size / max(img.shape[:2]), 1)
        if scale < 1:
            resized_img = cv2.resize(
                img, None, fx=scale, fy=scale, interpolation=cv2.INTER_AREA)
            logger.debug(
                f"Image resized to {resized_img.shape[1]}x{resized_img.shape[0]}")
            return resized_img
        logger.debug("No resizing needed.")
        return img

    def normalize_image(self, img: np.ndarray) -> np.ndarray:
        """Normalize the image to 8-bit if it's not already."""
        logger.debug("Starting image normalization.")
        if img.dtype != np.uint8:
            normalized_img = cv2.normalize(
                img, None, alpha=0, beta=255, norm_type=cv2.NORM_MINMAX)
            normalized_img = normalized_img.astype(np.uint8)
            logger.debug("Image normalization completed.")
            return normalized_img
        logger.debug("Image is already in 8-bit format.")
        return img

    def stretch_image(self, img: np.ndarray, is_color: bool) -> np.ndarray:
        """Apply stretching to the image."""
        logger.debug("Starting image stretching.")
        # Implement stretching logic here
        # Placeholder implementation
        stretched_img = img  # Replace with actual stretching function
        logger.debug("Image stretching completed.")
        return stretched_img

    def enhance_image(self, img: np.ndarray) -> np.ndarray:
        """Apply a series of image enhancements based on the configuration."""
        logger.debug("Starting image enhancement.")
        if self.config.remove_hot_pixels:
            img = self.remove_hot_pixels(img)
        if self.config.denoise:
            img = self.denoise_image(img)
        if self.config.equalize_histogram:
            img = self.equalize_histogram(img)
        if self.config.apply_clahe:
            img = self.apply_clahe(img)
        if self.config.unsharp_mask:
            img = self.apply_unsharp_mask(img)
        if self.config.adjust_gamma:
            img = self.adjust_gamma(img)
        if self.config.apply_gaussian_blur:
            img = self.apply_gaussian_blur(img)
        logger.debug("Image enhancement completed.")
        return img

    def remove_hot_pixels(self, img: np.ndarray, threshold: float = 3.0) -> np.ndarray:
        """Remove hot pixels using median filter and thresholding."""
        logger.debug("Removing hot pixels from the image.")
        median = ndimage.median_filter(img, size=3)
        diff = np.abs(img - median)
        std_dev = np.std(diff)
        mask = diff > (threshold * std_dev)
        img[mask] = median[mask]
        logger.debug("Hot pixels removed.")
        return img

    def denoise_image(self, img: np.ndarray, h: float = 10) -> np.ndarray:
        """Apply Non-Local Means Denoising."""
        logger.debug("Applying denoising to the image.")
        denoised_img = cv2.fastNlMeansDenoisingColored(img, None, h, h, 7, 21)
        logger.debug("Denoising completed.")
        return denoised_img

    def equalize_histogram(self, img: np.ndarray) -> np.ndarray:
        """Apply histogram equalization to improve contrast."""
        logger.debug("Applying histogram equalization.")
        if len(img.shape) == 2:
            equalized_img = cv2.equalizeHist(img)
        else:
            ycrcb = cv2.cvtColor(img, cv2.COLOR_BGR2YCrCb)
            ycrcb[:, :, 0] = cv2.equalizeHist(ycrcb[:, :, 0])
            equalized_img = cv2.cvtColor(ycrcb, cv2.COLOR_YCrCb2BGR)
        logger.debug("Histogram equalization completed.")
        return equalized_img

    def apply_clahe(self, img: np.ndarray) -> np.ndarray:
        """Apply CLAHE to the image."""
        logger.debug("Applying CLAHE to the image.")
        clahe = cv2.createCLAHE(
            clipLimit=self.config.clahe_clip_limit,
            tileGridSize=self.config.clahe_tile_grid_size
        )
        if len(img.shape) == 2:
            clahe_img = clahe.apply(img)
        else:
            lab = cv2.cvtColor(img, cv2.COLOR_BGR2LAB)
            channels = cv2.split(lab)
            channels[0] = clahe.apply(channels[0])
            lab = cv2.merge(channels)
            clahe_img = cv2.cvtColor(lab, cv2.COLOR_LAB2BGR)
        logger.debug("CLAHE applied.")
        return clahe_img

    def apply_unsharp_mask(self, img: np.ndarray) -> np.ndarray:
        """Apply unsharp mask to enhance image details."""
        logger.debug("Applying unsharp mask.")
        kernel_size = (self.config.gaussian_kernel_size,
                       self.config.gaussian_kernel_size)
        blurred = cv2.GaussianBlur(img, kernel_size, 0)
        sharpened = cv2.addWeighted(
            img, 1 + self.config.unsharp_amount, blurred, -self.config.unsharp_amount, 0)
        logger.debug("Unsharp mask applied.")
        return sharpened

    def adjust_gamma(self, img: np.ndarray) -> np.ndarray:
        """Adjust image gamma."""
        logger.debug(f"Adjusting gamma with value: {self.config.gamma_value}")
        inv_gamma = 1.0 / self.config.gamma_value
        table = np.array([((i / 255.0) ** inv_gamma) * 255
                          for i in np.arange(256)]).astype("uint8")
        gamma_img = cv2.LUT(img, table)
        logger.debug("Gamma adjustment completed.")
        return gamma_img

    def apply_gaussian_blur(self, img: np.ndarray) -> np.ndarray:
        """Apply Gaussian blur to reduce noise."""
        logger.debug("Applying Gaussian blur.")
        kernel_size = (self.config.gaussian_kernel_size,
                       self.config.gaussian_kernel_size)
        blurred_img = cv2.GaussianBlur(img, kernel_size, 0)
        logger.debug("Gaussian blur applied.")
        return blurred_img

    def process_image(self, filepath: Path) -> Tuple[Optional[np.ndarray], Dict[str, float]]:
        """Process a single image file."""
        logger.info(f"Processing image: {filepath}")
        try:
            img_data = fits.getdata(filepath, header=True)
            if isinstance(img_data, tuple):
                img, header = img_data
            else:
                img = img_data
                header = {}

            is_color = 'BAYERPAT' in header
            if is_color:
                self.config.bayer_type = self.config.bayer_type or header.get(
                    'BAYERPAT', 'rggb')
                img = self.debayer_image(img)

            img = self.resize_image(img)
            img = self.normalize_image(img)

            # Apply image enhancements
            img = self.enhance_image(img)

            if self.config.do_stretch:
                img = self.stretch_image(img, is_color)

            result = {"star_count": -1, "average_hfr": -1.0,
                      "max_star": -1.0, "min_star": -1.0, "average_star": -1.0}
            if self.config.do_star_count:
                img, star_count, avg_hfr, area_range = self.detect_stars(img)
                result.update({
                    "star_count": star_count,
                    "average_hfr": avg_hfr,
                    "max_star": area_range['max'],
                    "min_star": area_range['min'],
                    "average_star": area_range['average']
                })

            if self.config.save_jpg and self.config.jpg_file and img is not None:
                cv2.imwrite(str(self.config.jpg_file), img)
                logger.info(f"Processed image saved to {self.config.jpg_file}")

            if self.config.save_star_data and self.config.star_file:
                with self.config.star_file.open('w') as f:
                    json.dump(result, f)
                logger.info(
                    f"Star count results saved to {self.config.star_file}")

            logger.info("Image processing completed successfully.")
            return img, result

        except Exception as e:
            logger.error(f"Error processing image {filepath}: {e}")
            return None, {"star_count": -1, "average_hfr": -1.0, "max_star": -1.0, "min_star": -1.0, "average_star": -1.0}

    def detect_stars(self, img: np.ndarray) -> Tuple[np.ndarray, int, float, Dict[str, float]]:
        """Detect stars in the image and calculate relevant metrics."""
        logger.debug("Starting star detection.")
        # Placeholder implementation
        # Implement actual star detection logic here
        star_count = 0
        average_hfr = 0.0
        area_range = {"max": 0.0, "min": 0.0, "average": 0.0}
        logger.debug("Star detection completed.")
        return img, star_count, average_hfr, area_range

    def load_config(self) -> None:
        """Load configuration from a YAML file if provided."""
        if self.config.config_file and self.config.config_file.is_file():
            logger.info(
                f"Loading configuration from {self.config.config_file}")
            try:
                with self.config.config_file.open('r') as f:
                    file_config = yaml.safe_load(f)
                for key, value in file_config.items():
                    if hasattr(self.config, key):
                        setattr(self.config, key, value)
                logger.debug("Configuration loaded from file.")
            except Exception as e:
                logger.error(f"Error loading configuration file: {e}")
        else:
            logger.debug(
                "No configuration file provided or file does not exist.")

    def main(self, filepath: Path) -> None:
        """Main processing function."""
        self.load_config()
        self.process_image(filepath)


if __name__ == "__main__":
    # Parse command-line arguments
    parser = argparse.ArgumentParser(
        description="Image Enhancement and Star Detection Script")
    parser.add_argument("filepath", type=Path, help="Path to the FITS file")
    parser.add_argument("--config", type=Path,
                        help="Path to the configuration YAML file")
    parser.add_argument("--resize-size", type=int, default=2048,
                        help="Target size for resizing the image")
    parser.add_argument("--jpg-file", type=Path,
                        help="Path to save the processed image as JPG")
    parser.add_argument("--star-file", type=Path,
                        help="Path to save the star count results as JSON")
    parser.add_argument("--remove-hot-pixels", action="store_true",
                        help="Remove hot pixels in the image")
    parser.add_argument("--denoise", action="store_true",
                        help="Apply denoising to the image")
    parser.add_argument("--equalize-histogram",
                        action="store_true", help="Apply histogram equalization")
    parser.add_argument("--apply-clahe", action="store_true",
                        help="Apply CLAHE to the image")
    parser.add_argument("--unsharp-mask", action="store_true",
                        help="Apply unsharp mask")
    parser.add_argument("--adjust-gamma", action="store_true",
                        help="Adjust gamma of the image")
    parser.add_argument("--gamma-value", type=float,
                        default=1.0, help="Gamma value for adjustment")
    parser.add_argument("--apply-gaussian-blur",
                        action="store_true", help="Apply Gaussian blur")
    parser.add_argument("--do-stretch", action="store_true",
                        help="Apply stretching to the image")
    parser.add_argument("--do-star-count", action="store_true",
                        help="Perform star count on the image")
    parser.add_argument("--do-star-mark", action="store_true",
                        help="Mark detected stars on the image")
    args = parser.parse_args()

    # Create configuration
    config = ImageProcessingConfig(
        remove_hot_pixels=args.remove_hot_pixels,
        denoise=args.denoise,
        equalize_histogram=args.equalize_histogram,
        apply_clahe=args.apply_clahe,
        unsharp_mask=args.unsharp_mask,
        adjust_gamma=args.adjust_gamma,
        gamma_value=args.gamma_value,
        apply_gaussian_blur=args.apply_gaussian_blur,
        do_stretch=args.do_stretch,
        do_star_count=args.do_star_count,
        do_star_mark=args.do_star_mark,
        resize_size=args.resize_size,
        config_file=args.config,
        save_jpg=bool(args.jpg_file),
        jpg_file=args.jpg_file,
        save_star_data=bool(args.star_file),
        star_file=args.star_file
    )

    # Initialize and run the processor
    processor = ImageProcessor(config)
    processor.main(args.filepath)
