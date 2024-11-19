import cv2
import numpy as np
from typing import List, Tuple, Optional, Union
from pathlib import Path
from dataclasses import dataclass, field
from loguru import logger
from sklearn.cluster import DBSCAN
import matplotlib.pyplot as plt
import json

from .preprocessing import (
    Preprocessor,
    PreprocessingConfig
)


@dataclass
class StarDetectionConfig:
    """
    Configuration settings for star detection.
    """
    median_filter_size: int = 3
    wavelet_levels: int = 4
    binarization_threshold: int = 30
    min_star_size: int = 10
    min_star_brightness: int = 20
    min_circularity: float = 0.7
    max_circularity: float = 1.3
    scales: List[float] = field(default_factory=lambda: [1.0, 0.75, 0.5])
    dbscan_eps: float = 10.0
    dbscan_min_samples: int = 2
    save_detected_stars: bool = False
    detected_stars_save_path: Optional[Path] = None
    visualize: bool = True
    visualization_save_path: Optional[Path] = None


class StarDetector:
    def __init__(self, config: Optional[StarDetectionConfig] = None) -> None:
        """
        Initialize the StarDetector with optional configuration.

        :param config: StarDetectionConfig object containing detection settings.
        """
        self.config = config or StarDetectionConfig()

        # Configure Loguru logger
        logger.remove()  # Remove default logger
        logger.add(
            "detection.log",
            rotation="10 MB",
            retention="10 days",
            level="DEBUG",
            format="{time:YYYY-MM-DD HH:mm:ss} | {level} | {message}"
        )
        logger.debug(
            "Initialized StarDetector with configuration: {}", self.config)

        # Initialize Preprocessor
        self.preprocessor = Preprocessor(
            config=PreprocessingConfig(
                median_filter_size=self.config.median_filter_size,
                wavelet_levels=self.config.wavelet_levels,
                binarization_threshold=self.config.binarization_threshold,
                save_preprocessed=self.config.save_detected_stars,
                preprocessed_save_path=self.config.detected_stars_save_path
            )
        )

    def multiscale_detect_stars(self, image: np.ndarray) -> List[Tuple[int, int]]:
        """
        Detect stars in an image using multiscale analysis and clustering.

        :param image: Grayscale input image as a numpy array.
        :return: List of detected star positions as (x, y) tuples.
        """
        logger.info("Starting multiscale star detection.")
        all_stars = []

        for scale in self.config.scales:
            logger.debug("Processing scale: {}", scale)
            resized_image = cv2.resize(
                image,
                None,
                fx=scale,
                fy=scale,
                interpolation=cv2.INTER_LINEAR
            )
            logger.debug("Image resized to scale {}.", scale)

            filtered_image = self.preprocessor.apply_median_filter(
                resized_image)
            logger.debug("Median filtering applied.")

            pyramid = self.preprocessor.wavelet_transform(filtered_image)
            background = self.preprocessor.extract_background(pyramid)
            subtracted_image = self.preprocessor.background_subtraction(
                filtered_image, background)
            logger.debug("Background subtraction completed.")

            processed_image = self.preprocessor.inverse_wavelet_transform(
                pyramid)
            logger.debug("Inverse wavelet transform completed.")

            binary_image = self.preprocessor.binarize(subtracted_image)
            logger.debug("Image binarization completed.")

            star_centers, star_properties = self.preprocessor.detect_stars(
                binary_image)
            logger.debug(
                "Star detection completed with {} stars detected.", len(star_centers))

            filtered_stars = self.filter_stars(star_properties, binary_image)
            logger.debug("{} stars passed the filtering criteria.",
                         len(filtered_stars))

            # Adjust star positions back to original scale
            scaled_stars = [(int(x / scale), int(y / scale))
                            for (x, y) in filtered_stars]
            all_stars.extend(scaled_stars)
            logger.debug("Star positions scaled back to original image size.")

        # Remove duplicate stars using DBSCAN clustering
        logger.info("Clustering detected stars to remove duplicates.")
        unique_stars = self.remove_duplicates(all_stars)
        logger.info(
            "Duplicate stars removed. {} unique stars detected.", len(unique_stars))

        if self.config.save_detected_stars and self.config.detected_stars_save_path:
            self.save_detected_stars(
                unique_stars, self.config.detected_stars_save_path)

        if self.config.visualize:
            self.visualize_stars(image, unique_stars,
                                 self.config.visualization_save_path)

        return unique_stars

    def filter_stars(
        self,
        star_properties: List[Tuple[Tuple[int, int], float, float]],
        binary_image: np.ndarray
    ) -> List[Tuple[int, int]]:
        """
        Filter detected stars based on shape, size, and brightness.

        :param star_properties: List of tuples containing star properties (center, area, perimeter).
        :param binary_image: Binary image used for star detection.
        :return: List of filtered star positions as (x, y) tuples.
        """
        logger.debug("Filtering stars based on defined criteria.")
        filtered_stars = []
        for (center, area, perimeter) in star_properties:
            if perimeter == 0:
                logger.debug(
                    "Skipping star at {} due to zero perimeter.", center)
                continue
            circularity = (4 * np.pi * area) / (perimeter ** 2)
            logger.debug("Star at {} has circularity {}.", center, circularity)

            mask = np.zeros_like(binary_image)
            cv2.circle(mask, center, 5, 255, -1)
            star_pixels = cv2.countNonZero(mask)
            brightness = np.mean(binary_image[mask == 255])

            logger.debug(
                "Star at {}: pixels={}, brightness={}.",
                center, star_pixels, brightness
            )

            if (
                star_pixels > self.config.min_star_size and
                brightness > self.config.min_star_brightness and
                self.config.min_circularity <= circularity <= self.config.max_circularity
            ):
                filtered_stars.append(center)
                logger.debug("Star at {} passed filtering.", center)

        logger.debug("Filtering completed. {} stars passed.",
                     len(filtered_stars))
        return filtered_stars

    def remove_duplicates(self, stars: List[Tuple[int, int]]) -> List[Tuple[int, int]]:
        """
        Remove duplicate stars using DBSCAN clustering.

        :param stars: List of star positions as (x, y) tuples.
        :return: List of unique star positions.
        """
        if not stars:
            logger.warning("No stars to cluster for duplicate removal.")
            return []

        try:
            star_array = np.array(stars)
            clustering = DBSCAN(eps=self.config.dbscan_eps,
                                min_samples=self.config.dbscan_min_samples)
            labels = clustering.fit_predict(star_array)
            logger.debug("DBSCAN clustering labels: {}", labels)

            unique_stars = []
            unique_labels = set(labels)
            for label in unique_labels:
                if label == -1:
                    continue  # Noise
                cluster = star_array[labels == label]
                centroid = tuple(cluster.mean(axis=0).astype(int))
                unique_stars.append(centroid)
                logger.debug("Cluster {}: Centroid at {}.", label, centroid)

            logger.info(
                "Duplicate removal completed. {} unique stars identified.", len(unique_stars))
            return unique_stars
        except Exception as e:
            logger.error("Error during duplicate removal: {}", e)
            raise

    def save_detected_stars(self, stars: List[Tuple[int, int]], save_path: Union[str, Path]) -> None:
        """
        Save the detected star positions to a JSON file.

        :param stars: List of star positions as (x, y) tuples.
        :param save_path: Path to save the JSON file.
        """
        logger.info("Saving detected stars to {}.", save_path)
        try:
            with open(save_path, 'w') as f:
                json.dump(stars, f, indent=4)
            logger.debug("Detected stars saved successfully.")
        except Exception as e:
            logger.error("Failed to save detected stars: {}", e)
            raise

    def load_detected_stars(self, file_path: Union[str, Path]) -> List[Tuple[int, int]]:
        """
        Load detected star positions from a JSON file.

        :param file_path: Path to the JSON file.
        :return: List of star positions as (x, y) tuples.
        """
        logger.info("Loading detected stars from {}.", file_path)
        try:
            with open(file_path, 'r') as f:
                stars = json.load(f)
            logger.debug(
                "Detected stars loaded successfully with {} entries.", len(stars))
            return stars
        except Exception as e:
            logger.error("Failed to load detected stars: {}", e)
            raise

    def visualize_stars(
        self,
        original_image: np.ndarray,
        stars: List[Tuple[int, int]],
        save_path: Optional[Path] = None
    ) -> None:
        """
        Visualize the detected stars on the original image.

        :param original_image: Original grayscale image as a numpy array.
        :param stars: List of detected star positions as (x, y) tuples.
        :param save_path: Optional path to save the visualization image.
        """
        logger.info("Visualizing detected stars.")
        try:
            color_image = cv2.cvtColor(original_image, cv2.COLOR_GRAY2BGR)
            for (x, y) in stars:
                cv2.circle(color_image, (x, y), 5, (0, 0, 255), 1)

            plt.figure(figsize=(10, 8))
            plt.imshow(cv2.cvtColor(color_image, cv2.COLOR_BGR2RGB))
            plt.title('Detected Stars')
            plt.axis('off')
            if save_path:
                plt.savefig(save_path)
                logger.debug("Star visualization saved to {}.", save_path)
            plt.show()
            logger.debug("Star visualization displayed successfully.")
        except Exception as e:
            logger.error("Error during star visualization: {}", e)
            raise


