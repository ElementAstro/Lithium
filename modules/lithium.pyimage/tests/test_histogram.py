import pytest
import numpy as np
from unittest.mock import patch, MagicMock
from pathlib import Path
import sys
from ..histogram import main

# FILE: modules/lithium.pyimage/image/transformation/test_histogram.py


@pytest.fixture
def sample_image():
    # Create a dummy BGR image
    return np.zeros((100, 100, 3), dtype=np.uint8)


@pytest.fixture
def mock_args():
    args = [
        '--input', 'dummy_input.jpg',
        '--output', 'dummy_output.jpg'
    ]
    return args


@pytest.fixture
def mock_cv2_imread(sample_image):
    with patch('cv2.imread', return_value=sample_image) as mock_imread:
        yield mock_imread


@pytest.fixture
def mock_cv2_imshow():
    with patch('cv2.imshow') as mock_imshow:
        yield mock_imshow


@pytest.fixture
def mock_cv2_waitKey():
    with patch('cv2.waitKey', return_value=0) as mock_waitKey:
        yield mock_waitKey


@pytest.fixture
def mock_cv2_destroyAllWindows():
    with patch('cv2.destroyAllWindows') as mock_destroy:
        yield mock_destroy


@pytest.fixture
def mock_matplotlib_show():
    with patch('matplotlib.pyplot.show') as mock_show:
        yield mock_show


@pytest.fixture
def mock_logger():
    with patch('histogram.logger') as mock_logger:
        yield mock_logger


def test_main_success(mock_args, mock_cv2_imread, mock_cv2_imshow,
                      mock_cv2_waitKey, mock_cv2_destroyAllWindows,
                      mock_matplotlib_show, mock_logger):
    with patch.object(sys, 'argv', ['histogram.py'] + mock_args):
        main()
        # Verify that imread was called with the correct input path
        mock_cv2_imread.assert_called_with('dummy_input.jpg')
        # Verify that imshow was called for original and transformed images
        assert mock_cv2_imshow.call_count >= 4  # Original, transformed, etc.
        # Verify that waitKey and destroyAllWindows were called
        mock_cv2_waitKey.assert_called()
        mock_cv2_destroyAllWindows.assert_called()
        # Verify that matplotlib show was called
        mock_matplotlib_show.assert_called()
        # Verify logging calls
        mock_logger.info.assert_any_call("Loading image from dummy_input.jpg")
        mock_logger.info.assert_any_call(
            "Image loaded successfully with shape (100, 100, 3)")
        mock_logger.info.assert_any_call(
            "Histogram transformation applied successfully")
        mock_logger.info.assert_any_call("Applying auto clipping")


def test_main_invalid_input_path(mock_args, mock_cv2_imread, mock_logger):
    # Configure imread to return None to simulate failed image load
    mock_cv2_imread.return_value = None
    with patch.object(sys, 'argv', ['histogram.py', '--input', 'non_existent.jpg', '--output', 'dummy_output.jpg']):
        with pytest.raises(SystemExit) as exc_info:
            main()
        assert exc_info.value.code == 1
        # Verify that an error was logged
        mock_logger.error.assert_called_with(
            "Failed to load image: non_existent.jpg")


def test_main_save_histogram(mock_args, mock_cv2_imread, mock_cv2_imshow,
                             mock_cv2_waitKey, mock_cv2_destroyAllWindows,
                             mock_matplotlib_show, mock_logger):
    args = [
        '--input', 'dummy_input.jpg',
        '--output', 'dummy_output.jpg',
        '--save_histogram', 'histogram.png'
    ]
    with patch.object(sys, 'argv', ['histogram.py'] + args):
        with patch('histogram.save_histogram') as mock_save_hist:
            main()
            # Verify that save_histogram was called with correct arguments
            mock_save_hist.assert_called_once()
            mock_logger.info.assert_any_call(
                "Saving histogram to histogram.png")


def test_main_real_time_preview(mock_args, mock_cv2_imread, mock_cv2_imshow,
                                mock_cv2_waitKey, mock_cv2_destroyAllWindows,
                                mock_matplotlib_show, mock_logger):
    args = [
        '--input', 'dummy_input.jpg',
        '--output', 'dummy_output.jpg',
        '--real_time_preview'
    ]
    with patch.object(sys, 'argv', ['histogram.py'] + args):
        with patch('histogram.real_time_preview') as mock_preview:
            main()
            # Verify that real_time_preview was called
            mock_preview.assert_called_once()


def test_main_missing_arguments():
    # Test missing required arguments
    with patch.object(sys, 'argv', ['histogram.py', '--input', 'dummy_input.jpg']):
        with pytest.raises(SystemExit) as exc_info:
            main()
        assert exc_info.value.code != 0  # Expecting non-zero exit due to missing --output


def test_main_invalid_arguments(mock_args, mock_cv2_imread, mock_cv2_imshow,
                                mock_cv2_waitKey, mock_cv2_destroyAllWindows,
                                mock_matplotlib_show, mock_logger):
    # Test with invalid operation argument
    args = [
        '--input', 'dummy_input.jpg',
        '--output', 'dummy_output.jpg',
        '--operation', 'invalid_op'
    ]
    with patch.object(sys, 'argv', ['histogram.py'] + args):
        with pytest.raises(SystemExit):
            main()
        # Verify that an error was logged
        mock_logger.error.assert_called()


def test_main_save_histogram_failure(mock_args, mock_cv2_imread, mock_cv2_imshow,
                                     mock_cv2_waitKey, mock_cv2_destroyAllWindows,
                                     mock_matplotlib_show, mock_logger):
    args = [
        '--input', 'dummy_input.jpg',
        '--output', 'dummy_output.jpg',
        '--save_histogram', '/invalid_path/histogram.png'
    ]
    with patch.object(sys, 'argv', ['histogram.py'] + args):
        with patch('histogram.save_histogram', side_effect=Exception("Save failed")):
            with pytest.raises(Exception) as exc_info:
                main()
            assert str(exc_info.value) == "Save failed"
            # Verify that an error was logged
            mock_logger.error.assert_called_with(
                "Failed to save histogram: Save failed")
