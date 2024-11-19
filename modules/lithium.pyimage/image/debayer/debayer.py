import numpy as np
import cv2
from concurrent.futures import ThreadPoolExecutor, as_completed
from typing import Optional, Tuple, List, Union
from pathlib import Path
from dataclasses import dataclass
from loguru import logger


@dataclass
class DebayerConfig:
    """
    Configuration settings for the Debayer process.
    """
    method: str = 'bilinear'  # 'superpixel', 'bilinear', 'vng', 'ahd', 'laplacian'
    # 'BGGR', 'RGGB', 'GBRG', 'GRBG', or None for auto-detection
    pattern: Optional[str] = None
    num_threads: int = 4
    visualize_intermediate: bool = False
    visualization_save_path: Optional[Path] = None
    save_debayered_images: bool = False
    debayered_save_path_template: str = "{original_name}_{method}.png"


class Debayer:
    def __init__(self, config: Optional[DebayerConfig] = None) -> None:
        """
        Initialize Debayer object with configuration.

        :param config: DebayerConfig object containing debayer settings.
        """
        self.config = config or DebayerConfig()

        # Configure Loguru logger
        logger.remove()  # Remove default logger
        logger.add(
            "debayer.log",
            rotation="10 MB",
            retention="10 days",
            level="DEBUG",
            format="{time:YYYY-MM-DD HH:mm:ss} | {level} | {message}"
        )
        logger.debug("Initialized Debayer with configuration: {}", self.config)

    def detect_bayer_pattern(self, image: np.ndarray) -> str:
        """
        Automatically detect Bayer pattern from the CFA image.

        :param image: Grayscale CFA image.
        :return: Detected Bayer pattern.
        """
        logger.info("Starting Bayer pattern detection.")
        height, width = image.shape

        # Initialize pattern scores
        patterns = {'BGGR': 0, 'RGGB': 0, 'GBRG': 0, 'GRBG': 0}

        # Edge detection to enhance pattern recognition
        edges = cv2.Canny(image, 50, 150)

        # Analyze each 2x2 block
        for i in range(0, height - 1, 2):
            for j in range(0, width - 1, 2):
                block = image[i:i+2, j:j+2]
                edge_block = edges[i:i+2, j:j+2]

                # Calculate scores based on intensity and edges
                patterns['BGGR'] += block[0, 0] + block[1, 1] + \
                    edge_block[0, 0] + edge_block[1, 1]
                patterns['RGGB'] += block[1, 0] + block[0, 1] + \
                    edge_block[1, 0] + edge_block[0, 1]
                patterns['GBRG'] += block[0, 1] + block[1, 0] + \
                    edge_block[0, 1] + edge_block[1, 0]
                patterns['GRBG'] += block[0, 0] + block[1, 1] + \
                    edge_block[0, 0] + edge_block[1, 1]

        detected_pattern = max(patterns, key=patterns.get)
        logger.info("Detected Bayer pattern: {}", detected_pattern)
        return detected_pattern

    def debayer_image(self, cfa_image: np.ndarray) -> np.ndarray:
        """
        Perform the debayering process using the specified method.

        :param cfa_image: Grayscale CFA image.
        :return: Debayered RGB image.
        """
        logger.info("Starting debayering process.")
        if self.config.pattern is None:
            self.config.pattern = self.detect_bayer_pattern(cfa_image)

        # Extend image edges to handle boundary conditions
        cfa_image_padded = self.extend_image_edges(cfa_image, pad_width=2)
        logger.debug("Image edges extended with padding.")

        logger.debug("Using Bayer pattern: {}", self.config.pattern)

        method_dispatcher = {
            'superpixel': self.debayer_superpixel,
            'bilinear': self.debayer_bilinear,
            'vng': self.debayer_vng,
            'ahd': self.parallel_debayer_ahd,
            'laplacian': self.debayer_laplacian_harmonization
        }

        debayer_method = self.config.method.lower()
        if debayer_method not in method_dispatcher:
            logger.error("Unknown debayer method: {}", self.config.method)
            raise ValueError(f"Unknown debayer method: {self.config.method}")

        rgb_image = method_dispatcher[debayer_method](cfa_image_padded)
        logger.info("Debayering completed using method: {}",
                    self.config.method)

        if self.config.save_debayered_images:
            output_path = self.generate_save_path("debayered_image.png")
            cv2.imwrite(str(output_path), rgb_image)
            logger.debug("Debayered image saved to {}", output_path)

        return rgb_image

    def debayer_superpixel(self, cfa_image: np.ndarray) -> np.ndarray:
        """
        Debayering using superpixel method.

        :param cfa_image: Padded CFA image.
        :return: Debayered RGB image.
        """
        logger.info("Debayering using superpixel method.")
        red = cfa_image[0::2, 0::2]
        green = (cfa_image[0::2, 1::2] + cfa_image[1::2, 0::2]) / 2
        blue = cfa_image[1::2, 1::2]

        rgb_image = np.stack((red, green, blue), axis=-1)
        logger.debug("Superpixel debayering completed.")
        return rgb_image

    def debayer_bilinear(self, cfa_image: np.ndarray) -> np.ndarray:
        """
        Debayering using bilinear interpolation.

        :param cfa_image: Padded CFA image.
        :return: Debayered RGB image.
        """
        logger.info("Debayering using bilinear interpolation.")
        pattern = self.config.pattern.upper()
        pattern_codes = {
            'BGGR': cv2.COLOR_BayerBG2BGR,
            'RGGB': cv2.COLOR_BayerRG2BGR,
            'GBRG': cv2.COLOR_BayerGB2BGR,
            'GRBG': cv2.COLOR_BayerGR2BGR
        }

        if pattern not in pattern_codes:
            logger.error("Unsupported Bayer pattern: {}", pattern)
            raise ValueError(f"Unsupported Bayer pattern: {pattern}")

        rgb_image = cv2.cvtColor(cfa_image, pattern_codes[pattern])
        logger.debug("Bilinear debayering completed.")
        return rgb_image

    def debayer_vng(self, cfa_image: np.ndarray) -> np.ndarray:
        """
        Debayering using VNG interpolation.

        :param cfa_image: Padded CFA image.
        :return: Debayered RGB image.
        """
        logger.info("Debayering using VNG interpolation.")
        pattern = self.config.pattern.upper()
        pattern_codes = {
            'BGGR': cv2.COLOR_BayerBG2BGR_VNG,
            'RGGB': cv2.COLOR_BayerRG2BGR_VNG,
            'GBRG': cv2.COLOR_BayerGB2BGR_VNG,
            'GRBG': cv2.COLOR_BayerGR2BGR_VNG
        }

        if pattern not in pattern_codes:
            logger.error("Unsupported Bayer pattern for VNG: {}", pattern)
            raise ValueError(f"Unsupported Bayer pattern for VNG: {pattern}")

        rgb_image = cv2.cvtColor(cfa_image, pattern_codes[pattern])
        logger.debug("VNG debayering completed.")
        return rgb_image

    def parallel_debayer_ahd(self, cfa_image: np.ndarray) -> np.ndarray:
        """
        Debayering using Adaptive Homogeneity-Directed (AHD) interpolation with multithreading.

        :param cfa_image: Padded CFA image.
        :return: Debayered RGB image.
        """
        logger.info(
            "Debayering using Adaptive Homogeneity-Directed (AHD) interpolation.")
        height, width = cfa_image.shape
        chunk_size = height // self.config.num_threads
        results: List[np.ndarray] = [None] * self.config.num_threads

        def process_chunk(start_row: int, end_row: int, index: int):
            logger.debug("Processing chunk {}: rows {} to {}",
                         index, start_row, end_row)
            chunk = cfa_image[start_row:end_row, :]
            gradient_x, gradient_y = self.calculate_gradients(chunk)
            green_channel = self.interpolate_green_channel(
                chunk, gradient_x, gradient_y)
            red_channel, blue_channel = self.interpolate_red_blue_channel(
                chunk, green_channel, self.config.pattern)
            rgb_chunk = np.stack(
                (red_channel, green_channel, blue_channel), axis=-1)
            results[index] = np.clip(rgb_chunk, 0, 255).astype(np.uint8)
            logger.debug("Chunk {} processing completed.", index)

        with ThreadPoolExecutor(max_workers=self.config.num_threads) as executor:
            futures = []
            for i in range(self.config.num_threads):
                start_row = i * chunk_size
                end_row = (
                    i + 1) * chunk_size if i < self.config.num_threads - 1 else height
                futures.append(executor.submit(
                    process_chunk, start_row, end_row, i))

            for future in as_completed(futures):
                future.result()

        rgb_image = np.vstack(results)
        logger.debug("AHD debayering completed with multithreading.")
        return rgb_image

    def debayer_laplacian_harmonization(self, cfa_image: np.ndarray) -> np.ndarray:
        """
        Debayering using Laplacian harmonization to enhance edges.

        :param cfa_image: Padded CFA image.
        :return: Debayered RGB image with harmonized edges.
        """
        logger.info("Debayering using Laplacian harmonization.")
        interpolated_image = self.debayer_bilinear(cfa_image)

        # Calculate Laplacian for each channel
        laplacian = {}
        for idx, color in enumerate(['Blue', 'Green', 'Red']):
            lap = self.calculate_laplacian(interpolated_image[:, :, idx])
            laplacian[color] = lap
            logger.debug("Laplacian calculated for {} channel.", color)

        # Harmonize each channel
        harmonized_channels = []
        for idx, color in enumerate(['Blue', 'Green', 'Red']):
            harmonized = self.harmonize_edges(
                interpolated_image[:, :, idx], laplacian[color])
            harmonized_channels.append(harmonized)
            logger.debug("{} channel harmonized.", color)

        harmonized_image = np.stack(harmonized_channels, axis=-1)
        logger.debug("Laplacian harmonization completed.")
        return harmonized_image

    @staticmethod
    def calculate_gradients(image: np.ndarray) -> Tuple[np.ndarray, np.ndarray]:
        """
        Calculate the gradients of the CFA image.

        :param image: CFA image chunk.
        :return: Tuple of gradient_x and gradient_y.
        """
        gradient_x = np.abs(np.diff(image, axis=1))
        gradient_y = np.abs(np.diff(image, axis=0))

        gradient_x = np.pad(gradient_x, ((0, 0), (0, 1)), 'constant')
        gradient_y = np.pad(gradient_y, ((0, 1), (0, 0)), 'constant')

        return gradient_x, gradient_y

    @staticmethod
    def interpolate_green_channel(cfa_image: np.ndarray, gradient_x: np.ndarray, gradient_y: np.ndarray) -> np.ndarray:
        """
        Interpolate the green channel of the CFA image based on gradients.

        :param cfa_image: CFA image chunk.
        :param gradient_x: Gradient in x-direction.
        :param gradient_y: Gradient in y-direction.
        :return: Interpolated green channel.
        """
        logger.debug("Interpolating green channel.")
        height, width = cfa_image.shape
        green_channel = np.zeros((height, width), dtype=np.float64)

        for i in range(1, height - 1):
            for j in range(1, width - 1):
                if (i % 2 == 0 and j % 2 == 1) or (i % 2 == 1 and j % 2 == 0):
                    # Green pixel
                    green_channel[i, j] = cfa_image[i, j]
                else:
                    # Interpolate green
                    if gradient_x[i, j] < gradient_y[i, j]:
                        green_channel[i, j] = 0.5 * \
                            (cfa_image[i, j-1] + cfa_image[i, j+1])
                    else:
                        green_channel[i, j] = 0.5 * \
                            (cfa_image[i-1, j] + cfa_image[i+1, j])

        return green_channel

    @staticmethod
    def interpolate_red_blue_channel(cfa_image: np.ndarray, green_channel: np.ndarray, pattern: Optional[str] = None) -> Tuple[np.ndarray, np.ndarray]:
        """
        Interpolate the red and blue channels of the CFA image based on the green channel.

        :param cfa_image: CFA image chunk.
        :param green_channel: Interpolated green channel.
        :param pattern: Bayer pattern.
        :return: Tuple of interpolated red and blue channels.
        """
        logger.debug("Interpolating red and blue channels.")
        height, width = cfa_image.shape
        red_channel = np.zeros((height, width), dtype=np.float64)
        blue_channel = np.zeros((height, width), dtype=np.float64)

        pattern = pattern.upper() if pattern else 'BGGR'

        for i in range(0, height - 1, 2):
            for j in range(0, width - 1, 2):
                if pattern == 'BGGR':
                    blue_channel[i, j] = cfa_image[i, j]
                    red_channel[i+1, j+1] = cfa_image[i+1, j+1]

                    green_r = 0.5 * \
                        (green_channel[i+1, j] + green_channel[i, j+1])
                    green_b = 0.5 * \
                        (green_channel[i, j] + green_channel[i+1, j+1])

                    blue_channel[i+1, j] = cfa_image[i+1, j] - \
                        green_b + green_channel[i+1, j]
                    blue_channel[i, j+1] = cfa_image[i, j+1] - \
                        green_b + green_channel[i, j+1]
                    red_channel[i, j] = cfa_image[i, j] - \
                        green_r + green_channel[i, j]
                    red_channel[i+1, j+1] = cfa_image[i+1, j+1] - \
                        green_r + green_channel[i+1, j+1]

                elif pattern == 'RGGB':
                    red_channel[i, j] = cfa_image[i, j]
                    blue_channel[i+1, j+1] = cfa_image[i+1, j+1]

                    green_r = 0.5 * \
                        (green_channel[i, j+1] + green_channel[i+1, j])
                    green_b = 0.5 * \
                        (green_channel[i+1, j] + green_channel[i, j+1])

                    red_channel[i+1, j] = cfa_image[i+1, j] - \
                        green_r + green_channel[i+1, j]
                    red_channel[i, j+1] = cfa_image[i, j+1] - \
                        green_r + green_channel[i, j+1]
                    blue_channel[i, j] = cfa_image[i, j] - \
                        green_b + green_channel[i, j]
                    blue_channel[i+1, j+1] = cfa_image[i+1, j+1] - \
                        green_b + green_channel[i+1, j+1]

                elif pattern == 'GBRG':
                    green_channel[i, j+1] = cfa_image[i, j+1]
                    blue_channel[i+1, j] = cfa_image[i+1, j]

                    green_r = 0.5 * \
                        (green_channel[i, j] + green_channel[i+1, j+1])
                    green_b = 0.5 * \
                        (green_channel[i+1, j] + green_channel[i, j+1])

                    red_channel[i, j] = cfa_image[i, j] - \
                        green_r + green_channel[i, j]
                    red_channel[i+1, j+1] = cfa_image[i+1, j+1] - \
                        green_r + green_channel[i+1, j+1]
                    blue_channel[i, j] = cfa_image[i, j] - \
                        green_b + green_channel[i, j]
                    blue_channel[i+1, j+1] = cfa_image[i+1, j+1] - \
                        green_b + green_channel[i+1, j+1]

                elif pattern == 'GRBG':
                    green_channel[i, j] = cfa_image[i, j]
                    red_channel[i+1, j] = cfa_image[i+1, j]

                    green_r = 0.5 * \
                        (green_channel[i, j] + green_channel[i+1, j+1])
                    green_b = 0.5 * \
                        (green_channel[i+1, j] + green_channel[i, j+1])

                    red_channel[i, j] = cfa_image[i, j] - \
                        green_r + green_channel[i, j]
                    red_channel[i+1, j+1] = cfa_image[i+1, j+1] - \
                        green_r + green_channel[i+1, j+1]
                    blue_channel[i+1, j] = cfa_image[i+1, j] - \
                        green_b + green_channel[i+1, j]
                    blue_channel[i, j+1] = cfa_image[i, j+1] - \
                        green_b + green_channel[i, j+1]

        return red_channel, blue_channel

    def calculate_laplacian(image: np.ndarray) -> np.ndarray:
        """
        Calculate the Laplacian of the image for edge enhancement.

        :param image: Single-channel image.
        :return: Laplacian image.
        """
        laplacian = cv2.Laplacian(image, cv2.CV_64F)
        return laplacian

    def harmonize_edges(original: np.ndarray, interpolated: np.ndarray, laplacian: np.ndarray) -> np.ndarray:
        """
        Harmonize edges using the Laplacian result.

        :param original: Original CFA image.
        :param interpolated: Interpolated channel image.
        :param laplacian: Laplacian image.
        :return: Harmonized channel image.
        """
        harmonized = np.clip(interpolated + 0.2 *
                             laplacian, 0, 255).astype(np.uint8)
        return harmonized

    def extend_image_edges(image: np.ndarray, pad_width: int) -> np.ndarray:
        """
        Extend image edges using mirror padding to handle boundary issues during interpolation.

        :param image: Input image.
        :param pad_width: Width of padding.
        :return: Padded image.
        """
        return np.pad(image, pad_width, mode='reflect')

    def visualize_intermediate_steps(image_path: Union[str, Path], debayered_image: np.ndarray, config: DebayerConfig):
        """
        Visualize intermediate steps in the debayering process.

        :param image_path: Path to the original CFA image.
        :param debayered_image: Debayered RGB image.
        :param config: DebayerConfig object.
        """
        import matplotlib.pyplot as plt

        if config.visualize_intermediate and config.visualization_save_path:
            logger.info("Visualizing intermediate steps.")
            # Load original image
            cfa_image = cv2.imread(str(image_path), cv2.IMREAD_GRAYSCALE)

            # Detect pattern if not set
            pattern = config.pattern or Debayer().detect_bayer_pattern(cfa_image)

            # Debayer using bilinear for visualization
            debayer_bilinear = Debayer().debayer_bilinear(cfa_image, pattern)

            # Calculate gradients
            gradient_x, gradient_y = Debayer.calculate_gradients(cfa_image)
            green_channel = Debayer.interpolate_green_channel(
                cfa_image, gradient_x, gradient_y)
            red_channel, blue_channel = Debayer.interpolate_red_blue_channel(
                cfa_image, green_channel, pattern)

            # Display images
            plt.figure(figsize=(15, 10))

            plt.subplot(2, 3, 1)
            plt.imshow(cfa_image, cmap='gray')
            plt.title('Original CFA Image')
            plt.axis('off')

            plt.subplot(2, 3, 2)
            plt.imshow(gradient_x, cmap='gray')
            plt.title('Gradient X')
            plt.axis('off')

            plt.subplot(2, 3, 3)
            plt.imshow(gradient_y, cmap='gray')
            plt.title('Gradient Y')
            plt.axis('off')

            plt.subplot(2, 3, 4)
            plt.imshow(green_channel, cmap='gray')
            plt.title('Interpolated Green Channel')
            plt.axis('off')

            plt.subplot(2, 3, 5)
            plt.imshow(red_channel, cmap='gray')
            plt.title('Interpolated Red Channel')
            plt.axis('off')

            plt.subplot(2, 3, 6)
            plt.imshow(blue_channel, cmap='gray')
            plt.title('Interpolated Blue Channel')
            plt.axis('off')

            plt.tight_layout()
            if config.visualization_save_path:
                plt.savefig(config.visualization_save_path)
                logger.debug("Intermediate visualization saved to {}.",
                             config.visualization_save_path)
            plt.show()
            logger.info("Intermediate visualization displayed successfully.")

    def generate_save_path(self, original_path: str) -> Path:
        """
        Generate a save path for the debayered image based on the original image name and method.

        :param original_path: Original image file path.
        :return: Path object for the debayered image.
        """
        original_name = Path(original_path).stem
        save_path = Path(self.config.debayered_save_path_template.format(
            original_name=original_name, method=self.config.method))
        return save_path

    # Example usage in __main__
    if __name__ == "__main__":
        # Example usage
        config = DebayerConfig(
            method='bilinear',
            pattern=None,  # Auto-detect
            num_threads=4,
            visualize_intermediate=True,
            visualization_save_path=Path("intermediate_steps.png"),
            save_debayered_images=True,
            debayered_save_path_template="{original_name}_{method}.png"
        )
        debayer = Debayer(config=config)
        image_path = "path/to/cfa_image.png"

        try:
            debayered_rgb = debayer.debayer_image(
                cv2.imread(image_path, cv2.IMREAD_GRAYSCALE))
            visualize_intermediate_steps(image_path, debayered_rgb, config)
            logger.info("Debayering process completed successfully.")
        except Exception as e:
            logger.error("An error occurred during debayering: {}", e)
