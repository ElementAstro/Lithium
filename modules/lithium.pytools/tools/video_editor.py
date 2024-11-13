import argparse
import os
import sys
import numpy as np
from moviepy.editor import VideoFileClip, AudioFileClip, CompositeAudioClip
from moviepy.audio.fx.volumex import volumex
from moviepy.audio.fx.audio_fadein import audio_fadein
from moviepy.audio.fx.audio_fadeout import audio_fadeout
from pydub import AudioSegment
import matplotlib.pyplot as plt
from scipy.io import wavfile
from scipy.signal import spectrogram, wiener
from loguru import logger

# Configure Loguru for logging
logger.remove()  # Remove the default logger
logger.add(
    "video_editor.log",
    rotation="10 MB",
    retention="30 days",
    compression="zip",
    enqueue=True,
    encoding="utf-8",
    level="DEBUG",
    format=(
        "<green>{time:YYYY-MM-DD HH:mm:ss}</green> | "
        "<level>{level}</level> | {message}"
    ),
)


def process_audio(
    video_path,
    audio_output_path,
    output_format='mp3',
    speed=1.0,
    volume=1.0,
    start_time=None,
    end_time=None,
    fade_in=0,
    fade_out=0,
    reverse=False,
    normalize=False,
    noise_reduce=False
):
    """
    Processes audio from a video file: extracts, modifies speed, volume, applies fade effects, reverses, and saves the audio.

    Parameters:
        video_path (str): Path to the input video file.
        audio_output_path (str): Path to save the processed audio file.
        output_format (str): Format of the output audio file ('mp3' or 'wav').
        speed (float): Speed factor to adjust the audio playback.
        volume (float): Volume multiplier for the audio.
        start_time (float): Start time (in seconds) to trim the audio.
        end_time (float): End time (in seconds) to trim the audio.
        fade_in (float): Duration (in seconds) for the fade-in effect.
        fade_out (float): Duration (in seconds) for the fade-out effect.
        reverse (bool): Whether to reverse the audio.
        normalize (bool): Normalize the audio volume to -1 to 1 range.
        noise_reduce (bool): Apply basic noise reduction using Wiener filter.
    """
    logger.info(f"Processing audio from video: {video_path}")
    try:
        # Load the video file
        video = VideoFileClip(video_path)
        logger.debug(f"Video file loaded: {video_path}")

        # Extract audio from the video
        audio = video.audio
        if audio is None:
            logger.error(f"No audio track found in video: {video_path}")
            raise ValueError(
                "No audio track found in the provided video file.")
        logger.debug("Audio extracted from video.")

        # Adjust audio speed
        if speed != 1.0:
            audio = audio.speedx(speed)
            logger.debug(f"Audio speed adjusted by a factor of {speed}.")

        # Adjust audio volume
        if volume != 1.0:
            audio = audio.fx(volumex, volume)
            logger.debug(f"Audio volume adjusted by a factor of {volume}.")

        # Trim audio (if start and/or end times are specified)
        if start_time is not None or end_time is not None:
            audio = audio.subclip(start_time, end_time)
            logger.debug(
                f"Audio trimmed: start_time={start_time}, end_time={end_time}.")

        # Apply fade-in and fade-out effects
        if fade_in > 0:
            audio = audio.fx(audio_fadein, duration=fade_in)
            logger.debug(f"Applied fade-in effect for {fade_in} seconds.")
        if fade_out > 0:
            audio = audio.fx(audio_fadeout, duration=fade_out)
            logger.debug(f"Applied fade-out effect for {fade_out} seconds.")

        # Reverse the audio
        if reverse:
            audio = audio.fx(lambda clip: clip.fx(
                lambda gf: lambda t: gf(clip.duration - t)))
            logger.debug("Audio reversed.")

        # Normalize the audio
        if normalize:
            audio_array = np.array(audio.to_soundarray())
            max_amplitude = np.max(np.abs(audio_array))
            if max_amplitude > 0:
                audio_array = audio_array / max_amplitude
                audio = audio.set_audio(audio_array)
                logger.debug("Audio normalized.")
            else:
                logger.warning(
                    "Audio normalization skipped due to zero amplitude.")

        # Save the processed audio
        codec = 'libmp3lame' if output_format.lower() == 'mp3' else 'pcm_s16le'
        audio.write_audiofile(audio_output_path, codec=codec)
        logger.info(f"Processed audio saved to: {audio_output_path}")

        # Apply noise reduction if specified
        if noise_reduce:
            logger.info("Applying noise reduction using Wiener filter.")
            sample_rate, data = wavfile.read(audio_output_path)
            reduced_noise = wiener(data)
            wavfile.write(audio_output_path, sample_rate,
                          reduced_noise.astype(data.dtype))
            logger.debug("Noise reduction applied.")

        # Close the video file
        video.close()
        logger.debug("Video file closed.")

    except Exception as e:
        logger.exception(f"Failed to process audio from {video_path}: {e}")
        raise


