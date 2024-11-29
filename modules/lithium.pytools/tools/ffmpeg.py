# ffmpeg.py

from dataclasses import dataclass
import ffmpeg
import asyncio
from concurrent.futures import ThreadPoolExecutor
import argparse
from loguru import logger
from rich.console import Console
from rich.table import Table
from typing import List, Optional, Dict, Any, Literal
import sys


# Configure Loguru logger with Rich handler
logger.remove()  # Remove the default logger
logger.add(
    sys.stderr,
    format="<green>{time:YYYY-MM-DD HH:mm:ss}</green> | <level>{level: <8}</level> | <cyan>{name}</cyan>:<cyan>{function}</cyan>:<cyan>{line}</cyan> - <level>{message}</level>",
    level="DEBUG"
)
logger.add(
    "ffmpeg_wrapper.log",
    rotation="5 MB",
    retention="7 days",
    level="DEBUG",
    format="{time:YYYY-MM-DD at HH:mm:ss} | {level} | {message}"
)

console = Console()


@dataclass
class FFmpegConfig:
    """Configuration settings for FFmpeg operations."""
    codec: str = "libx264"
    audio_codec: str = "aac"
    audio_bitrate: str = "192k"
    brightness: float = 0.0
    contrast: float = 1.0


class FFmpegWrapper:
    """
    A wrapper class for FFmpeg operations, providing asynchronous methods
    for various video and audio processing tasks.
    """

    def __init__(self, config: Optional[FFmpegConfig] = None):
        """
        Initialize the FFmpegWrapper with an optional configuration.

        Args:
            config (Optional[FFmpegConfig]): Configuration settings for FFmpeg.
        """
        self.executor = ThreadPoolExecutor()  # For executing blocking FFmpeg commands
        self.config = config or FFmpegConfig()
        logger.info("FFmpegWrapper initialized with ThreadPoolExecutor.")

    async def _run_ffmpeg(self, command: ffmpeg.nodes.FilterableStream) -> None:
        """
        Asynchronously run an FFmpeg command.

        Args:
            command (ffmpeg.nodes.FilterableStream): The FFmpeg command to execute.

        Raises:
            ffmpeg.Error: If FFmpeg execution fails.
            Exception: For any unexpected errors.
        """
        try:
            loop = asyncio.get_running_loop()
            command_str = ' '.join(command.compile())
            logger.debug(f"Executing FFmpeg command: {command_str}")
            await loop.run_in_executor(self.executor, command.run)
            logger.info("FFmpeg command executed successfully.")
        except ffmpeg.Error as e:
            logger.error(f"FFmpeg execution failed: {e.stderr.decode()}")
            raise
        except Exception as e:
            logger.exception(f"Unexpected error during FFmpeg execution: {e}")
            raise

    async def convert_format(self, input_file: str, output_file: str, codec: Optional[str] = None) -> None:
        """
        Convert video format using FFmpeg.

        Args:
            input_file (str): Path to the input video file.
            output_file (str): Path to the output video file.
            codec (Optional[str]): Video codec to use. Defaults to the configuration codec.
        """
        codec = codec or self.config.codec
        try:
            command = ffmpeg.input(input_file).output(
                output_file, vcodec=codec)
            logger.debug(
                f"Converting format: {input_file} to {output_file} with codec {codec}.")
            await self._run_ffmpeg(command)
            console.print(
                f"[green]Conversion from {input_file} to {output_file} completed successfully.[/green]")
        except Exception as e:
            logger.error(
                f"Failed to convert format from {input_file} to {output_file}: {e}")
            raise

    async def extract_audio(self, input_file: str, output_file: str) -> None:
        """
        Extract audio from a video file.

        Args:
            input_file (str): Path to the input video file.
            output_file (str): Path to the output audio file.
        """
        try:
            command = ffmpeg.input(input_file).output(
                output_file, acodec='copy')
            logger.debug(
                f"Extracting audio from {input_file} to {output_file}.")
            await self._run_ffmpeg(command)
            console.print(
                f"[green]Audio extracted from {input_file} to {output_file} successfully.[/green]")
        except Exception as e:
            logger.error(
                f"Failed to extract audio from {input_file} to {output_file}: {e}")
            raise

    async def trim_video(self, input_file: str, output_file: str, start_time: int, duration: int) -> None:
        """
        Trim a video file.

        Args:
            input_file (str): Path to the input video file.
            output_file (str): Path to the output video file.
            start_time (int): Start time in seconds.
            duration (int): Duration in seconds.
        """
        try:
            command = ffmpeg.input(
                input_file, ss=start_time, t=duration).output(output_file)
            logger.debug(
                f"Trimming video {input_file}: start_time={start_time}, duration={duration}.")
            await self._run_ffmpeg(command)
            console.print(
                f"[green]Video trimmed and saved to {output_file} successfully.[/green]")
        except Exception as e:
            logger.error(
                f"Failed to trim video {input_file} to {output_file}: {e}")
            raise

    async def resize_video(self, input_file: str, output_file: str, width: int, height: int) -> None:
        """
        Resize a video to the specified width and height.

        Args:
            input_file (str): Path to the input video file.
            output_file (str): Path to the output video file.
            width (int): Target width in pixels.
            height (int): Target height in pixels.
        """
        try:
            command = ffmpeg.input(input_file).filter(
                'scale', width, height).output(output_file)
            logger.debug(
                f"Resizing video {input_file} to {width}x{height}, output {output_file}.")
            await self._run_ffmpeg(command)
            console.print(
                f"[green]Video resized to {width}x{height} and saved to {output_file} successfully.[/green]")
        except Exception as e:
            logger.error(
                f"Failed to resize video {input_file} to {output_file}: {e}")
            raise

    async def extract_frames(self, input_file: str, output_pattern: str, fps: int = 1) -> None:
        """
        Extract frames from a video at the specified frames per second.

        Args:
            input_file (str): Path to the input video file.
            output_pattern (str): Output frames pattern (e.g., frame_%04d.png).
            fps (int): Frames per second to extract. Defaults to 1.
        """
        try:
            command = ffmpeg.input(input_file).filter(
                'fps', fps=fps).output(output_pattern)
            logger.debug(
                f"Extracting frames from {input_file} to {output_pattern} at {fps} FPS.")
            await self._run_ffmpeg(command)
            console.print(
                f"[green]Frames extracted from {input_file} to {output_pattern} at {fps} FPS successfully.[/green]")
        except Exception as e:
            logger.error(
                f"Failed to extract frames from {input_file} to {output_pattern}: {e}")
            raise

    async def merge_videos(self, input_files: List[str], output_file: str) -> None:
        """
        Merge multiple video files into a single output file.

        Args:
            input_files (List[str]): List of input video file paths.
            output_file (str): Path to the output merged video file.
        """
        try:
            inputs = [ffmpeg.input(file) for file in input_files]
            command = ffmpeg.concat(*inputs, v=1, a=1).output(output_file)
            logger.debug(f"Merging videos {input_files} into {output_file}.")
            await self._run_ffmpeg(command)
            console.print(
                f"[green]Videos merged into {output_file} successfully.[/green]")
        except Exception as e:
            logger.error(
                f"Failed to merge videos {input_files} into {output_file}: {e}")
            raise

    async def merge_audios(self, input_files: List[str], output_file: str) -> None:
        """
        Merge multiple audio files into a single output file.

        Args:
            input_files (List[str]): List of input audio file paths.
            output_file (str): Path to the output merged audio file.
        """
        try:
            inputs = [ffmpeg.input(file) for file in input_files]
            command = ffmpeg.filter(
                inputs, 'amix', inputs=len(inputs)).output(output_file)
            logger.debug(f"Merging audios {input_files} into {output_file}.")
            await self._run_ffmpeg(command)
            console.print(
                f"[green]Audios merged into {output_file} successfully.[/green]")
        except Exception as e:
            logger.error(
                f"Failed to merge audios {input_files} into {output_file}: {e}")
            raise

    async def add_watermark(self, input_file: str, output_file: str, watermark_image: str, position: Literal["topleft", "topright", "bottomleft", "bottomright"] = "topright") -> None:
        """
        Add a watermark image to a video at the specified position.

        Args:
            input_file (str): Path to the input video file.
            output_file (str): Path to the output video file.
            watermark_image (str): Path to the watermark image file.
            position (Literal["topleft", "topright", "bottomleft", "bottomright"]): 
                Position to place the watermark. Defaults to "topright".
        """
        try:
            positions = {
                "topleft": "10:10",
                "topright": "main_w-overlay_w-10:10",
                "bottomleft": "10:main_h-overlay_h-10",
                "bottomright": "main_w-overlay_w-10:main_h-overlay_h-10"
            }
            pos_x, pos_y = positions.get(position, "10:10").split(":")
            command = (
                ffmpeg
                .input(input_file)
                .overlay(watermark_image, x=pos_x, y=pos_y)
                .output(output_file)
            )
            logger.debug(
                f"Adding watermark {watermark_image} to {input_file} at position {position}.")
            await self._run_ffmpeg(command)
            console.print(
                f"[green]Watermark added to {input_file} and saved as {output_file} successfully.[/green]")
        except Exception as e:
            logger.error(f"Failed to add watermark to {input_file}: {e}")
            raise

    async def add_subtitles(self, input_file: str, output_file: str, subtitle_file: str) -> None:
        """
        Add subtitles to a video.

        Args:
            input_file (str): Path to the input video file.
            output_file (str): Path to the output video file with subtitles.
            subtitle_file (str): Path to the subtitle file.
        """
        try:
            command = ffmpeg.input(input_file).output(
                output_file, vf=f"subtitles={subtitle_file}")
            logger.debug(
                f"Adding subtitles from {subtitle_file} to {input_file}, output {output_file}.")
            await self._run_ffmpeg(command)
            console.print(
                f"[green]Subtitles added to {input_file} and saved as {output_file} successfully.[/green]")
        except Exception as e:
            logger.error(f"Failed to add subtitles to {input_file}: {e}")
            raise

    async def change_speed(self, input_file: str, output_file: str, speed_factor: float) -> None:
        """
        Change the playback speed of a video.

        Args:
            input_file (str): Path to the input video file.
            output_file (str): Path to the output video file with changed speed.
            speed_factor (float): Speed factor (e.g., 2.0 for double speed).
        """
        try:
            command = ffmpeg.input(input_file).filter(
                'setpts', f"{1/speed_factor}*PTS").output(output_file)
            logger.debug(
                f"Changing speed of {input_file} by a factor of {speed_factor}, output {output_file}.")
            await self._run_ffmpeg(command)
            console.print(
                f"[green]Video speed changed by a factor of {speed_factor} and saved as {output_file} successfully.[/green]")
        except Exception as e:
            logger.error(f"Failed to change speed of {input_file}: {e}")
            raise

    async def extract_video_info(self, input_file: str) -> Dict[str, Any]:
        """
        Extract information from a video file.

        Args:
            input_file (str): Path to the input video file.

        Returns:
            Dict[str, Any]: Extracted video information.
        """
        try:
            logger.debug(f"Extracting video info from {input_file}.")
            probe = ffmpeg.probe(input_file)
            logger.info(f"Video info extracted: {probe}")
            table = Table(title="Video Information")

            for key, value in probe.items():
                table.add_row(str(key), str(value))

            console.print(table)
            return probe
        except ffmpeg.Error as e:
            logger.error(
                f"Failed to extract video info from {input_file}: {e.stderr.decode()}")
            raise
        except Exception as e:
            logger.exception(
                f"Unexpected error while extracting video info: {e}")
            raise

    async def add_background_music(self, input_video: str, input_audio: str, output_file: str, volume: float = 0.5) -> None:
        """
        Add background music to a video.

        Args:
            input_video (str): Path to the input video file.
            input_audio (str): Path to the input audio file.
            output_file (str): Path to the output video file with background music.
            volume (float): Volume level of the background music. Defaults to 0.5.
        """
        try:
            command = (
                ffmpeg
                .input(input_video)
                .input(input_audio)
                .filter('amix', inputs=2, duration='first', dropout_transition=3)
                .output(output_file, vcodec='copy', acodec='aac', audio_bitrate=self.config.audio_bitrate)
            )
            logger.debug(
                f"Adding background music from {input_audio} to {input_video}, output {output_file}.")
            await self._run_ffmpeg(command)
            console.print(
                f"[green]Background music added to {input_video} and saved as {output_file} successfully.[/green]")
        except Exception as e:
            logger.error(
                f"Failed to add background music to {input_video}: {e}")
            raise

    async def overlay_image(self, input_video: str, overlay_image: str, output_file: str, x: int = 10, y: int = 10) -> None:
        """
        Overlay an image onto a video at the specified coordinates.

        Args:
            input_video (str): Path to the input video file.
            overlay_image (str): Path to the overlay image file.
            output_file (str): Path to the output video file with the overlay.
            x (int): X-axis position of the overlay. Defaults to 10.
            y (int): Y-axis position of the overlay. Defaults to 10.
        """
        try:
            command = (
                ffmpeg
                .input(input_video)
                .overlay(overlay_image, x=x, y=y)
                .output(output_file)
            )
            logger.debug(
                f"Overlaying image {overlay_image} on {input_video} at position ({x}, {y}), output {output_file}.")
            await self._run_ffmpeg(command)
            console.print(
                f"[green]Image {overlay_image} overlaid on {input_video} and saved as {output_file} successfully.[/green]")
        except Exception as e:
            logger.error(
                f"Failed to overlay image {overlay_image} on {input_video}: {e}")
            raise

    async def adjust_brightness_contrast(self, input_file: str, output_file: str, brightness: float = 0.0, contrast: float = 1.0) -> None:
        """
        Adjust the brightness and contrast of a video.

        Args:
            input_file (str): Path to the input video file.
            output_file (str): Path to the output video file with adjustments.
            brightness (float): Brightness level (-1.0 to 1.0). Defaults to 0.0.
            contrast (float): Contrast level (0.0 and above). Defaults to 1.0.
        """
        try:
            command = ffmpeg.input(input_file).filter(
                'eq', brightness=brightness, contrast=contrast).output(output_file)
            logger.debug(
                f"Adjusting brightness to {brightness} and contrast to {contrast} for {input_file}, output {output_file}.")
            await self._run_ffmpeg(command)
            console.print(
                f"[green]Brightness and contrast adjusted for {input_file} and saved as {output_file} successfully.[/green]")
        except Exception as e:
            logger.error(
                f"Failed to adjust brightness and contrast for {input_file}: {e}")
            raise


