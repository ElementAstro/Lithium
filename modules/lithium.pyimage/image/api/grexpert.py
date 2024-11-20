import argparse
import subprocess
import sys
from pathlib import Path

from loguru import logger


class SerializationException(Exception):
    """Custom exception for serialization errors."""
    pass


def setup_logging(log_level: str):
    """
    Configure the Loguru logger.

    Args:
        log_level (str): The logging level (e.g., "INFO", "DEBUG", "ERROR").
    """
    logger.remove()  # Remove the default logger
    logger.add(sys.stderr, level=log_level,
               format="{time} | {level} | {message}")


def validate_args(args: argparse.Namespace):
    """
    Validate command-line arguments to ensure correctness.

    Args:
        args (argparse.Namespace): Parsed command-line arguments.

    Raises:
        SerializationException: If any validation check fails.
    """
    # Check if input file(s) exist
    for file in args.filenames:
        if not file.is_file():
            logger.error(f"Input file '{file}' does not exist.")
            raise SerializationException(
                f"Input file '{file}' does not exist.")

    # Check operation type
    if args.operation not in ["background-extraction", "denoising"]:
        logger.error(
            f"Unsupported operation '{args.operation}'. Use 'background-extraction' or 'denoising'.")
        raise SerializationException(
            f"Unsupported operation '{args.operation}'.")

    # Check parameter ranges
    if args.smoothing is not None and not (0.0 <= args.smoothing <= 1.0):
        logger.error(
            f"Smoothing value must be between 0.0 and 1.0. Received: {args.smoothing}")
        raise SerializationException(
            f"Smoothing value out of range: {args.smoothing}")

    if args.strength is not None and not (0.0 <= args.strength <= 1.0):
        logger.error(
            f"Strength value must be between 0.0 and 1.0. Received: {args.strength}")
        raise SerializationException(
            f"Strength value out of range: {args.strength}")

    if args.batch_size is not None and not (1 <= args.batch_size <= 32):
        logger.error(
            f"Batch size must be between 1 and 32. Received: {args.batch_size}")
        raise SerializationException(
            f"Batch size out of range: {args.batch_size}")

    # Check background correction method validity
    if args.correction and args.correction not in ["Subtraction", "Division"]:
        logger.error(
            f"Correction method must be 'Subtraction' or 'Division'. Received: {args.correction}")
        raise SerializationException(
            f"Invalid correction method: {args.correction}")

    # For denoising, ensure strength parameter exists
    if args.operation == "denoising" and args.strength is None:
        logger.error(
            "The 'strength' parameter is required for the 'denoising' operation.")
        raise SerializationException(
            "Missing 'strength' parameter for 'denoising' operation.")


def run_graxpert(args: argparse.Namespace):
    """
    Run GraXpert with specified command-line arguments.

    Args:
        args (argparse.Namespace): Parsed command-line arguments.

    Raises:
        SerializationException: If GraXpert execution fails.
    """
    graxpert_executable = Path(args.graxpert_path).expanduser()

    if not graxpert_executable.is_file():
        logger.error(
            f"GraXpert executable not found at '{graxpert_executable}'.")
        raise SerializationException(
            f"GraXpert executable not found at '{graxpert_executable}'.")

    # Base command
    cmd = [str(graxpert_executable), "-cli"]

    # Add operation and filenames
    cmd += ["-cmd", args.operation]
    cmd += [str(file) for file in args.filenames]

    # Add optional parameters
    if args.output:
        cmd += ["-output", args.output]
    if args.gpu is not None:
        cmd += ["-gpu", "true" if args.gpu else "false"]
    if args.ai_version:
        cmd += ["-ai_version", args.ai_version]
    if args.preferences_file:
        cmd += ["-preferences_file", str(args.preferences_file)]
    if args.correction:
        cmd += ["-correction", args.correction]
    if args.smoothing is not None:
        cmd += ["-smoothing", str(args.smoothing)]
    if args.bg:
        cmd += ["-bg"]
    if args.strength is not None:
        cmd += ["-strength", str(args.strength)]
    if args.batch_size is not None:
        cmd += ["-batch_size", str(args.batch_size)]

    # Print the command being run
    logger.debug(f"Running command: {' '.join(cmd)}")

    try:
        subprocess.run(cmd, check=True)
        logger.info("GraXpert processing completed successfully.")
    except subprocess.CalledProcessError as e:
        logger.error(f"Error running GraXpert: {e}")
        raise SerializationException(f"GraXpert execution failed: {e}") from e


def parse_arguments() -> argparse.Namespace:
    """
    Parse command-line arguments.

    Returns:
        argparse.Namespace: Parsed arguments.
    """
    parser = argparse.ArgumentParser(
        description="GraXpert Command-Line Interface")

    parser.add_argument("filenames", type=Path, nargs='+',
                        help="Path(s) to the unprocessed image file(s)")

    parser.add_argument("-cmd", "--operation", choices=[
                        "background-extraction", "denoising"], default="background-extraction", help="Operation to perform")

    parser.add_argument(
        "-output", help="Name of the output file (without extension)")

    parser.add_argument("-gpu", type=lambda x: x.lower() == "true",
                        help="Enable ('true') or disable ('false') GPU acceleration")

    parser.add_argument(
        "-ai_version", help="Specify the AI model version to use")

    parser.add_argument("-preferences_file", type=Path,
                        help="Path to preferences file containing background grid points")

    parser.add_argument("-correction", choices=["Subtraction", "Division"],
                        help="Background correction method")

    parser.add_argument("-smoothing", type=float,
                        help="Strength of smoothing (0.0 to 1.0)")

    parser.add_argument("-bg", action="store_true",
                        help="Also save the generated background model")

    parser.add_argument("-strength", type=float,
                        help="Strength of denoising (0.0 to 1.0)")

    parser.add_argument("-batch_size", type=int,
                        help="Number of image tiles to process in parallel (1 to 32)")

    parser.add_argument("-log_level", type=str, default="INFO",
                        choices=["DEBUG", "INFO",
                                 "WARNING", "ERROR", "CRITICAL"],
                        help="Set the logging level")

    parser.add_argument("-graxpert_path", type=Path, default="GraXpert",
                        help="Path to the GraXpert executable")

    args = parser.parse_args()
    return args


def main():
    """
    Main function to execute the script logic.
    """
    try:
        args = parse_arguments()
        setup_logging(args.log_level)
        logger.debug("Parsed arguments successfully.")
        validate_args(args)
        logger.info("Arguments validated successfully.")
        run_graxpert(args)
    except SerializationException as e:
        logger.critical(f"Serialization Exception: {e}")
        sys.exit(1)
    except Exception as e:
        logger.critical(f"Unexpected error: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
