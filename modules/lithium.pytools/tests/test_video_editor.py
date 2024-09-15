import pytest
import os
import numpy as np
from pathlib import Path
from moviepy.editor import AudioFileClip
from ..tools.video_editor import (
    process_audio,
    batch_process,
    mix_audio,
    split_audio,
    convert_audio_format,
    visualize_audio
)


@pytest.fixture
def sample_video_path(tmp_path):
    video_path = tmp_path / "sample_video.mp4"
    # Create a dummy video file for testing
    video_path.write_text("dummy video content")
    return str(video_path)


@pytest.fixture
def sample_audio_path(tmp_path):
    audio_path = tmp_path / "sample_audio.mp3"
    # Create a dummy audio file for testing
    audio_path.write_text("dummy audio content")
    return str(audio_path)


@pytest.fixture
def sample_output_dir(tmp_path):
    output_dir = tmp_path / "output"
    output_dir.mkdir()
    return str(output_dir)


def test_process_audio(sample_video_path, sample_audio_path):
    process_audio(
        sample_video_path, sample_audio_path,
        output_format='mp3', speed=1.0, volume=1.0,
        start_time=0, end_time=10, fade_in=1, fade_out=1,
        reverse=False, normalize=False, noise_reduce=False
    )
    assert os.path.exists(sample_audio_path)


def test_batch_process(sample_video_path, sample_output_dir):
    batch_process(
        os.path.dirname(sample_video_path), sample_output_dir,
        output_format='mp3', speed=1.0, volume=1.0,
        start_time=0, end_time=10, fade_in=1, fade_out=1,
        reverse=False, normalize=False, noise_reduce=False
    )
    output_files = os.listdir(sample_output_dir)
    assert len(output_files) > 0


def test_mix_audio(sample_audio_path, tmp_path):
    output_file = tmp_path / "mixed_audio.mp3"
    mix_audio([sample_audio_path, sample_audio_path],
              str(output_file), volumes=[1.0, 0.5])
    assert os.path.exists(output_file)


def test_split_audio(sample_audio_path, tmp_path):
    output_prefix = tmp_path / "segment"
    split_audio(sample_audio_path, str(output_prefix), segment_length=5)
    segment_files = list(tmp_path.glob("segment_*.mp3"))
    assert len(segment_files) > 0


def test_convert_audio_format(sample_audio_path, tmp_path):
    output_file = tmp_path / "converted_audio.wav"
    convert_audio_format(sample_audio_path, str(output_file))
    assert os.path.exists(output_file)


def test_visualize_audio(sample_audio_path, tmp_path):
    output_file = tmp_path / "visualization.png"
    visualize_audio(sample_audio_path, str(output_file))
    assert os.path.exists(output_file)