def parse_args() -> argparse.Namespace:
    """
    Parse command-line arguments.

    Returns:
        argparse.Namespace: Parsed arguments.
    """
    parser = argparse.ArgumentParser(
        description="FFmpeg Wrapper CLI with Enhanced Features")
    subparsers = parser.add_subparsers(
        dest="command", help="Available commands")

    # Convert Format
    parser_convert = subparsers.add_parser(
        "convert", help="Convert video format")
    parser_convert.add_argument("input", help="Input file path")
    parser_convert.add_argument("output", help="Output file path")
    parser_convert.add_argument(
        "--codec", type=str, default="libx264", help="Video codec (default: libx264)")

    # Extract Audio
    parser_extract_audio = subparsers.add_parser(
        "extract-audio", help="Extract audio from video")
    parser_extract_audio.add_argument("input", help="Input video file path")
    parser_extract_audio.add_argument("output", help="Output audio file path")

    # Trim Video
    parser_trim = subparsers.add_parser("trim", help="Trim video")
    parser_trim.add_argument("input", help="Input video file path")
    parser_trim.add_argument("output", help="Output video file path")
    parser_trim.add_argument("start_time", type=int,
                             help="Start time in seconds")
    parser_trim.add_argument("duration", type=int, help="Duration in seconds")

    # Resize Video
    parser_resize = subparsers.add_parser("resize", help="Resize video")
    parser_resize.add_argument("input", help="Input video file path")
    parser_resize.add_argument("output", help="Output video file path")
    parser_resize.add_argument("width", type=int, help="Width in pixels")
    parser_resize.add_argument("height", type=int, help="Height in pixels")

    # Extract Frames
    parser_extract_frames = subparsers.add_parser(
        "extract-frames", help="Extract frames from video")
    parser_extract_frames.add_argument("input", help="Input video file path")
    parser_extract_frames.add_argument(
        "output", help="Output frames pattern (e.g., frame_%04d.png)")
    parser_extract_frames.add_argument(
        "--fps", type=int, default=1, help="Frames per second (default: 1)")

    # Merge Videos
    parser_merge_videos = subparsers.add_parser(
        "merge-videos", help="Merge multiple videos")
    parser_merge_videos.add_argument(
        "inputs", nargs='+', help="Input video file paths")
    parser_merge_videos.add_argument("output", help="Output video file path")

    # Merge Audios
    parser_merge_audios = subparsers.add_parser(
        "merge-audios", help="Merge multiple audio files")
    parser_merge_audios.add_argument(
        "inputs", nargs='+', help="Input audio file paths")
    parser_merge_audios.add_argument("output", help="Output audio file path")

    # Add Watermark
    parser_watermark = subparsers.add_parser(
        "add-watermark", help="Add watermark to video")
    parser_watermark.add_argument("input", help="Input video file path")
    parser_watermark.add_argument("output", help="Output video file path")
    parser_watermark.add_argument(
        "watermark", help="Watermark image file path")
    parser_watermark.add_argument("--position", type=str, default="topright", choices=[
                                  "topleft", "topright", "bottomleft", "bottomright"], help="Position of watermark (default: topright)")

    # Add Subtitles
    parser_subtitles = subparsers.add_parser(
        "add-subtitles", help="Add subtitles to video")
    parser_subtitles.add_argument("input", help="Input video file path")
    parser_subtitles.add_argument("output", help="Output video file path")
    parser_subtitles.add_argument("subtitle", help="Subtitle file path")

    # Change Speed
    parser_speed = subparsers.add_parser(
        "change-speed", help="Change playback speed of video")
    parser_speed.add_argument("input", help="Input video file path")
    parser_speed.add_argument("output", help="Output video file path")
    parser_speed.add_argument(
        "speed", type=float, help="Speed factor (e.g., 2 for double speed)")

    # Extract Video Info
    parser_info = subparsers.add_parser(
        "extract-info", help="Extract video information")
    parser_info.add_argument("input", help="Input video file path")

    # Add Background Music
    parser_bg_music = subparsers.add_parser(
        "add-bg-music", help="Add background music to video")
    parser_bg_music.add_argument("video", help="Input video file path")
    parser_bg_music.add_argument("audio", help="Input audio file path")
    parser_bg_music.add_argument("output", help="Output video file path")
    parser_bg_music.add_argument(
        "--volume", type=float, default=0.5, help="Volume of background music (default: 0.5)")

    # Overlay Image
    parser_overlay = subparsers.add_parser(
        "overlay-image", help="Overlay image on video")
    parser_overlay.add_argument("video", help="Input video file path")
    parser_overlay.add_argument("image", help="Overlay image file path")
    parser_overlay.add_argument("output", help="Output video file path")
    parser_overlay.add_argument(
        "--x", type=int, default=10, help="X position (default: 10)")
    parser_overlay.add_argument(
        "--y", type=int, default=10, help="Y position (default: 10)")

    # Adjust Brightness and Contrast
    parser_brightness = subparsers.add_parser(
        "adjust-bc", help="Adjust brightness and contrast of video")
    parser_brightness.add_argument("input", help="Input video file path")
    parser_brightness.add_argument("output", help="Output video file path")
    parser_brightness.add_argument(
        "--brightness", type=float, default=0.0, help="Brightness level (-1.0 to 1.0, default: 0.0)")
    parser_brightness.add_argument(
        "--contrast", type=float, default=1.0, help="Contrast level (0.0 and above, default: 1.0)")

    return parser.parse_args()


