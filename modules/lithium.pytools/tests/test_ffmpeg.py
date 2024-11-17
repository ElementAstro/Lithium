import pytest
from unittest.mock import patch, MagicMock
from ffmpeg import FFmpegWrapper
import ffmpeg
import asyncio

# test_ffmpeg.py


@pytest.fixture
def ffmpeg_wrapper():
    return FFmpegWrapper()

@pytest.mark.asyncio
@patch("ffmpeg.run")
async def test_run_ffmpeg_success(mock_run, ffmpeg_wrapper):
    mock_run.return_value = None
    command = ffmpeg.input("input.mp4").output("output.mp4")
    await ffmpeg_wrapper._run_ffmpeg(command)
    mock_run.assert_called_once()

@pytest.mark.asyncio
@patch("ffmpeg.run", side_effect=ffmpeg.Error("error", b"stderr"))
async def test_run_ffmpeg_failure(mock_run, ffmpeg_wrapper):
    command = ffmpeg.input("input.mp4").output("output.mp4")
    with pytest.raises(ffmpeg.Error):
        await ffmpeg_wrapper._run_ffmpeg(command)

@pytest.mark.asyncio
@patch("ffmpeg.run")
async def test_convert_format(mock_run, ffmpeg_wrapper):
    mock_run.return_value = None
    await ffmpeg_wrapper.convert_format("input.mp4", "output.mp4")
    mock_run.assert_called_once()

@pytest.mark.asyncio
@patch("ffmpeg.run")
async def test_extract_audio(mock_run, ffmpeg_wrapper):
    mock_run.return_value = None
    await ffmpeg_wrapper.extract_audio("input.mp4", "output.mp3")
    mock_run.assert_called_once()

@pytest.mark.asyncio
@patch("ffmpeg.run")
async def test_trim_video(mock_run, ffmpeg_wrapper):
    mock_run.return_value = None
    await ffmpeg_wrapper.trim_video("input.mp4", "output.mp4", 10, 20)
    mock_run.assert_called_once()

@pytest.mark.asyncio
@patch("ffmpeg.run")
async def test_resize_video(mock_run, ffmpeg_wrapper):
    mock_run.return_value = None
    await ffmpeg_wrapper.resize_video("input.mp4", "output.mp4", 1920, 1080)
    mock_run.assert_called_once()

@pytest.mark.asyncio
@patch("ffmpeg.run")
async def test_extract_frames(mock_run, ffmpeg_wrapper):
    mock_run.return_value = None
    await ffmpeg_wrapper.extract_frames("input.mp4", "frame_%04d.png", 1)
    mock_run.assert_called_once()

@pytest.mark.asyncio
@patch("ffmpeg.run")
async def test_merge_videos(mock_run, ffmpeg_wrapper):
    mock_run.return_value = None
    await ffmpeg_wrapper.merge_videos(["input1.mp4", "input2.mp4"], "output.mp4")
    mock_run.assert_called_once()

@pytest.mark.asyncio
@patch("ffmpeg.run")
async def test_merge_audios(mock_run, ffmpeg_wrapper):
    mock_run.return_value = None
    await ffmpeg_wrapper.merge_audios(["input1.mp3", "input2.mp3"], "output.mp3")
    mock_run.assert_called_once()

@pytest.mark.asyncio
@patch("ffmpeg.run")
async def test_add_watermark(mock_run, ffmpeg_wrapper):
    mock_run.return_value = None
    await ffmpeg_wrapper.add_watermark("input.mp4", "output.mp4", "watermark.png", "topright")
    mock_run.assert_called_once()

@pytest.mark.asyncio
@patch("ffmpeg.run")
async def test_add_subtitles(mock_run, ffmpeg_wrapper):
    mock_run.return_value = None
    await ffmpeg_wrapper.add_subtitles("input.mp4", "output.mp4", "subtitles.srt")
    mock_run.assert_called_once()

@pytest.mark.asyncio
@patch("ffmpeg.run")
async def test_change_speed(mock_run, ffmpeg_wrapper):
    mock_run.return_value = None
    await ffmpeg_wrapper.change_speed("input.mp4", "output.mp4", 2.0)
    mock_run.assert_called_once()

@pytest.mark.asyncio
@patch("ffmpeg.probe")
async def test_extract_video_info(mock_probe, ffmpeg_wrapper):
    mock_probe.return_value = {"streams": []}
    info = await ffmpeg_wrapper.extract_video_info("input.mp4")
    assert info == {"streams": []}
    mock_probe.assert_called_once()

@pytest.mark.asyncio
@patch("ffmpeg.run")
async def test_add_background_music(mock_run, ffmpeg_wrapper):
    mock_run.return_value = None
    await ffmpeg_wrapper.add_background_music("input.mp4", "audio.mp3", "output.mp4", 0.5)
    mock_run.assert_called_once()

@pytest.mark.asyncio
@patch("ffmpeg.run")
async def test_overlay_image(mock_run, ffmpeg_wrapper):
    mock_run.return_value = None
    await ffmpeg_wrapper.overlay_image("input.mp4", "overlay.png", "output.mp4", 10, 10)
    mock_run.assert_called_once()

@pytest.mark.asyncio
@patch("ffmpeg.run")
async def test_adjust_brightness_contrast(mock_run, ffmpeg_wrapper):
    mock_run.return_value = None
    await ffmpeg_wrapper.adjust_brightness_contrast("input.mp4", "output.mp4", 0.1, 1.2)
    mock_run.assert_called_once()