def batch_process(input_dir, output_dir, **kwargs):
    """
    Processes all video files in a given directory in batch mode.

    Parameters:
        input_dir (str): Directory containing the input video files.
        output_dir (str): Directory where the processed audio files will be saved.
        kwargs: Additional arguments passed to the audio processing function.
    """
    logger.info(
        f"Starting batch processing. Input directory: {input_dir}, Output directory: {output_dir}")
    try:
        if not os.path.exists(output_dir):
            os.makedirs(output_dir)
            logger.debug(f"Created output directory: {output_dir}")

        supported_formats = ('.mp4', '.avi', '.mov', '.mkv')
        processed_files = 0
        for filename in os.listdir(input_dir):
            if filename.lower().endswith(supported_formats):
                input_path = os.path.join(input_dir, filename)
                output_extension = kwargs.get('output_format', 'mp3')
                output_path = os.path.join(
                    output_dir, os.path.splitext(
                        filename)[0] + f'.{output_extension}'
                )
                try:
                    process_audio(input_path, output_path, **kwargs)
                    logger.info(f"Processed: {filename}")
                    processed_files += 1
                except Exception as e:
                    logger.error(f"Error processing {filename}: {e}")
        logger.info(
            f"Batch processing completed. Total files processed: {processed_files}")
    except Exception as e:
        logger.exception(f"Batch processing failed: {e}")
        raise


def mix_audio(audio_files, output_file, volumes=None):
    """
    Mixes multiple audio files into one.

    Parameters:
        audio_files (list): List of paths to audio files to be mixed.
        output_file (str): Path to save the mixed audio file.
        volumes (list): List of volume multipliers for each audio file.
    """
    logger.info(f"Mixing audio files: {audio_files} into {output_file}")
    try:
        if volumes is None:
            volumes = [1.0] * len(audio_files)
        elif len(volumes) != len(audio_files):
            logger.error(
                "The number of volume levels must match the number of audio files.")
            raise ValueError(
                "Volumes list length does not match audio files list length.")

        audio_clips = []
        for f, v in zip(audio_files, volumes):
            clip = AudioFileClip(f).fx(volumex, v)
            audio_clips.append(clip)
            logger.debug(f"Loaded and adjusted volume for: {f}")

        final_audio = CompositeAudioClip(audio_clips)
        final_audio.write_audiofile(output_file)
        logger.info(f"Mixed audio saved to: {output_file}")

    except Exception as e:
        logger.exception(f"Failed to mix audio files: {e}")
        raise


def split_audio(input_file, output_prefix, segment_length):
    """
    Splits an audio file into segments of specified length.

    Parameters:
        input_file (str): Path to the input audio file.
        output_prefix (str): Prefix for the output segment files.
        segment_length (float): Length of each segment in seconds.
    """
    logger.info(
        f"Splitting audio file: {input_file} into segments of {segment_length} seconds.")
    try:
        audio = AudioSegment.from_file(input_file)
        length_ms = len(audio)
        segment_length_ms = segment_length * 1000

        for i, start in enumerate(range(0, length_ms, int(segment_length_ms))):
            end = start + int(segment_length_ms)
            segment = audio[start:end]
            segment_filename = f"{output_prefix}_{i+1}.mp3"
            segment.export(segment_filename, format="mp3")
            logger.debug(f"Exported segment: {segment_filename}")

        logger.info(f"Audio file split into segments successfully.")

    except Exception as e:
        logger.exception(f"Failed to split audio file {input_file}: {e}")
        raise


def convert_audio_format(input_file, output_file):
    """
    Converts an audio file to a different format.

    Parameters:
        input_file (str): Path to the input audio file.
        output_file (str): Path to the output file with the desired format.
    """
    logger.info(f"Converting audio file from {input_file} to {output_file}")
    try:
        audio = AudioSegment.from_file(input_file)
        export_format = os.path.splitext(output_file)[1][1:]
        if not export_format:
            logger.error(
                "Output file must have a valid extension to determine the format.")
            raise ValueError("Output file extension is missing or invalid.")
        audio.export(output_file, format=export_format)
        logger.info(f"Audio format converted and saved to: {output_file}")
    except Exception as e:
        logger.exception(
            f"Failed to convert audio format for {input_file}: {e}")
        raise


