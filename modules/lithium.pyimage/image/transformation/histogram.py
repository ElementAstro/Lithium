import cv2
import numpy as np
import matplotlib.pyplot as plt
from pathlib import Path
from typing import Optional, Tuple, List
import argparse
from loguru import logger

# Configure Loguru logger
logger.remove()  # Remove default logger
logger.add("histogram.log", rotation="10 MB", retention="10 days",
           level="DEBUG", format="{time:YYYY-MM-DD HH:mm:ss} | {level} | {message}")


def calculate_histogram(image: np.ndarray, channel: int = 0) -> np.ndarray:
    """
    Calculate the histogram of an image channel.

    :param image: Input image as a numpy array.
    :param channel: Channel index (0 for Blue, 1 for Green, 2 for Red in BGR images).
    :return: Histogram as a numpy array.
    """
    logger.debug(f"Calculating histogram for channel {channel}")
    histogram = cv2.calcHist([image], [channel], None, [256], [0, 256])
    logger.debug(f"Histogram calculated: {histogram.flatten()}")
    return histogram


def display_histogram(histogram: np.ndarray, title: str = "Histogram") -> None:
    """
    Display the histogram using Matplotlib.

    :param histogram: Histogram data as a numpy array.
    :param title: Title of the histogram plot.
    """
    logger.debug(f"Displaying histogram with title: {title}")
    plt.figure()
    plt.plot(histogram, color='black')
    plt.title(title)
    plt.xlabel('Pixel Intensity')
    plt.ylabel('Frequency')
    plt.grid(True)
    plt.show()
    logger.debug("Histogram displayed successfully")


def apply_histogram_transformation(image: np.ndarray,
                                   shadows_clip: float = 0.0,
                                   highlights_clip: float = 1.0,
                                   midtones_balance: float = 0.5,
                                   lower_bound: float = -1.0,
                                   upper_bound: float = 2.0) -> np.ndarray:
    """
    Apply histogram-based transformation to an image.

    :param image: Input grayscale image as a numpy array.
    :param shadows_clip: Shadow clipping threshold (0.0 to 1.0).
    :param highlights_clip: Highlight clipping threshold (0.0 to 1.0).
    :param midtones_balance: Balance factor for midtones.
    :param lower_bound: Lower bound for dynamic range expansion.
    :param upper_bound: Upper bound for dynamic range expansion.
    :return: Transformed image as a numpy array.
    """
    logger.info("Applying histogram transformation")
    try:
        # Normalize the image to [0, 1]
        normalized_image = image.astype(np.float32) / 255.0
        logger.debug("Image normalized to [0, 1]")

        # Shadows and highlights clipping
        clipped_image = np.clip(
            (normalized_image - shadows_clip) / (highlights_clip - shadows_clip), 0, 1)
        logger.debug(
            f"Clipped image with shadows_clip={shadows_clip}, highlights_clip={highlights_clip}")

        # Midtones balance using a custom transfer function
        def mtf(x): return (x ** midtones_balance) / \
            ((x ** midtones_balance + (1 - x) ** midtones_balance)
             ** (1 / midtones_balance))
        transformed_image = mtf(clipped_image)
        logger.debug(
            f"Midtones balanced with midtones_balance={midtones_balance}")

        # Dynamic range expansion
        expanded_image = np.clip(
            (transformed_image - lower_bound) / (upper_bound - lower_bound), 0, 1)
        logger.debug(
            f"Dynamic range expanded with lower_bound={lower_bound}, upper_bound={upper_bound}")

        # Rescale to [0, 255]
        output_image = (expanded_image * 255).astype(np.uint8)
        logger.debug("Image rescaled to [0, 255]")
        logger.info("Histogram transformation applied successfully")
        return output_image
    except Exception as e:
        logger.error(f"Error in histogram transformation: {e}")
        raise


def auto_clip(image: np.ndarray, clip_percent: float = 0.01) -> np.ndarray:
    """
    Automatically clip the histogram based on a percentage.

    :param image: Input grayscale image as a numpy array.
    :param clip_percent: Percentage for clipping the histogram tails.
    :return: Auto-clipped image as a numpy array.
    """
    logger.info(f"Applying auto clipping with clip_percent={clip_percent}")
    try:
        # Compute the histogram
        hist, bins = np.histogram(image.flatten(), 256, [0, 256])
        cdf = hist.cumsum()
        total_pixels = image.size
        logger.debug("Computed cumulative distribution function (CDF)")

        # Calculate clipping points
        lower_clip = np.searchsorted(cdf, total_pixels * clip_percent)
        upper_clip = np.searchsorted(cdf, total_pixels * (1 - clip_percent))
        logger.debug(f"Lower clip at {lower_clip}, Upper clip at {upper_clip}")

        # Apply histogram transformation
        auto_clipped_image = apply_histogram_transformation(
            image,
            shadows_clip=lower_clip / 255.0,
            highlights_clip=upper_clip / 255.0
        )
        logger.info("Auto clipping applied successfully")
        return auto_clipped_image
    except Exception as e:
        logger.error(f"Error in auto clipping: {e}")
        raise


def display_rgb_histogram(image: np.ndarray) -> None:
    """
    Display RGB histograms for an image.

    :param image: Input image as a numpy array (BGR format).
    """
    logger.debug("Displaying RGB histograms")
    colors = ('b', 'g', 'r')
    plt.figure()
    for i, col in enumerate(colors):
        hist = calculate_histogram(image, channel=i)
        plt.plot(hist, color=col)
        plt.xlim([0, 256])
    plt.title('RGB Histogram')
    plt.xlabel('Pixel Intensity')
    plt.ylabel('Frequency')
    plt.grid(True)
    plt.show()
    logger.debug("RGB histograms displayed successfully")


