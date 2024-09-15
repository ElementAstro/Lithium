import argparse
import os
import numpy as np
from moviepy.editor import VideoFileClip, AudioFileClip, CompositeAudioClip
from moviepy.audio.fx.all import volumex, audio_fadein, audio_fadeout
from pydub import AudioSegment
import matplotlib.pyplot as plt
from scipy.io import wavfile
from scipy.signal import spectrogram, wiener


def process_audio(video_path, audio_output_path, output_format='mp3', speed=1.0, volume=1.0,
                  start_time=None, end_time=None, fade_in=0, fade_out=0, reverse=False, normalize=False, noise_reduce=False):
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
    # Load the video file
    video = VideoFileClip(video_path)

    # Extract audio from the video
    audio = video.audio

    # Adjust audio speed
    if speed != 1.0:
        audio = audio.speedx(speed)

    # Adjust audio volume
    if volume != 1.0:
        audio = audio.fx(volumex, volume)

    # Trim audio (if start and/or end times are specified)
    if start_time is not None or end_time is not None:
        audio = audio.subclip(start_time, end_time)

    # Apply fade-in and fade-out effects
    if fade_in > 0:
        audio = audio.fx(audio_fadein, duration=fade_in)
    if fade_out > 0:
        audio = audio.fx(audio_fadeout, duration=fade_out)

    # Reverse the audio
    if reverse:
        audio = audio.fx(lambda clip: clip.fl_time(
            lambda t: clip.duration - t))

    # Normalize the audio
    if normalize:
        audio_array = np.array(audio.to_soundarray())
        max_amplitude = np.max(np.abs(audio_array))
        audio_array = audio_array / max_amplitude
        audio = audio.set_audio_array(audio_array)

    # Apply noise reduction
    if noise_reduce:
        sample_rate, data = wavfile.read(audio_output_path)
        data = wiener(data)
        wavfile.write(audio_output_path, sample_rate, data)

    # Save the processed audio
    audio.write_audiofile(
        audio_output_path, codec='libmp3lame' if output_format == 'mp3' else 'pcm_s16le')

    # Close the video file
    video.close()


def batch_process(input_dir, output_dir, **kwargs):
    """
    Processes all video files in a given directory in batch mode.

    Parameters:
        input_dir (str): Directory containing the input video files.
        output_dir (str): Directory where the processed audio files will be saved.
        kwargs: Additional arguments passed to the audio processing function.
    """
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    for filename in os.listdir(input_dir):
        if filename.endswith(('.mp4', '.avi', '.mov', '.mkv')):
            input_path = os.path.join(input_dir, filename)
            output_path = os.path.join(output_dir, os.path.splitext(
                filename)[0] + '.' + kwargs.get('output_format', 'mp3'))
            try:
                process_audio(input_path, output_path, **kwargs)
                print(f"Processed: {filename}")
            except Exception as e:
                print(f"Error processing {filename}: {e}")


def mix_audio(audio_files, output_file, volumes=None):
    """
    Mixes multiple audio files into one.

    Parameters:
        audio_files (list): List of paths to audio files to be mixed.
        output_file (str): Path to save the mixed audio file.
        volumes (list): List of volume multipliers for each audio file.
    """
    if volumes is None:
        volumes = [1.0] * len(audio_files)

    audio_clips = [AudioFileClip(f).volumex(v)
                   for f, v in zip(audio_files, volumes)]
    final_audio = CompositeAudioClip(audio_clips)
    final_audio.write_audiofile(output_file)


def split_audio(input_file, output_prefix, segment_length):
    """
    Splits an audio file into segments of specified length.

    Parameters:
        input_file (str): Path to the input audio file.
        output_prefix (str): Prefix for the output segment files.
        segment_length (float): Length of each segment in seconds.
    """
    audio = AudioSegment.from_file(input_file)
    length_ms = len(audio)
    segment_length_ms = segment_length * 1000

    for i, start in enumerate(range(0, length_ms, segment_length_ms)):
        end = start + segment_length_ms
        segment = audio[start:end]
        segment.export(f"{output_prefix}_{i+1}.mp3", format="mp3")