def visualize_audio(input_file, output_file):
    """
    Visualizes an audio file as a waveform and spectrogram.

    Parameters:
        input_file (str): Path to the input audio file (WAV format).
        output_file (str): Path to save the visualization image.
    """
    logger.info(
        f"Visualizing audio file: {input_file} and saving to {output_file}")
    try:
        sample_rate, data = wavfile.read(input_file)
        logger.debug(
            f"Audio file loaded. Sample rate: {sample_rate}, Data shape: {data.shape}")

        plt.figure(figsize=(12, 8))

        # Plot waveform
        plt.subplot(2, 1, 1)
        if data.ndim > 1:
            plt.plot(np.arange(len(data)) / sample_rate, data[:, 0])
        else:
            plt.plot(np.arange(len(data)) / sample_rate, data)
        plt.title('Audio Waveform')
        plt.xlabel('Time (seconds)')
        plt.ylabel('Amplitude')

        # Plot spectrogram
        plt.subplot(2, 1, 2)
        frequencies, times, Sxx = spectrogram(data, sample_rate)
        plt.pcolormesh(times, frequencies, 10 * np.log10(Sxx + 1e-10))
        plt.title('Spectrogram')
        plt.xlabel('Time (seconds)')
        plt.ylabel('Frequency (Hz)')
        plt.colorbar(label='Intensity (dB)')

        plt.tight_layout()
        plt.savefig(output_file)
        plt.close()
        logger.info(f"Audio visualization saved to: {output_file}")

    except Exception as e:
        logger.exception(f"Failed to visualize audio file {input_file}: {e}")
        raise


def main():
    parser = argparse.ArgumentParser(
        description="Full-featured audio processing tool."
    )
    parser.add_argument(
        "input",
        help="Path to the input video/audio file or directory containing files."
    )
    parser.add_argument(
        "output",
        help="Path to save the output audio file or directory."
    )
    parser.add_argument(
        "--format",
        choices=['mp3', 'wav'],
        default='mp3',
        help="Output audio format (default: mp3)"
    )
    parser.add_argument(
        "--speed",
        type=float,
        default=1.0,
        help="Adjust audio speed (default: 1.0)"
    )
    parser.add_argument(
        "--volume",
        type=float,
        default=1.0,
        help="Adjust audio volume (default: 1.0)"
    )
    parser.add_argument(
        "--start",
        type=float,
        help="Start time of the audio (seconds)"
    )
    parser.add_argument(
        "--end",
        type=float,
        help="End time of the audio (seconds)"
    )
    parser.add_argument(
        "--fade-in",
        type=float,
        default=0,
        help="Fade-in duration (seconds)"
    )
    parser.add_argument(
        "--fade-out",
        type=float,
        default=0,
        help="Fade-out duration (seconds)"
    )
    parser.add_argument(
        "--reverse",
        action="store_true",
        help="Reverse the audio"
    )
    parser.add_argument(
        "--normalize",
        action="store_true",
        help="Normalize the audio volume"
    )
    parser.add_argument(
        "--noise-reduce",
        action="store_true",
        help="Apply basic noise reduction"
    )
    parser.add_argument(
        "--batch",
        action="store_true",
        help="Batch process all videos in a directory"
    )
    parser.add_argument(
        "--mix",
        nargs='+',
        help="Mix multiple audio files"
    )
    parser.add_argument(
        "--mix-volumes",
        nargs='+',
        type=float,
        help="Volumes for each audio file during mix"
    )
    parser.add_argument(
        "--split",
        type=float,
        help="Split audio into segments of specified length (seconds)"
    )
    parser.add_argument(
        "--convert",
        help="Convert audio to a different format"
    )
    parser.add_argument(
        "--visualize",
        action="store_true",
        help="Generate a visualization of the audio (waveform and spectrogram)"
    )

    args = parser.parse_args()

    try:
        # Mix multiple audio files
        if args.mix:
            if args.mix_volumes and len(args.mix_volumes) != len(args.mix):
                logger.error(
                    "Number of mix volumes must match number of audio files.")
                print("Error: Number of mix volumes must match number of audio files.")
                sys.exit(1)
            mix_audio(args.mix, args.output, args.mix_volumes)

        # Split audio into segments
        elif args.split:
            split_audio(args.input, args.output, args.split)

        # Convert audio format
        elif args.convert:
            convert_audio_format(args.input, args.output)

        # Visualize the audio as waveform and spectrogram
        elif args.visualize:
            visualize_audio(args.input, args.output)

        # Batch process all video files in a directory
        elif args.batch:
            batch_process(
                args.input,
                args.output,
                output_format=args.format,
                speed=args.speed,
                volume=args.volume,
                start_time=args.start,
                end_time=args.end,
                fade_in=args.fade_in,
                fade_out=args.fade_out,
                reverse=args.reverse,
                normalize=args.normalize,
                noise_reduce=args.noise_reduce
            )

        # Process a single video or audio file
        else:
            process_audio(
                args.input,
                args.output,
                output_format=args.format,
                speed=args.speed,
                volume=args.volume,
                start_time=args.start,
                end_time=args.end,
                fade_in=args.fade_in,
                fade_out=args.fade_out,
                reverse=args.reverse,
                normalize=args.normalize,
                noise_reduce=args.noise_reduce
            )

        logger.info("Processing completed successfully.")
        print("Processing completed successfully.")

    except Exception as e:
        logger.exception(f"An error occurred during processing: {e}")
        print(f"An error occurred: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