def real_time_preview(image: np.ndarray,
                      transformation_function,
                      window_name: str = "Real-Time Preview",
                      **kwargs) -> None:
    """
    Display real-time preview of image transformations.

    :param image: Input image as a numpy array.
    :param transformation_function: Function to apply transformation.
    :param window_name: Name of the display window.
    :param kwargs: Additional keyword arguments for the transformation function.
    """
    logger.info("Starting real-time preview")
    try:
        preview_image = transformation_function(image, **kwargs)
        cv2.imshow(window_name, preview_image)
        logger.debug("Transformation applied for real-time preview")
    except Exception as e:
        logger.error(f"Error in real-time preview: {e}")
        raise


def save_histogram(histogram: np.ndarray, filepath: Path, title: str = "Histogram") -> None:
    """
    Save the histogram plot to a file.

    :param histogram: Histogram data as a numpy array.
    :param filepath: Path to save the histogram image.
    :param title: Title of the histogram plot.
    """
    logger.debug(f"Saving histogram to {filepath}")
    try:
        plt.figure()
        plt.plot(histogram, color='black')
        plt.title(title)
        plt.xlabel('Pixel Intensity')
        plt.ylabel('Frequency')
        plt.grid(True)
        plt.savefig(filepath)
        plt.close()
        logger.info(f"Histogram saved to {filepath}")
    except Exception as e:
        logger.error(f"Failed to save histogram: {e}")
        raise


def parse_arguments() -> argparse.Namespace:
    """
    Parse command-line arguments for the histogram tool.

    :return: Parsed arguments namespace.
    """
    parser = argparse.ArgumentParser(
        description="Image Histogram Processing Tool")
    parser.add_argument('--input', type=Path, required=True,
                        help='Path to the input image.')
    parser.add_argument('--output', type=Path, required=True,
                        help='Path to save the processed image.')
    parser.add_argument('--save_histogram', type=Path, default=None,
                        help='Path to save the histogram plot.')
    parser.add_argument('--operation', type=str, choices=['mean', 'gaussian', 'minimum',
                                                          'maximum', 'median', 'bilinear', 'bicubic'],
                        default='mean', help='Histogram transformation operation.')
    parser.add_argument('--structure', type=str, choices=['square', 'circular', 'horizontal', 'vertical'],
                        default='square', help='Neighborhood structure for transformation.')
    parser.add_argument('--radius', type=int, default=1,
                        help='Radius for neighborhood structure.')
    parser.add_argument('--clip_percent', type=float, default=0.01,
                        help='Clip percentage for auto clipping.')
    parser.add_argument('--midtones_balance', type=float, default=0.5,
                        help='Balance factor for midtones.')
    parser.add_argument('--lower_bound', type=float, default=-1.0,
                        help='Lower bound for dynamic range expansion.')
    parser.add_argument('--upper_bound', type=float, default=2.0,
                        help='Upper bound for dynamic range expansion.')
    parser.add_argument('--real_time_preview', action='store_true',
                        help='Enable real-time preview of transformations.')

    return parser.parse_args()


def main():
    """
    Main function to execute histogram processing.
    """
    args = parse_arguments()

    # Load image
    logger.info(f"Loading image from {args.input}")
    image = cv2.imread(str(args.input))
    if image is None:
        logger.error(f"Failed to load image: {args.input}")
        sys.exit(1)
    logger.info(f"Image loaded successfully with shape {image.shape}")

    # Convert to grayscale
    grayscale_image = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    logger.debug("Converted image to grayscale")

    # Display original image and histogram
    cv2.imshow('Original Image', image)
    original_histogram = calculate_histogram(grayscale_image)
    display_histogram(original_histogram, title="Original Grayscale Histogram")

    # Display RGB histogram
    display_rgb_histogram(image)

    # Apply histogram transformation
    logger.info("Applying histogram transformation")
    transformed_image = apply_histogram_transformation(
        grayscale_image,
        shadows_clip=0.1,
        highlights_clip=0.9,
        midtones_balance=args.midtones_balance,
        lower_bound=args.lower_bound,
        upper_bound=args.upper_bound
    )
    cv2.imshow('Transformed Image', transformed_image)
    logger.info("Histogram transformation applied")

    # Display transformed histogram
    transformed_histogram = calculate_histogram(transformed_image)
    display_histogram(transformed_histogram,
                      title="Transformed Grayscale Histogram")

    # Apply auto clipping
    logger.info("Applying auto clipping")
    auto_clipped_image = auto_clip(
        grayscale_image, clip_percent=args.clip_percent)
    cv2.imshow('Auto Clipped Image', auto_clipped_image)
    logger.info("Auto clipping applied")

    # Save histogram if required
    if args.save_histogram:
        logger.info(f"Saving histogram to {args.save_histogram}")
        save_histogram(auto_clipped_image if args.clip_percent else transformed_image,
                       args.save_histogram,
                       title="Auto Clipped Grayscale Histogram")

    # Real-time preview
    if args.real_time_preview:
        logger.info("Enabling real-time preview")
        real_time_preview(
            image=grayscale_image,
            transformation_function=apply_histogram_transformation,
            shadows_clip=0.05,
            highlights_clip=0.95,
            midtones_balance=args.midtones_balance,
            lower_bound=args.lower_bound,
            upper_bound=args.upper_bound
        )

    logger.info("Displaying all processed images. Press any key to exit.")
    cv2.waitKey(0)
    cv2.destroyAllWindows()
    logger.info("All windows closed. Program terminated successfully.")


if __name__ == "__main__":
    main()
