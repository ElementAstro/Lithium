# FFmpeg Wrapper Documentation

## Overview

The **FFmpeg Wrapper** is a Python script that provides an asynchronous interface for performing various audio and video processing tasks using FFmpeg. It supports a wide range of operations, including format conversion, audio extraction, video trimming, resizing, merging, and more. The script utilizes `asyncio` for non-blocking operations, `loguru` for comprehensive logging, and `rich` for enhanced terminal output.

---

## Features

- **Convert Video Formats**: Convert videos to different formats with specified codecs.
- **Extract Audio**: Extract audio tracks from video files.
- **Trim Videos**: Cut videos to specified start times and durations.
- **Resize Videos**: Change the dimensions of video files.
- **Extract Frames**: Capture frames from videos at specified intervals.
- **Merge Videos and Audios**: Combine multiple video or audio files into a single output.
- **Add Watermarks and Subtitles**: Overlay images and add subtitle tracks to videos.
- **Change Playback Speed**: Adjust the speed of video playback.
- **Extract Video Information**: Retrieve metadata and information from video files.
- **Add Background Music**: Integrate audio tracks into video files.
- **Overlay Images**: Place images on videos at specified coordinates.
- **Adjust Brightness and Contrast**: Modify the brightness and contrast levels of videos.
- **Comprehensive Logging**: Detailed logging of operations and errors with Loguru.
- **Beautiful CLI Output**: Enhanced terminal output with Rich, including tables and progress indicators.

---

## Requirements

- Python 3.x
- Required Python packages:
  - `ffmpeg-python`: For FFmpeg command construction and execution.
  - `loguru`: For logging.
  - `rich`: For styled console output.
  - `psutil`: For system and process utilities.

Install the necessary packages using:

```bash
pip install ffmpeg-python loguru rich psutil
```

---

## Usage

To run the script, use the following command:

```bash
python ffmpeg.py --help
```

### Command-Line Arguments

The script supports various commands for different FFmpeg operations:

- **`convert`**: Convert video format.

  - `input`: Path to the input video file.
  - `output`: Path to the output video file.
  - `--codec`: (Optional) Video codec to use (default: `libx264`).

- **`extract-audio`**: Extract audio from a video file.

  - `input`: Path to the input video file.
  - `output`: Path to the output audio file.

- **`trim`**: Trim a video file.

  - `input`: Path to the input video file.
  - `output`: Path to the output video file.
  - `start_time`: Start time in seconds.
  - `duration`: Duration in seconds.

- **`resize`**: Resize a video.

  - `input`: Path to the input video file.
  - `output`: Path to the output video file.
  - `width`: Target width in pixels.
  - `height`: Target height in pixels.

- **`extract-frames`**: Extract frames from a video.

  - `input`: Path to the input video file.
  - `output`: Output frames pattern (e.g., `frame_%04d.png`).
  - `--fps`: (Optional) Frames per second to extract (default: 1).

- **`merge-videos`**: Merge multiple video files.

  - `inputs`: List of input video file paths.
  - `output`: Path to the output merged video file.

- **`merge-audios`**: Merge multiple audio files.

  - `inputs`: List of input audio file paths.
  - `output`: Path to the output merged audio file.

- **`add-watermark`**: Add a watermark image to a video.

  - `input`: Path to the input video file.
  - `output`: Path to the output video file.
  - `watermark`: Path to the watermark image file.
  - `--position`: (Optional) Position of the watermark (default: `topright`).

- **`add-subtitles`**: Add subtitles to a video.

  - `input`: Path to the input video file.
  - `output`: Path to the output video file.
  - `subtitle`: Path to the subtitle file.

- **`change-speed`**: Change the playback speed of a video.

  - `input`: Path to the input video file.
  - `output`: Path to the output video file.
  - `speed`: Speed factor (e.g., `2` for double speed).

- **`extract-info`**: Extract information from a video file.

  - `input`: Path to the input video file.

- **`add-bg-music`**: Add background music to a video.

  - `video`: Path to the input video file.
  - `audio`: Path to the input audio file.
  - `output`: Path to the output video file.
  - `--volume`: (Optional) Volume of the background music (default: `0.5`).

- **`overlay-image`**: Overlay an image on a video.

  - `video`: Path to the input video file.
  - `image`: Path to the overlay image file.
  - `output`: Path to the output video file.
  - `--x`: (Optional) X position of the overlay (default: `10`).
  - `--y`: (Optional) Y position of the overlay (default: `10`).

- **`adjust-bc`**: Adjust brightness and contrast of a video.
  - `input`: Path to the input video file.
  - `output`: Path to the output video file.
  - `--brightness`: (Optional) Brightness level (-1.0 to 1.0, default: `0.0`).
  - `--contrast`: (Optional) Contrast level (0.0 and above, default: `1.0`).

---

## Logging

The script uses Loguru for logging, which provides detailed logs of the operations performed, including success messages and error details. The logs are written to both the console and a log file (`ffmpeg_wrapper.log`), which is rotated after reaching 5 MB and retained for 7 days.

---

## Error Handling

The script includes robust error handling:

- If FFmpeg commands fail, the error messages are logged, and exceptions are raised.
- Each operation is wrapped in try-except blocks to catch and log exceptions.
- The user is informed of any operational errors through the console.

---

## Conclusion

The **FFmpeg Wrapper** is a powerful and user-friendly tool for performing audio and video processing tasks with FFmpeg. With support for a wide range of operations, comprehensive logging, and beautiful terminal output, it simplifies the process of working with multimedia files, making it an invaluable addition to any developer's toolkit.

This script serves as a versatile solution for anyone needing to manipulate audio and video files programmatically, facilitating efficient media processing.