def convert_audio_format(input_file, output_file):
    """
    Converts an audio file to a different format.

    Parameters:
        input_file (str): Path to the input audio file.
        output_file (str): Path to the output file with the desired format.
    """
    audio = AudioSegment.from_file(input_file)
    export_format = os.path.splitext(output_file)[1][1:]
    audio.export(output_file, format=export_format)


def visualize_audio(input_file, output_file):
    """
    Visualizes an audio file as a waveform and spectrogram.

    Parameters:
        input_file (str): Path to the input audio file (WAV format).
        output_file (str): Path to save the visualization image.
    """
    sample_rate, data = wavfile.read(input_file)

    plt.figure(figsize=(12, 8))

    # Plot waveform
    plt.subplot(2, 1, 1)
    plt.plot(np.arange(len(data)) / sample_rate, data)
    plt.title('Audio Waveform')
    plt.xlabel('Time (seconds)')
    plt.ylabel('Amplitude')

    # Plot spectrogram
    plt.subplot(2, 1, 2)
    frequencies, times, Sxx = spectrogram(data, sample_rate)
    plt.pcolormesh(times, frequencies, 10 * np.log10(Sxx))
    plt.title('Spectrogram')
    plt.xlabel('Time (seconds)')
    plt.ylabel('Frequency (Hz)')
    plt.colorbar(label='Intensity (dB)')

    plt.tight_layout()
    plt.savefig(output_file)
    plt.close()


def main():
    parser = argparse.ArgumentParser(
        description="Full-featured audio processing tool.")
    parser.add_argument(
        "input", help="Path to the input video/audio file or directory containing files.")
    parser.add_argument(
        "output", help="Path to save the output audio file or directory.")
    parser.add_argument(
        "--format", choices=['mp3', 'wav'], default='mp3', help="Output audio format (default: mp3)")
    parser.add_argument("--speed", type=float, default=1.0,
                        help="Adjust audio speed (default: 1.0)")
    parser.add_argument("--volume", type=float, default=1.0,
                        help="Adjust audio volume (default: 1.0)")
    parser.add_argument("--start", type=float,
                        help="Start time of the audio (seconds)")
    parser.add_argument("--end", type=float,
                        help="End time of the audio (seconds)")
    parser.add_argument("--fade-in", type=float, default=0,
                        help="Fade-in duration (seconds)")
    parser.add_argument("--fade-out", type=float, default=0,
                        help="Fade-out duration (seconds)")
    parser.add_argument("--reverse", action="store_true",
                        help="Reverse the audio")
    parser.add_argument("--normalize", action="store_true",
                        help="Normalize the audio volume")
    parser.add_argument("--noise-reduce", action="store_true",
                        help="Apply basic noise reduction")
    parser.add_argument("--batch", action="store_true",
                        help="Batch process all videos in a directory")
    parser.add_argument("--mix", nargs='+', help="Mix multiple audio files")
    parser.add_argument("--mix-volumes", nargs='+', type=float,
                        help="Volumes for each audio file during mix")
    parser.add_argument("--split", type=float,
                        help="Split audio into segments of specified length (seconds)")
    parser.add_argument(
        "--convert", help="Convert audio to a different format")
    parser.add_argument("--visualize", action="store_true",
                        help="Generate a visualization of the audio (waveform and spectrogram)")

    args = parser.parse_args()

    # Mix multiple audio files
    if args.mix:
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
            args.input, args.output,
            output_format=args.format, speed=args.speed, volume=args.volume,
            start_time=args.start, end_time=args.end,
            fade_in=args.fade_in, fade_out=args.fade_out, reverse=args.reverse,
            normalize=args.normalize, noise_reduce=args.noise_reduce
        )
    # Process a single video or audio file
    else:
        process_audio(
            args.input, args.output,
            output_format=args.format, speed=args.speed, volume=args.volume,
            start_time=args.start, end_time=args.end,
            fade_in=args.fade_in, fade_out=args.fade_out, reverse=args.reverse,
            normalize=args.normalize, noise_reduce=args.noise_reduce
        )

    print("Processing completed successfully.")


if __name__ == "__main__":
    main()