# Example Usage
if __name__ == "__main__":
    from preprocessing import PreprocessingConfig

    # Configure star detection parameters
    detection_config = StarDetectionConfig(
        median_filter_size=5,
        wavelet_levels=3,
        binarization_threshold=40,
        min_star_size=15,
        min_star_brightness=25,
        min_circularity=0.8,
        max_circularity=1.2,
        scales=[1.0, 0.8, 0.6],
        dbscan_eps=15.0,
        dbscan_min_samples=3,
        save_detected_stars=True,
        detected_stars_save_path=Path("detected_stars.json"),
        visualize=True,
        visualization_save_path=Path("detected_stars.png")
    )

    # Initialize StarDetector
    star_detector = StarDetector(config=detection_config)

    # Load and preprocess image
    image_path = "path/to/grayscale_image.png"
    try:
        preprocessor = star_detector.preprocessor
        preprocessed_image = preprocessor.load_image(image_path)
        logger.info("Image loaded and preprocessed.")

        # Perform star detection
        detected_stars = star_detector.multiscale_detect_stars(
            preprocessed_image)
        logger.info(
            "Star detection completed with {} stars detected.", len(detected_stars))

        # Optionally, load detected stars from file
        # detected_stars = star_detector.load_detected_stars("detected_stars.json")

    except Exception as e:
        logger.error("An error occurred during star detection: {}", e)
