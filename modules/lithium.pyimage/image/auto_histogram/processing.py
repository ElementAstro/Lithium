import os
from pathlib import Path
from typing import List, Optional, Union
from loguru import logger
from .histogram import HistogramProcessor, HistogramConfig
from .utils import save_image, load_image


def process_directory(
    input_directory: Union[str, Path],
    output_directory: Union[str, Path],
    method: str = 'gamma',
    clip_shadow: float = 0.01,
    clip_highlight: float = 0.01,
    target_median: int = 128,
    apply_clahe: bool = False,
    clahe_clip_limit: float = 2.0,
    clahe_tile_grid_size: Tuple[int, int] = (8, 8),
    apply_noise_reduction: bool = False,
    noise_reduction_method: str = 'median',
    apply_sharpening: bool = False,
    sharpening_strength: float = 1.0,
    recursive: bool = False
) -> None:
    """
    Process all images in a directory using the HistogramProcessor.

    :param input_directory: Directory containing images to process.
    :param output_directory: Directory to save processed images.
    :param method: Histogram stretching method ('gamma', 'logarithmic', 'mtf').
    :param clip_shadow: Percentage of shadow pixels to clip.
    :param clip_highlight: Percentage of highlight pixels to clip.
    :param target_median: Target median value for histogram stretching.
    :param apply_clahe: Apply CLAHE (Contrast Limited Adaptive Histogram Equalization).
    :param clahe_clip_limit: CLAHE clip limit.
    :param clahe_tile_grid_size: CLAHE grid size.
    :param apply_noise_reduction: Apply noise reduction.
    :param noise_reduction_method: Noise reduction method ('median', 'gaussian').
    :param apply_sharpening: Apply image sharpening.
    :param sharpening_strength: Strength of sharpening.
    :param recursive: Process directories recursively.
    """
    try:
        input_directory = Path(input_directory)
        output_directory = Path(output_directory)
        if not input_directory.exists():
            logger.error(f"Input directory does not exist: {input_directory}")
            raise FileNotFoundError(
                f"Input directory does not exist: {input_directory}")

        output_directory.mkdir(parents=True, exist_ok=True)
        logger.info(
            f"Processing images from {input_directory} to {output_directory}")

        # Gather image files
        if recursive:
            file_list = [p for p in input_directory.rglob(
                '*') if p.suffix.lower() in ['.jpg', '.jpeg', '.png', '.tif', '.tiff']]
        else:
            file_list = [p for p in input_directory.glob(
                '*') if p.suffix.lower() in ['.jpg', '.jpeg', '.png', '.tif', '.tiff']]

        logger.info(f"Found {len(file_list)} images to process.")

        if not file_list:
            logger.warning("No images found to process.")
            return

        # Configure HistogramProcessor
        config = HistogramConfig(
            clip_shadow=clip_shadow,
            clip_highlight=clip_highlight,
            target_median=target_median,
            method=method,
            apply_clahe=apply_clahe,
            clahe_clip_limit=clahe_clip_limit,
            clahe_tile_grid_size=clahe_tile_grid_size,
            apply_noise_reduction=apply_noise_reduction,
            noise_reduction_method=noise_reduction_method,
            apply_sharpening=apply_sharpening,
            sharpening_strength=sharpening_strength,
            batch_process=True,
            file_list=file_list,
            output_directory=output_directory
        )

        processor = HistogramProcessor(config=config)
        processor.process()

        logger.info("Directory processing completed successfully.")
    except Exception as e:
        logger.error(f"Error during directory processing: {e}")
        raise