async def main() -> None:
    """
    Main function to parse command-line arguments and execute FFmpeg operations.
    """
    args = parse_args()
    ffmpeg_wrapper = FFmpegWrapper()

    try:
        if args.command == "convert":
            await ffmpeg_wrapper.convert_format(args.input, args.output, args.codec)

        elif args.command == "extract-audio":
            await ffmpeg_wrapper.extract_audio(args.input, args.output)

        elif args.command == "trim":
            await ffmpeg_wrapper.trim_video(args.input, args.output, args.start_time, args.duration)

        elif args.command == "resize":
            await ffmpeg_wrapper.resize_video(args.input, args.output, args.width, args.height)

        elif args.command == "extract-frames":
            await ffmpeg_wrapper.extract_frames(args.input, args.output, args.fps)

        elif args.command == "merge-videos":
            await ffmpeg_wrapper.merge_videos(args.inputs, args.output)

        elif args.command == "merge-audios":
            await ffmpeg_wrapper.merge_audios(args.inputs, args.output)

        elif args.command == "add-watermark":
            await ffmpeg_wrapper.add_watermark(args.input, args.output, args.watermark, args.position)

        elif args.command == "add-subtitles":
            await ffmpeg_wrapper.add_subtitles(args.input, args.output, args.subtitle)

        elif args.command == "change-speed":
            await ffmpeg_wrapper.change_speed(args.input, args.output, args.speed)

        elif args.command == "extract-info":
            info = await ffmpeg_wrapper.extract_video_info(args.input)
            # Display info in a readable table
            table = Table(title="Video Information")
            for key, value in info.items():
                table.add_row(str(key), str(value))
            console.print(table)

        elif args.command == "add-bg-music":
            await ffmpeg_wrapper.add_background_music(args.video, args.audio, args.output, args.volume)

        elif args.command == "overlay-image":
            await ffmpeg_wrapper.overlay_image(args.video, args.image, args.output, args.x, args.y)

        elif args.command == "adjust-bc":
            await ffmpeg_wrapper.adjust_brightness_contrast(args.input, args.output, args.brightness, args.contrast)

        else:
            console.print("[yellow]Use -h for help.[/yellow]")
            sys.exit(1)
    except Exception as e:
        logger.critical(f"An error occurred: {e}")
        console.print(f"[red]An error occurred: {e}[/red]")
        sys.exit(1)


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except Exception as e:
        logger.critical(f"Unhandled exception: {e}")
        sys.exit(1)
