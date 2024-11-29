# Example 1: Convert video format
# This example converts the input video file to the specified output format using the default codec (libx264).
$ python3 ffmpeg.py convert input.mp4 output.mkv

# Example 2: Extract audio from video
# This example extracts the audio from the input video file and saves it to the specified output audio file.
$ python3 ffmpeg.py extract-audio input.mp4 output.mp3

# Example 3: Trim video
# This example trims the input video file starting at 10 seconds for a duration of 30 seconds and saves it to the specified output file.
$ python3 ffmpeg.py trim input.mp4 output_trimmed.mp4 10 30

# Example 4: Resize video
# This example resizes the input video file to the specified width and height and saves it to the specified output file.
$ python3 ffmpeg.py resize input.mp4 output_resized.mp4 1280 720

# Example 5: Extract frames from video
# This example extracts frames from the input video file at 1 frame per second and saves them with the specified output pattern.
$ python3 ffmpeg.py extract-frames input.mp4 frame_%04d.png --fps 1

# Example 6: Merge multiple videos
# This example merges the specified input video files into a single output video file.
$ python3 ffmpeg.py merge-videos input1.mp4 input2.mp4 input3.mp4 output_merged.mp4

# Example 7: Merge multiple audio files
# This example merges the specified input audio files into a single output audio file.
$ python3 ffmpeg.py merge-audios input1.mp3 input2.mp3 input3.mp3 output_merged.mp3

# Example 8: Add watermark to video
# This example adds a watermark image to the input video file at the specified position and saves it to the specified output file.
$ python3 ffmpeg.py add-watermark input.mp4 output_watermarked.mp4 watermark.png --position topright

# Example 9: Add subtitles to video
# This example adds subtitles from the specified subtitle file to the input video file and saves it to the specified output file.
$ python3 ffmpeg.py add-subtitles input.mp4 output_subtitled.mp4 subtitles.srt

# Example 10: Change playback speed of video
# This example changes the playback speed of the input video file by a factor of 2 (double speed) and saves it to the specified output file.
$ python3 ffmpeg.py change-speed input.mp4 output_speed.mp4 2.0

# Example 11: Extract video information
# This example extracts information from the input video file and displays it in a readable table.
$ python3 ffmpeg.py extract-info input.mp4

# Example 12: Add background music to video
# This example adds background music from the specified audio file to the input video file and saves it to the specified output file.
$ python3 ffmpeg.py add-bg-music input_video.mp4 input_audio.mp3 output_with_music.mp4 --volume 0.5

# Example 13: Overlay image on video
# This example overlays an image onto the input video file at the specified coordinates and saves it to the specified output file.
$ python3 ffmpeg.py overlay-image input_video.mp4 overlay_image.png output_with_overlay.mp4 --x 10 --y 10

# Example 14: Adjust brightness and contrast of video
# This example adjusts the brightness and contrast of the input video file and saves it to the specified output file.
$ python3 ffmpeg.py adjust-bc input.mp4 output_adjusted.mp4 --brightness 0.1 --contrast 1.2